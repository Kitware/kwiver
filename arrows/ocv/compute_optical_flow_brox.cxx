/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <arrows/ocv/compute_optical_flow_brox.h>

#include <kwiversys/SystemTools.hxx>
#include <vital/vital_types.h>
#include <vital/config/config_block_formatter.h>
#include <vital/types/image_container.h>
#include <arrows/ocv/image_container.h>
#include <vital/types/image.h>

#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/cudaoptflow.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

class compute_optical_flow_brox::priv
{
public:
  priv()
    : alpha( 0.197 )
    , gamma( 50 )
    , scale_factor( 0.8 )
    , inner_iterations( 10 )
    , outer_iterations( 77 )
    , solver_iterations( 10 )
  {}

  ~priv()
  {}
  
  float alpha, gamma, scale_factor;
  int inner_iterations, outer_iterations, solver_iterations;

  // Initialize all the matrices
  cv::Mat img, s_img, img_32FC1, s_img_32FC1, u_out, v_out, img_out;
  cv::cuda::GpuMat img_gpu, s_img_gpu, u_gpu_out, v_gpu_out, flow_gpu_out;
  cv::cuda::GpuMat flow_planes[2];
  
  // Helper function
  void color_code(const cv::Mat &u_mat, const cv::Mat &v_mat, 
                                        cv::Mat &image);

  cv::Ptr<cv::cuda::BroxOpticalFlow> brox_flow;

  kwiver::vital::logger_handle_t m_logger;  
}; // end of private class

compute_optical_flow_brox::
compute_optical_flow_brox()
  : o( new priv() )
{
  attach_logger( "arrows.ocv.compute_optical_flow_brox" );
  o->m_logger = logger();
}

compute_optical_flow_brox::
~compute_optical_flow_brox()
{}

vital::config_block_sptr
compute_optical_flow_brox::
get_configuration() const
{
  vital::config_block_sptr config = vital::algorithm::get_configuration();
  config->set_value( "alpha", o->alpha, "Alpha value for optical flow algorithm");
  config->set_value( "gamma", o->gamma, "Gamma value for optical flow algorithm");
  config->set_value( "scale_factor", o->scale_factor, 
                    "Scale factor for optical flow algorithm");
  config->set_value( "inner_iterations", o->inner_iterations, 
                    "Inner iteration for optical flow algorithm");
  config->set_value( "outer_iterations", o->outer_iterations, 
                    "Outer iteration for optical flow algorithm");
  config->set_value( "solver_iterations", o->solver_iterations, 
                    "Solver iterations for optical flow algorithm");
  return config;
}

void
compute_optical_flow_brox::
set_configuration( vital::config_block_sptr input_config )
{ 
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( input_config );
  this->o->alpha = config->get_value< float >( "alpha" );
  this->o->gamma = config->get_value< float >( "gamma" );
  this->o->scale_factor = config->get_value< float >( "scale_factor" );
  this->o->inner_iterations = config->get_value< int >( "inner_iterations" );
  this->o->outer_iterations = config->get_value< int >( "outer_iterations" );
  this->o->solver_iterations = config->get_value< int >( "solver_iterations" );
}


void compute_optical_flow_brox::priv::
color_code(const cv::Mat &u_mat, const cv::Mat &v_mat, cv::Mat &image)
{
    // Slightly modified version of 
    // https://gist.github.com/denkiwakame/56667938239ab8ee5d8a
    cv::Mat magnitude, angle, saturation;
    cv::cartToPolar(u_mat, v_mat, magnitude, angle, true);
    cv::normalize(magnitude, magnitude, 0, 255, cv::NORM_MINMAX);
    saturation =  cv::Mat::ones(magnitude.size(), CV_32F);
    cv::multiply(saturation, 255, saturation);
    cv::Mat hsvPlanes[] = { angle, saturation, magnitude };
    cv::merge(hsvPlanes, 3, image);
    cv::cvtColor(image, image, cv::COLOR_HSV2BGR);
}

vital::image_container_sptr
compute_optical_flow_brox::
compute( vital::image_container_sptr image,
         vital::image_container_sptr successive_image ) const
{
  // Convert the image container to cv images
  o->img = kwiver::arrows::ocv::image_container::vital_to_ocv( 
                    image->get_image(), 
                    kwiver::arrows::ocv::image_container::ColorMode::RGB_COLOR );
  o->s_img = kwiver::arrows::ocv::image_container::vital_to_ocv( 
                    successive_image->get_image(), 
                    kwiver::arrows::ocv::image_container::ColorMode::RGB_COLOR );
  
  // Create greyscale version of the images
  cv::cvtColor(o->img, o->img, cv::COLOR_RGB2GRAY);
  cv::cvtColor(o->s_img, o->s_img, cv::COLOR_RGB2GRAY);

  // Bring the matrices in the format used by the algorithm
  o->img.convertTo(o->img, CV_32F, 1.0/255.0);
  o->s_img.convertTo(o->s_img, CV_32F, 1.0/255.0);
  o->img.convertTo(o->img_32FC1, CV_32FC1);
  o->s_img.convertTo(o->s_img_32FC1, CV_32FC1);

  // Transfer the image to gpu
  o->img_gpu.upload(o->img_32FC1);
  o->s_img_gpu.upload(o->s_img_32FC1);

  // Compute optical flow
  o->brox_flow->calc(o->img_gpu, o->s_img_gpu, o->flow_gpu_out);

  // Split the vector into its component
  cv::cuda::split(o->flow_gpu_out, o->flow_planes);
  o->flow_planes[0].download(o->u_out);
  o->flow_planes[1].download(o->v_out); 
   
  o->color_code(o->u_out, o->v_out, o->img_out);
  return std::make_shared< kwiver::arrows::ocv::image_container >(o->img_out,
                          kwiver::arrows::ocv::image_container::ColorMode::RGB_COLOR);
}

}
}
}

