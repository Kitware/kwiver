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
 * \brief Implementation of ocv::equalize_histogram
 */

#include "equalize_histogram.h"

#include <vital/exceptions.h>
#include <vital/util/wall_timer.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class equalize_histogram::priv
{
public:
  enum color_handling_modes { all_separately, luminance } ;
  color_handling_modes m_color_mode=all_separately;
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;

  /// Constructor
  priv()
  {
  }

  /// Destructor
  ~priv()
  {
  }
  
  void
  set_color_handling(const std::string color_mode)
  {
    if( color_mode == "all_separately" )
    {
      m_color_mode = all_separately;
    }
    else if( color_mode == "luminance" )
    {
      m_color_mode = luminance;
    }
    else
    {
      throw vital::invalid_data( "color_mode '" + color_mode + "' not recognized." );
    }
  }
  
  
  void
  equalize_histogram(const cv::Mat &src, cv::Mat &dst)
  {
    if( src.channels() == 1 )
    {
      cv::equalizeHist( src, dst );
    }
    else if( src.channels() == 3 )
    {
      switch( m_color_mode )
      {
        case all_separately:
        {
          // Each channel is equalized independently
          cv::Mat RGB_3[3];
          cv::split( src, RGB_3 );
          for( size_t i=0; i < 3; ++i)
          {
            cv::equalizeHist( RGB_3[i], RGB_3[i] );
          }
          cv::merge( RGB_3, 3, dst );
          break;
        }
        case luminance:
        {
          cv::Mat ycbcr, YCBCR[3];
          cv::cvtColor(src, ycbcr, CV_RGB2YCrCb, 3);
          cv::split( ycbcr, YCBCR );
          cv::equalizeHist( YCBCR[0], YCBCR[0] );
          //YCBCR[1] = cv::Scalar(123);   // for testing
          //YCBCR[2] = cv::Scalar(123);   // for testing
          cv::merge( YCBCR, 3, ycbcr );
          cv::cvtColor(ycbcr, dst, CV_YCrCb2RGB, 3);
          break;
        }
        default:
        {
          throw "Unrecognized color mode.";
        }
      }
    }
    else
    {
      throw vital::invalid_data( "Image must have 1 or 3 channels but instead "
                                 "had "  + std::to_string( src.channels() ));
    }
    return;
  }
};


/// Constructor
equalize_histogram
::equalize_histogram()
: d_(new priv)
{
  attach_logger( "arrows.ocv.equalize_histogram" );
  d_->m_logger = logger();
}

/// Destructor
equalize_histogram
::~equalize_histogram() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
equalize_histogram
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value("color_mode", "all_separately",
                    "In the case of color images, this sets how the channels "
                    "are equalized. If set to 'all_separately', each channel "
                    "is equalized independently. If set to 'luminance', the "
                    "image is converted into YCbCr, the luminance is "
                    "equalized, and then the image is converted back to RGB." );
  return config;
}


/// Set this algo's properties via a config block
void
equalize_histogram
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  std::string color_mode = config->get_value<std::string>("color_mode");
  d_->set_color_handling(color_mode);
  LOG_DEBUG( logger(), "Color mode: " + color_mode);
}


bool
equalize_histogram
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


/// Equalize an image's histogram
kwiver::vital::image_container_sptr
equalize_histogram::
filter( kwiver::vital::image_container_sptr img )
{
  if ( !img )
  {
    throw vital::invalid_data("Inputs to ocv::equalize_histogram are null");
  }
  
  LOG_TRACE( logger(), "Received image ([" + std::to_string(img->width()) + 
             ", " + std::to_string(img->height()) + ", " +
             std::to_string(img->depth()) + "]");

  cv::Mat cv_src {arrows::ocv::image_container::vital_to_ocv( img->get_image() )};
  cv::Mat cv_dest;
  
  if( cv_src.channels() == 1 )
  {
    // TODO: Figure out why this is necessary. Something is wrong with 
    // vital_to_ocv for grayscale images.
    cv_src = cv_src.clone();
  }
  
  d_->equalize_histogram(cv_src, cv_dest);
  
  kwiver::vital::image_container_sptr image_dest;
  image_dest = std::make_shared<ocv::image_container>(cv_dest);

  return image_dest;
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
