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
 * \brief Implementation of cv::cvtColor
 */

#include "cvt_color_process.h"

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

  create_config_trait( code, std::string, "", "Conversion code string. See "
                       "OpenCv documentation for function cv::cvtColor for "
                       "more details. Valid options are: 'BGR2GRAY',"
                       "'RGB2GRAY', 'GRAY2BGR', 'GRAY2RGB','BGR2XYZ'"
                       "'RGB2XYZ', 'XYZ2BGR', 'XYZ2RGB', 'BGR2YCrCb'"
                       "'RGB2YCrCb', 'YCrCb2BGR', 'YCrCb2RGB', 'BGR2HSV',"
                       "'RGB2HSV', 'HSV2BGR', 'HSV2RGB', 'BGR2HLS', 'RGB2HLS', "
                       "'HLS2BGR', 'HLS2RGB', 'BGR2Lab', 'RGB2Lab', 'Lab2BGR', "
                       "'Lab2RGB', 'BGR2Luv', 'RGB2Luv', 'Luv2BGR', 'Luv2RGB', "
                       "'BayerBG2BGR', 'BayerGB2BGR', 'BayerRG2BGR', "
                       "'BayerGR2BGR', 'BayerBG2RGB', 'BayerGB2RGB', "
                       "'BayerRG2RGB', 'BayerGR2RGB'." );
  
// ==================================================================
class cvt_color_process::priv
{
public:
    
  priv()
  {
  }

  ~priv()
  {
  }
  
  int m_conversion_code;
  kwiver::vital::wall_timer m_timer;
  
  void
  set_conversion_code(const std::string code_str)
  {
    if( code_str == "BGR2GRAY" )
    {
      m_conversion_code = CV_BGR2GRAY;
    }
    else if( code_str == "RGB2GRAY" )
    {
      m_conversion_code = CV_RGB2GRAY;
    }
    else if( code_str == "GRAY2BGR" )
    {
      m_conversion_code = CV_GRAY2BGR;
    }
    else if( code_str == "GRAY2RGB" )
    {
      m_conversion_code = CV_GRAY2RGB;
    }
    else if( code_str == "BGR2XYZ" )
    {
      m_conversion_code = CV_BGR2XYZ;
    }
    else if( code_str == "RGB2XYZ" )
    {
      m_conversion_code = CV_RGB2XYZ;
    }
    else if( code_str == "XYZ2BGR" )
    {
      m_conversion_code = CV_XYZ2BGR;
    }
    else if( code_str == "XYZ2RGB" )
    {
      m_conversion_code = CV_XYZ2RGB;
    }
    else if( code_str == "BGR2YCrCb" )
    {
      m_conversion_code = CV_BGR2YCrCb;
    }
    else if( code_str == "RGB2YCrCb" )
    {
      m_conversion_code = CV_RGB2YCrCb;
    }
    else if( code_str == "YCrCb2BGR" )
    {
      m_conversion_code = CV_YCrCb2BGR;
    }
    else if( code_str == "YCrCb2RGB" )
    {
      m_conversion_code = CV_YCrCb2RGB;
    }
    else if( code_str == "BGR2HSV" )
    {
      m_conversion_code = CV_BGR2HSV;
    }
    else if( code_str == "RGB2HSV" )
    {
      m_conversion_code = CV_RGB2HSV;
    }
    else if( code_str == "HSV2BGR" )
    {
      m_conversion_code = CV_HSV2BGR;
    }
    else if( code_str == "HSV2RGB" )
    {
      m_conversion_code = CV_HSV2RGB;
    }
    else if( code_str == "BGR2HLS" )
    {
      m_conversion_code = CV_BGR2HLS;
    }
    else if( code_str == "RGB2HLS" )
    {
      m_conversion_code = CV_RGB2HLS;
    }
    else if( code_str == "HLS2BGR" )
    {
      m_conversion_code = CV_HLS2BGR;
    }
    else if( code_str == "HLS2RGB" )
    {
      m_conversion_code = CV_HLS2RGB;
    }
    else if( code_str == "BGR2Lab" )
    {
      m_conversion_code = CV_BGR2Lab;
    }
    else if( code_str == "RGB2Lab" )
    {
      m_conversion_code = CV_RGB2Lab;
    }
    else if( code_str == "Lab2BGR" )
    {
      m_conversion_code = CV_Lab2BGR;
    }
    else if( code_str == "Lab2RGB" )
    {
      m_conversion_code = CV_Lab2RGB;
    }
    else if( code_str == "BGR2Luv" )
    {
      m_conversion_code = CV_BGR2Luv;
    }
    else if( code_str == "RGB2Luv" )
    {
      m_conversion_code = CV_RGB2Luv;
    }
    else if( code_str == "Luv2BGR" )
    {
      m_conversion_code = CV_Luv2BGR;
    }
    else if( code_str == "Luv2RGB" )
    {
      m_conversion_code = CV_Luv2RGB;
    }
    else if( code_str == "BayerBG2BGR" )
    {
      m_conversion_code = CV_BayerBG2BGR;
    }
    else if( code_str == "BayerGB2BGR" )
    {
      m_conversion_code = CV_BayerGB2BGR;
    }
    else if( code_str == "BayerRG2BGR" )
    {
      m_conversion_code = CV_BayerRG2BGR;
    }
    else if( code_str == "BayerGR2BGR" )
    {
      m_conversion_code = CV_BayerGR2BGR;
    }
    else if( code_str == "BayerBG2RGB" )
    {
      m_conversion_code = CV_BayerBG2RGB;
    }
    else if( code_str == "BayerGB2RGB" )
    {
      m_conversion_code = CV_BayerGB2RGB;
    }
    else if( code_str == "BayerRG2RGB" )
    {
      m_conversion_code = CV_BayerRG2RGB;
    }
    else if( code_str == "BayerGR2RGB" )
    {
      m_conversion_code = CV_BayerGR2RGB;
    }
    else
    {
      throw vital::invalid_data( "code: '" + code_str + "' not recognized." );
    }
  }
  
  
  void
  convert(const cv::Mat &src, cv::Mat &dst)
  {
    if( src.channels() <= 3 )
    {
      cv::cvtColor(src, dst, m_conversion_code);
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
cvt_color_process
::cvt_color_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new cvt_color_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
  make_ports();
  make_config();
}


cvt_color_process
  ::~cvt_color_process()
{
}


// ------------------------------------------------------------------
void
cvt_color_process::_configure()
{
  std::string code_str = config_value_using_trait( code );
  d->set_conversion_code( code_str );
  LOG_DEBUG( logger(), "Conversion code: " + code_str );
} // cvt_color_process::_configure


// ------------------------------------------------------------------
void
cvt_color_process::_step()
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
  d->convert(img_ocv0, img_ocv);
  
  // --------------------------------------------------------------------------
  
  // Convert back to an image_container_sptr
  vital::image_container_sptr img_out;
  img_out = std::make_shared<arrows::ocv::image_container>(img_ocv);
  
  push_to_port_using_trait( image, img_out );
  
  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( logger(), "Total processing time: " << elapsed_time << " seconds");
}


// --------------z----------------------------------------------------
void
cvt_color_process::make_ports()
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
cvt_color_process::make_config()
{
  declare_config_using_trait( code );
}
} //end namespace
