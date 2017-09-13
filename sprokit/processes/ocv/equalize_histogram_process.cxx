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
  
  kwiver::vital::wall_timer m_timer;
  
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
  cv::Mat img_ocv;
  const cv::Mat img_ocv0 {arrows::ocv::image_container::vital_to_ocv( img->get_image() )};
  if( img->depth() == 1 )
  {
    cv::equalizeHist( img_ocv0, img_ocv );
  }
  else if( img->depth() == 3 )
  {
    cv::Mat RGB_3[3];
    cv::split( img_ocv0, RGB_3 );
    for( size_t i=0; i < 3; ++i)
    {
      cv::equalizeHist( RGB_3[i], RGB_3[i] );
    }
    cv::merge( RGB_3, 3, img_ocv );
  }
  else
  {
    throw vital::invalid_data( "Image must have 1 or 3 channels" );
  }
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
}

} //end namespace
