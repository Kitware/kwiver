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
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER544
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Implementation of ocv::warp_image
 */

#include "warp_image.h"

#include <vital/exceptions.h>
#include <vital/util/wall_timer.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class warp_image::priv
{
public:
  /// Constructor
  bool m_auto_size_output;
  int m_flags;
  int m_interpolation;
  bool m_inverse;
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;
  
  priv()
    : m_auto_size_output(false), 
      m_interpolation(cv::INTER_LINEAR),
      m_inverse(false)
  {
  }
  
  void
  set_inverse(bool inverse)
  {
    m_inverse = inverse;
    update_flags();
  }
  
  // Set OpenCV interpolation integer value from the string representation
  void
  set_interp_from_str(std::string interp_str)
  {
    if( interp_str == "nearest" )
    {
      m_interpolation = cv::INTER_NEAREST;
    }
    else if( interp_str == "linear" )
    {
      m_interpolation = cv::INTER_LINEAR;
    }
    else if( interp_str == "cubic" )
    {
      m_interpolation = cv::INTER_CUBIC;
    }
    else if( interp_str == "lanczos4" )
    {
      m_interpolation = cv::INTER_LANCZOS4;
    }
    else
    {
      throw vital::invalid_value( "Invalid interpolation method: " + interp_str );
    }
    
    update_flags();
  }
  
  // Combine m_inverse and m_interpolation into a flag integer
  void
  update_flags()
  {
    m_flags = m_interpolation;
    if( m_inverse )
    {
      m_flags = m_flags + cv::WARP_INVERSE_MAP;
    }
  }
  
  // Warp image using homography
  void
  warp(cv::Mat const &cv_src, cv::Mat &cv_dest, cv::Mat const &cv_H)
  {
    cv::Size cv_dsize = cv_dest.size();
    if ( cv_dsize.width < 1 || cv_dsize.height < 1 )
    {
      cv_dsize = cv_src.size();
    }
    
    LOG_TRACE( m_logger, "Warping source image [" << cv_src.cols << ", " << 
               cv_src.rows << "] channels = " << cv_src.channels() << 
               ", type = " << cv_src.type() << " to destination resolution " << 
               cv_dsize);
    cv::warpPerspective(cv_src, cv_dest, cv_H, cv_dsize, m_flags);
    //LOG_TRACE( m_logger, "Finished warping");
  }
};


/// Constructor
warp_image
::warp_image()
: d_(new priv)
{
  attach_logger( "arrows.ocv.warp_image" );
  d_->m_logger = logger();
}

/// Destructor
warp_image
::~warp_image() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
warp_image
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value("auto_size_output", d_->m_auto_size_output,
                    "If an output image is allocated, resize to contain all input pixels");
  config->set_value("interpolation", "linear", 
                    "Interpolation method (nearest, linear, cubic, lanczos4)");
  config->set_value("inverse", d_->m_inverse, 
                    "Homography is interpreted as mapping points from the "
                    "output image back to the input image");
  return config;
}


/// Set this algo's properties via a config block
void
warp_image
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  d_->m_auto_size_output = config->get_value<bool>("auto_size_output");
  if( d_->m_auto_size_output )
  {
    throw std::logic_error( "Not implemented!" );
  }
  
  d_->set_inverse(config->get_value<bool>("inverse"));
  std::string interp_str = config->get_value<std::string>("interpolation");
  d_->set_interp_from_str(interp_str);
  
  LOG_DEBUG( logger(), "Inverting homography: " << d_->m_inverse);
  LOG_DEBUG( logger(), "Interpolation method: " << interp_str);
}


bool
warp_image
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


/// Warp an input image with a homography
void
warp_image
::warp( image_container_sptr const image_src,
        image_container_sptr& image_dest,
        homography_sptr homog) const
{
  LOG_TRACE( logger(), "Starting algorithm");
  d_->m_timer.start();
  
  if ( !image_src || !homog )
  {
    throw vital::invalid_data("Inputs to ocv::warp_image are null");
  }

  cv::Mat cv_src = ocv::image_container::vital_to_ocv(image_src->get_image());

  cv::Mat cv_H;
  eigen2cv(homog->matrix(), cv_H);

  cv::Mat cv_dest;
  if ( image_dest )
  {
    cv_dest = ocv::image_container::vital_to_ocv(image_dest->get_image());
    
    if( cv_dest.channels() == 1 )
    {
      // TODO: Figure out why this is necessary. Something is wrong with 
      // vital_to_ocv for grayscale images.
      cv_dest = cv_dest.clone();
    }
  }
  else
  {
    throw vital::invalid_data("Inputs to ocv::warp_image are null");
  }
  
  d_->m_timer.stop();
  LOG_TRACE( logger(), "Getting and converting imagery operation time: " << 
             d_->m_timer.elapsed() << " seconds");
  
  d_->m_timer.start();
  d_->warp(cv_src, cv_dest, cv_H);
  d_->m_timer.stop();
  LOG_TRACE( logger(), "Warping operation time: " << d_->m_timer.elapsed() << " seconds");
  
  
  LOG_TRACE( logger(), "Rendered image [" << cv_dest.cols << ", " << 
             cv_dest.rows << "] channels = " << cv_dest.channels() << 
             ", type = " << cv_dest.type() );

  image_dest = std::make_shared<ocv::image_container>(cv_dest);
  LOG_TRACE( logger(), "Finished algorithm");
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
