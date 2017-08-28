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
 * \brief Implementation of crop image process
 */

#include "stabilization_inspection_image_process.h"

#include <vital/vital_types.h>
#include <vital/types/vector.h>

#include <sprokit/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <arrows/ocv/image_container.h>

namespace kwiver {

// ----------------------------------------------------------------
/**
 * \class stabilization_inspection_image_process
 *
 * \brief Crops an image based on bounding box.
 *
 * \iport{image} The stabilized image to be processed for analysis.
 *
 * \oport{image} The processed image.
 *
 * \configs
 *
 * \config{buffer} The number of pixels to expand the input box. This
 * expansion is done on all four sides.
 */
create_config_trait( patch_width, int, "64", "Patch width" );
create_config_trait( patch_height, int, "64", "Patch height" );
create_config_trait( num_patches_wide, int, "10", "Number of patches in the horizontal direction" );
create_config_trait( num_patches_high, int, "10", "Number of patches in the vertical direction" );


class stabilization_inspection_image_process::priv
{
public:
  priv()
  { }

  ~priv()
  { }

  // Configuration values
  int m_num_patches_high, m_num_patches_wide, m_patch_height, m_patch_width;
  int m_buffer{1};

  /**
   * @brief Process image for stabilization quality analysis
   *
   * @param[in] image_data Input image to process
   *
   * @return The processed image
   */
  vital::image_container_sptr
  process( const vital::image_container_sptr image_data ) const
  {
    if ( image_data == NULL )
    {
      throw kwiver::vital::invalid_value( "Input image pointer is NULL." );
    }

    cv::Mat image = arrows::ocv::image_container::vital_to_ocv( image_data->get_image() );
    
    int cols_out = m_num_patches_wide*(m_patch_width + m_buffer);
    int rows_out = m_num_patches_high*(m_patch_height + m_buffer);
    
    cv::Mat processed_image;
    if( image.channels() == 3 )
    {
      processed_image = cv::Mat::zeros(cv::Size(cols_out, rows_out), 
                                       image.type());
    }
    else
    {
      processed_image = cv::Mat::zeros(cv::Size(cols_out, rows_out), 
                                       image.type());
    }
    
    // Copy patches over
    cv::Rect in_rect( 0, 0, m_patch_width, m_patch_height);
    cv::Rect out_rect( 0, 0, m_patch_width, m_patch_height);
    for( int i = 0; i < m_num_patches_high; i++ )
    {
      for( int j = 0; j < m_num_patches_wide; j++ )
      {
        in_rect.x = (float)(j)/m_num_patches_wide*(image.cols-m_patch_width);
        in_rect.y = (float)(i)/m_num_patches_high*(image.rows-m_patch_height);
        out_rect.x = j*(m_patch_width + m_buffer);
        out_rect.y = i*(m_patch_height + m_buffer);
        image(in_rect).copyTo(processed_image(out_rect));
      }
    }
    
    vital::image_container_sptr result( new arrows::ocv::image_container( processed_image ) );
    return result;
  }

};


// ==================================================================
stabilization_inspection_image_process
::stabilization_inspection_image_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new stabilization_inspection_image_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) );   // could use a better approach
  make_ports();
  make_config();
}


stabilization_inspection_image_process
::~stabilization_inspection_image_process()
{
}


// ------------------------------------------------------------------
void
stabilization_inspection_image_process
::_configure()
{
  d->m_patch_width = config_value_using_trait( patch_width );
  d->m_patch_height = config_value_using_trait( patch_height );
  d->m_num_patches_wide = config_value_using_trait( num_patches_wide );
  d->m_num_patches_high = config_value_using_trait( num_patches_high );
}


// ------------------------------------------------------------------
void
stabilization_inspection_image_process
::_step()
{
  auto img = grab_from_port_using_trait( image );

  auto result = d->process( img );

  push_to_port_using_trait( image, result );
}


// ------------------------------------------------------------------
void
stabilization_inspection_image_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, required );

  //output
  declare_output_port_using_trait( image, optional );
}


// ------------------------------------------------------------------
void
stabilization_inspection_image_process
::make_config()
{
  declare_config_using_trait( patch_width );
  declare_config_using_trait( patch_height );
  declare_config_using_trait( num_patches_wide );
  declare_config_using_trait( num_patches_high );
}

} //end namespace
