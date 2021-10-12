/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#include "refine_detections_add_fixed.h"

namespace kwiver {
namespace arrows {
namespace core {

/// Private implementation class
class refine_detections_add_fixed::priv
{
public:

  /// Constructor
  priv()
  : add_full_image_detection( true )
  , detection_type( "generic_object_proposal" )
  {
  }

  /// Destructor
  ~priv()
  {
  }

  /// Parameters
  bool add_full_image_detection;
  std::string detection_type;
};


/// Constructor
refine_detections_add_fixed
::refine_detections_add_fixed()
: d_( new priv() )
{
}


/// Destructor
refine_detections_add_fixed
::~refine_detections_add_fixed()
{
}


/// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
refine_detections_add_fixed
::get_configuration() const
{
  vital::config_block_sptr config = vital::algo::refine_detections::get_configuration();

  config->set_value( "add_full_image_detection", d_->add_full_image_detection,
                     "Add full image detection of the same size as the input image." );
  config->set_value( "detection_type", d_->detection_type,
                     "Object type to add to newly created detected objects" );

  return config;
}


/// Set this algorithm's properties via a config block
void
refine_detections_add_fixed
::set_configuration( vital::config_block_sptr in_config )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d_->add_full_image_detection = config->get_value<bool>( "add_full_image_detection" );
  d_->detection_type = config->get_value<std::string>( "detection_type" );
}

/// Check that the algorithm's currently configuration is valid
bool
refine_detections_add_fixed
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}

// ----------------------------------------------------------------------------------
vital::detected_object_set_sptr
refine_detections_add_fixed
::refine( vital::image_container_sptr image_data,
          vital::detected_object_set_sptr detections ) const
{
  vital::detected_object_set_sptr output = detections->clone();

  if( d_->add_full_image_detection &&
      image_data->height() > 0 && image_data->width() > 0 )
  {
    kwiver::vital::bounding_box_d det_box( 0, 0,
                                           image_data->width(),
                                           image_data->height() );

    if( !d_->detection_type.empty() )
    {
      auto dot = std::make_shared< kwiver::vital::detected_object_type >();
      dot->set_score( d_->detection_type, 1.0 );

      output->add(
        std::make_shared< kwiver::vital::detected_object >(
          det_box, 1.0, dot ) );
    }
    else
    {
      output->add(
        std::make_shared< kwiver::vital::detected_object >(
          det_box, 1.0 ) );
    }
  }

  return output;
}

}}} // end namespace kwiver
