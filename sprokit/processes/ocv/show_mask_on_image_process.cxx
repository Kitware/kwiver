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
 * \brief Implementation of show_mask_on_image_process
 */

#include "show_mask_on_image_process.h"

#include <vital/vital_types.h>
#include <vital/vital_foreach.h>
#include <vital/util/wall_timer.h>
#include <vital/exceptions/image.h>

#include <arrows/ocv/image_container.h>

#include <sprokit/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <sstream>
#include <iostream>

namespace kwiver {
  
typedef  Eigen::Matrix< unsigned int, 3, 1 > ColorVector;

create_port_trait( mask, image, "Mask image" );

create_config_trait( color_mode, std::string, "RGB", "Describes the "
                     "channel color ordering of the input image: RGB or BGR." );
create_config_trait( mask_color, std::string, "255 0 0", "The RGB color for the "
                     "mask. The default is 255 0 0." );
create_config_trait( scaling, double, "1", "Scaling factor applied to the "
                     "image before superimposing the mask. Values other than 1 "
                     "currently slow down the processing appreciably." );
create_config_trait( alpha, double, "0.9", "Fraction of masked color to "
                     "blend with image (0-1)." );


static std::string type2str(int type) 
{
  std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}


// ==================================================================
class show_mask_on_image_process::priv
{
public:
  std::string m_color_mode;
  ColorVector m_mask_color;
  double m_scaling;
  double m_alpha;
  kwiver::vital::logger_handle_t m_logger;
  kwiver::vital::wall_timer m_timer;
  
  priv()
    : m_color_mode("RGB"),
      m_scaling(1),
      m_alpha(0.9)
  {
  }

  ~priv()
  {
  }
  
  /// Superimpose mask on image
  /**
   *
   * \param[in] img1   the image
   * \param[in] img2   the mask
   * \return           the superimposed image
   */
  cv::Mat
  superimpose_mask(cv::Mat const image, cv::Mat const mask)
  {
    LOG_TRACE( m_logger, "Superimposing mask ([" + std::to_string(mask.cols) + 
               ", " + std::to_string(mask.rows) + ", " +
               std::to_string(mask.channels()) + "], " + 
               type2str(mask.type()) + ") on image ([" + 
               std::to_string(image.cols) + ", " + std::to_string(image.rows) + 
               ", " + std::to_string(image.channels()) + "], " + 
               type2str(image.type()) + ")");
    
    assert( image.channels() == 3 );
    assert( mask.channels() == 1 );
    
    if( (mask.rows != image.rows) || mask.cols != image.cols )
    {
      throw vital::image_size_mismatch_exception("Mask dimensions do not match "
                                                 "the image dimensions", 
                                                 image.cols, image.rows, 
                                                 mask.cols, mask.rows);
    }
    
    cv::Mat image_out;
    if( m_scaling != 1 )
    {
      image_out = image*m_scaling;
    }
    else
    {
      image.copyTo(image_out);
    }
    
    //kwiver::vital::wall_timer timer;
    //timer.start();
    if( true )
    {
      image_out.setTo(cv::Scalar(m_mask_color[2],m_mask_color[1],m_mask_color[0]), mask);
      
      if( m_alpha != 1 )
      {
        cv::addWeighted(image_out, m_alpha, image, 1-m_alpha, 0.0, image_out, -1);
      }
    }
    else
    {
      // Loop over each pixel. Possibly could be optimized beyond the 
      // above approach.
      double a = m_alpha;
      double b = 1-m_alpha;
      for(int i=0; i<image.rows; i++)
      {
        for(int j=0; j<image.cols; j++) 
        {
          if( mask.at<uchar>(i,j) > 0 )
          {
            // OCV image is BGR, so invert the RGB mask color order
            image_out.at<cv::Vec3b>(i,j)[0] = a*m_mask_color[2] + b*image_out.at<cv::Vec3b>(i,j)[0];
            image_out.at<cv::Vec3b>(i,j)[1] = a*m_mask_color[1] + b*image_out.at<cv::Vec3b>(i,j)[1];
            image_out.at<cv::Vec3b>(i,j)[2] = a*m_mask_color[0] + b*image_out.at<cv::Vec3b>(i,j)[2];
          }
        }
      }
    }
    //timer.stop();
    //LOG_DEBUG( m_logger, "Loop over pixels processing time: " << timer.elapsed() << " seconds");
    
    return image_out;
  }

}; // end priv class


// ==================================================================
show_mask_on_image_process
::show_mask_on_image_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new show_mask_on_image_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach
  d->m_logger = logger();
  make_ports();
  make_config();
}


show_mask_on_image_process
  ::~show_mask_on_image_process()
{
}


// ------------------------------------------------------------------
void
show_mask_on_image_process::_configure()
{
  {
    std::stringstream lss( config_value_using_trait( mask_color ) );
    lss >> d->m_mask_color[0] >> d->m_mask_color[1] >> d->m_mask_color[2];
  }
  
  d->m_scaling            = config_value_using_trait( scaling );
  d->m_alpha              = config_value_using_trait( alpha );
  
  LOG_DEBUG( d->m_logger, "mask_color: (" + std::to_string(d->m_mask_color[0]) +
             ", " + std::to_string(d->m_mask_color[1]) + ", " + 
             std::to_string(d->m_mask_color[2]) + ")");
  LOG_DEBUG( d->m_logger, "scaling: " + std::to_string(d->m_scaling));
  LOG_DEBUG( d->m_logger, "alpha: " + std::to_string(d->m_alpha));
} // show_mask_on_image_process::_configure


// ------------------------------------------------------------------
void
show_mask_on_image_process::_step()
{
  d->m_timer.start();
  
  // Get input images
  kwiver::vital::image_container_sptr img = grab_from_port_using_trait( image );
  kwiver::vital::image_container_sptr mask = grab_from_port_using_trait( mask );
  
  LOG_TRACE( d->m_logger, "Received image ([" + std::to_string(img->width()) + 
             ", " + std::to_string(img->height()) + ", " +
             std::to_string(img->depth()) + "] with mask ([" + 
             std::to_string(mask->width()) + ", " + std::to_string(mask->height()) + 
             ", " + std::to_string(mask->depth()) + "]");
  
  if( (img->width() != mask->width()) || img->height() != mask->height() )
    {
      throw vital::image_size_mismatch_exception("Mask dimensions do not match"
                                                 "the image dimensions", 
                                                 mask->width(), mask->height(), 
                                                 img->width(), img->height());
    }
  
  // --------------------- Convert Input Images to OCV Format ----------------- 
  cv::Mat img_ocv, mask_ocv;
  if( img->depth() == 1 )
  {
    cv::Mat img_gray = arrows::ocv::image_container::vital_to_ocv( img->get_image() );
    cvtColor( img_gray, img_ocv, CV_GRAY2RGB );
  }
  else
  {
    if( d->m_color_mode == "BGR" )
    {
      img_ocv = arrows::ocv::image_container::vital_to_ocv( img->get_image(), 
                                                            arrows::ocv::image_container::BGR );
    }
    else if( d->m_color_mode == "RGB" )
    {
      img_ocv = arrows::ocv::image_container::vital_to_ocv( img->get_image(), 
                                                            arrows::ocv::image_container::RGB );
    }
    else
    {
      throw vital::invalid_value( "Invalid image one color mode: " + 
                                  d->m_color_mode + "!" );
    }
  }
  mask_ocv = arrows::ocv::image_container::vital_to_ocv( mask->get_image() );
  // --------------------------------------------------------------------------
  
  // Get the blended OCV image
  cv::Mat img_out_ocv = d->superimpose_mask(img_ocv, mask_ocv);
  vital::image_container_sptr img_out( new arrows::ocv::image_container( img_out_ocv ) );

  push_to_port_using_trait( image, img_out );
  
  d->m_timer.stop();
  double elapsed_time = d->m_timer.elapsed();
  LOG_DEBUG( d->m_logger, "Total processing time: " << elapsed_time << " seconds");
}


// ------------------------------------------------------------------
void
show_mask_on_image_process::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, required );
  declare_input_port_using_trait( mask, required );

  // -- output --
  declare_output_port_using_trait( image, required );
}


// ------------------------------------------------------------------
void
show_mask_on_image_process::make_config()
{
  declare_config_using_trait( color_mode );
  declare_config_using_trait( mask_color );
  declare_config_using_trait( scaling );
  declare_config_using_trait( alpha );
}

} //end namespace
