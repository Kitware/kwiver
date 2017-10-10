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
 * \brief Implementation of ocv::filter_blur
 */

#include "filter_blur.h"

#include <vital/exceptions.h>
#include <vital/util/wall_timer.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class filter_blur::priv
{
public:
  /// Constructor
  int m_k_width;
  int m_k_height;
  cv::Size m_ksize;
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;
  
  priv()
    : m_k_width(0), 
      m_k_height(0)
  {
  }
  
  void set_ksize()
  {
    m_ksize = cv::Size(m_k_width, m_k_height);
  }
  
  // Apply blur operation to image
  void
  filter(cv::Mat const &cv_src, cv::Mat &cv_dest)
  {
    cv::blur( cv_src, cv_dest, m_ksize );
    //LOG_TRACE( m_logger, "Finished warping");
  }
};


/// Constructor
filter_blur
::filter_blur()
: d_(new priv)
{
  attach_logger( "arrows.ocv.filter_blur" );
  d_->m_logger = logger();
}

/// Destructor
filter_blur
::~filter_blur() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
filter_blur
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value("k_width", d_->m_k_width,
                    "Width of the Gaussian kernel, which must be positive and "
                    "odd." );
  config->set_value("k_height", d_->m_k_width,
                    "Height of the Gaussian kernel, which must be positive and "
                    "odd." );
  return config;
}


/// Set this algo's properties via a config block
void
filter_blur
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  d_->m_k_width = config->get_value<int>("k_width");
  d_->m_k_height = config->get_value<int>("k_height");
  
  d_->set_ksize();
  
  LOG_DEBUG( logger(), "k_width: " << d_->m_k_width);
  LOG_DEBUG( logger(), "m_k_height: " << d_->m_k_height);
}


bool
filter_blur
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


/// Warp an input image with a homography
kwiver::vital::image_container_sptr
filter_blur::
filter( kwiver::vital::image_container_sptr image_data )
{
  LOG_TRACE( logger(), "Starting algorithm" );
  d_->m_timer.start();
  
  if ( !image_data )
  {
    throw vital::invalid_data("Inputs to ocv::filter_blur are null");
  }

  cv::Mat cv_src = ocv::image_container::vital_to_ocv( image_data->get_image() );
  
  if( cv_src.channels() == 1 )
  {
    // TODO: Figure out why this is necessary. Something is wrong with 
    // vital_to_ocv for grayscale images.
    cv_src = cv_src.clone();
  }
  
  cv::Mat cv_dest;
  d_->filter( cv_src, cv_dest );
  
  kwiver::vital::image_container_sptr image_dest;
  image_dest = std::make_shared<ocv::image_container>(cv_dest);
  
  d_->m_timer.stop();
  LOG_TRACE( logger(), "Total processing time: " << d_->m_timer.elapsed() << " seconds");
  return image_dest;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
