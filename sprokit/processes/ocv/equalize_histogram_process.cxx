/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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

/**
 * \file
 * \brief Implementation of equalize_histogram_process
 */

#include "equalize_histogram_process.h"

#include <vital/vital_types.h>
#include <vital/vital_foreach.h>
#include <vital/util/wall_timer.h>

#include <arrows/ocv/image_container.h>

#include <sprokit/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <sstream>
#include <iostream>

namespace kwiver {

  create_config_trait( color_mode, std::string, "all_separately", "In the "
                       "case of color images, this sets how the channels are "
                       "equalized. If set to 'all_separately', each channel is "
                       "equalized independently. If set to 'luminance', the "
                       "image is converted into YCbCr, the luminance is "
                       "equalized, and then the image is converted back to RGB" );
  
// ==================================================================
class equalize_histogram_process::priv
{
public:
    
  priv()
  {
  }

  ~priv()
  {
  }
  
  enum color_handling_modes { all_separately, luminance } ;
  color_handling_modes m_color_mode;
  kwiver::vital::wall_timer m_timer;
  
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
  
}; // end priv class


// ==================================================================
equalize_histogram_process
::equalize_histogram_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new equalize_histogram_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
  make_ports();
  make_config();
}


equalize_histogram_process
  ::~equalize_histogram_process()
{
}


// ------------------------------------------------------------------
void
equalize_histogram_process::_configure()
{
  std::string color_mode = config_value_using_trait( color_mode );
  d->set_color_handling(color_mode);
  LOG_DEBUG( logger(), "Color mode: " + color_mode);
} // equalize_histogram_process::_configure


// ------------------------------------------------------------------
void
equalize_histogram_process::_step()
{
  d->m_timer.start();
  
  // image
  kwiver::vital::image_container_sptr img = grab_from_port_using_trait( image );
  
  if ( img == NULL )
  {
    throw kwiver::vital::invalid_value( "Input image pointer is NULL." );
  }
  
  LOG_DEBUG( logger(), "Received image ([" + std::to_string(img->width()) + 
             ", " + std::to_string(img->height()) + ", " +
             std::to_string(img->depth()) + "]");
  
  // --------------------- Convert Input Images to OCV Format ----------------- 
  const cv::Mat img_ocv0 {arrows::ocv::image_container::vital_to_ocv( img->get_image() )};
  cv::Mat img_ocv;
  d->equalize_histogram(img_ocv0, img_ocv);
  
  // --------------------------------------------------------------------------
  
  // Convert back to an image_container_sptr
  vital::image_container_sptr img_out;
  img_out = std::make_shared<arrows::ocv::image_container>(img_ocv);
  
  push_to_port_using_trait( image, img_out );
  
  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time << " seconds");
}


// ------------------------------------------------------------------
void
equalize_histogram_process::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, required );
  
  // -- output --
  declare_output_port_using_trait( image, optional );
}


// ------------------------------------------------------------------
void
equalize_histogram_process::make_config()
{
  declare_config_using_trait( color_mode );
}
} //end namespace
