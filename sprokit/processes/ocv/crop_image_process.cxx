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
 * \brief Implementation of crop image process
 */

#include "crop_image_process.h"

#include <vital/vital_types.h>
#include <vital/types/vector.h>

#include <sprokit/processes/kwiver_type_traits.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <arrows/ocv/image_container.h>
#include <arrows/ocv/image_container.h>

namespace kwiver {

// ----------------------------------------------------------------
/**
 * \class crop_image_process
 *
 * \brief Crops an image based on bounding box.
 *
 * \iports
 *
 * \iport{bounding_box} Box describing the portion of the image to be cropped.
 *
 * \iport{image} The image to be cropped.
 *
 * \oports
 *
 * \oport{image} The cropped portion of the input image.
 *
 * \oport{bounding_box} Bounding box is updated to the actual area
 * cropped. The box may be expanded by a configured boarder.
 *
 * \configs
 *
 * \config{buffer} The number of pixels to expand the input box. This
 * expansion is done on all four sides.
 */

create_config_trait( buffer, int, "5", "buffer in pixels around bounding box" );

class crop_image_process::priv
{
public:
  priv()
  { }

  ~priv()
  { }

  // Configuration values
  int m_buffer;

  /**
   * @brief Crop image to bounding box
   *
   * @param[in] image_data Input image to crop
   * @param[in,out] bbox Area to crop from image. Box updated.
   *
   * @return The cropped portion of the image.
   */
  vital::image_container_sptr crop( const vital::image_container_sptr     image_data,
                                    vital::detected_object::bounding_box& bbox ) const
  {
    if ( image_data == NULL )
    {
      throw kwiver::vital::invalid_value("Input image pointer is NULL.");
    }

    if ( bbox.area() == 0 )
    {
      throw kwiver::vital::invalid_value("Bounding box contains no pixels.");
    }

    cv::Mat image = arrows::ocv::image_container::vital_to_ocv( image_data->get_image() );
    vital::vector_2d buf( this->m_buffer, this->m_buffer );
    vital::detected_object::bounding_box expanded( bbox.upper_left() - buf,
                                                   bbox.lower_right() + buf );
    vital::detected_object::bounding_box img_bound( vital::vector_2d( 0, 0 ),
                                                    vital::vector_2d( image_data->width(),
                                                                      image_data->height() ) );
    vital::detected_object::bounding_box cb = img_bound.intersection( expanded );
    if ( cb.area() == 0 )
    {
      // Is this not an error? Throw something.
      return NULL;
    }

    bbox = cb;
    cv::Mat cropedImage = image( cv::Rect( cb.upper_left()[0], cb.upper_left()[1],
                                           cb.width(), cb.height() ) ).clone();
    vital::image_container_sptr result( new arrows::ocv::image_container( cropedImage ) );
    return result;
  }

};


// ==================================================================
crop_image_process
::crop_image_process( vital::config_block_sptr const& config )
  : process( config ),
  d( new crop_image_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) );   // could use a better approach
  make_ports();
  make_config();
}


crop_image_process
::~crop_image_process()
{
}


// ------------------------------------------------------------------
void
crop_image_process
::_configure()
{
  d->m_buffer = config_value_using_trait( buffer );
}


// ------------------------------------------------------------------
void
crop_image_process
::_step()
{
  vital::image_container_sptr img = grab_from_port_using_trait( image );
  vital::detected_object::bounding_box bbox = grab_from_port_using_trait( bounding_box );

  vital::image_container_sptr result = d->crop( img, bbox );

  push_to_port_using_trait( image, result );
  push_to_port_using_trait( bounding_box, bbox );
}


// ------------------------------------------------------------------
void
crop_image_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t optional;

  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( bounding_box, required );
  declare_input_port_using_trait( image, required );

  //output
  declare_output_port_using_trait( image, optional );
  declare_output_port_using_trait( bounding_box, optional );
}


// ------------------------------------------------------------------
void
crop_image_process
::make_config()
{
  declare_config_using_trait( buffer );
}


} //end namespace
