/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

/**
 * \file
 * \brief Implementation of ocv::stabilize_video_KLT
 */

#include "stabilize_video_KLT.h"

#include <vital/exceptions.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/eigen.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class stabilize_video_KLT::priv
{
public:
  /// Constructor
  priv()
  {
  }
};


/// Constructor
stabilize_video_KLT
::stabilize_video_KLT()
: d_(new priv)
{
  attach_logger( "arrows.ocv.stabilize_video_KLT" );
}


/// Destructor
stabilize_video_KLT
::~stabilize_video_KLT() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
stabilize_video_KLT
::get_configuration() const
{
  std::cout << "stabilize_video_KLT::get_configuration" << std::endl;
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  return config;
}


/// Set this algo's properties via a config block
void
stabilize_video_KLT
::set_configuration(vital::config_block_sptr in_config)
{
  std::cout << "stabilize_video_KLT::set_configuration" << std::endl;
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);
}


bool
stabilize_video_KLT
::check_configuration(vital::config_block_sptr config) const
{
  std::cout << "stabilize_video_KLT::check_configuration" << std::endl;
  return true;
}


/// Return homography to stabilize the image_src relative to the key frame 
void
stabilize_video_KLT
::process_image( const timestamp& ts,
                 const image_container_sptr image_src,
                 homography_f2f_sptr src_to_ref,
                 bool&  coordinate_system_updated)
{
  std::cout << "HERE1!!" << std::endl;
  if ( !image_src)
  {
    throw vital::invalid_data("Inputs to ocv::stabilize_video_KLT are null");
  }

  cv::Mat cv_src = ocv::image_container::vital_to_ocv(image_src->get_image());
  
  cv::Mat H_cv = (cv::Mat_<double>(3,3) << 1, 0, 0, 0, 1, 0, 0, 0, 1);
  vital::matrix_3x3d H_mat;
  cv2eigen(H_cv, H_mat);
  src_to_ref = homography_f2f_sptr(new homography_f2f(H_mat, ts, ts));
  
  std::cout << "HERE2!!" << std::endl;  
  coordinate_system_updated = false;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
