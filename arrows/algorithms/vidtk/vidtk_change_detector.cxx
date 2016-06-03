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
 * \brief Implementation of vidtk change detector.
 */

#include <object_detectors/detector_factory.h>

// This is not ideal, but there are the same macro definitions from
// vidtk that conflict with those from vital. We want vital logging
// here, so we undef the vidtk symbol(s)
#undef LOG_ERROR

#include "vidtk_change_detector.h"

#include <vital/types/vector.h>
#include <vital/config/config_block_exception.h>

#include <arrows/algorithms/vxl/image_container.h>
#include <arrows/algorithms/ocv/image_container.h>

namespace kwiver {
namespace arrows {
namespace vidtk {

class vidtk_change_detector::priv
{
public:
// =====================================================================
  priv()
    : m_factory( "detector_factory" ),
      m_frame_number( 0 )
  {
  }

  // copy CTOR
  priv( priv const& other )
    : m_factory( other.m_factory ),
      m_detector( other.m_detector ),
      m_config_filename( other.m_config_filename ),
      m_frame_number( other.m_frame_number ),
      m_labels( other.m_labels )
  {
  }


  ~priv()
  {
  }


  ::vidtk::detector_factory< vxl_byte > m_factory;
  ::vidtk::process_smart_pointer< ::vidtk::detector_super_process< vxl_byte > > m_detector;
  std::string m_config_filename;
  unsigned int m_frame_number;
  vital::object_labels_sptr m_labels;
};

vidtk_change_detector
::vidtk_change_detector()
  : d( new priv() )
{
  attach_logger( "vidtk_change_detector" );
}


vidtk_change_detector
::vidtk_change_detector( vidtk_change_detector const& frd )
  : d( new priv( *frd.d ) )
{
  attach_logger( "vidtk_change_detector" );
}


vidtk_change_detector
::~vidtk_change_detector()
{
}


// --------------------------------------------------------------------
vital::config_block_sptr
vidtk_change_detector
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "config_file", d->m_config_filename,  "config file for vidtk" );
  return config;
}


// --------------------------------------------------------------------
void
vidtk_change_detector
::set_configuration( vital::config_block_sptr config_in )
{
  // Get default parameters from detector factory
  // Note that we are bridging vidtk config and vital config here.
  ::vidtk::config_block vidtkConfig = d->m_factory.params();
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );

  // Look for config entry that specified our config file name
  this->d->m_config_filename  = config->get_value< std::string > ( "config_file" );
  if ( this->d->m_config_filename.empty() )
  {
    throw kwiver::vital::no_such_configuration_value_exception( "config_file" );
  }

  vidtkConfig.parse( this->d->m_config_filename );

  ::vidtk::config_block block;
  block.add_subblock( vidtkConfig, "detector_factory" );
  d->m_detector = d->m_factory.create_detector( block ); // Create detector based on config
  d->m_detector->set_params( vidtkConfig );              // give it its parameters
  d->m_detector->initialize();                           // Do post-config initialization

  this->d->m_labels = vital::object_labels_sptr( new vital::object_labels( std::vector< std::string > ( 1, "motion" ) ) );
}


// --------------------------------------------------------------------
bool
vidtk_change_detector
::check_configuration( vital::config_block_sptr config ) const
{
  std::string fname = config->get_value< std::string > ( "config_file" );
  LOG_ERROR( m_logger, "Required configuration item \"config_file\" is missing." );

  return (! fname.empty());
}


// --------------------------------------------------------------------
vital::detected_object_set_sptr
vidtk_change_detector
::detect( vital::image_container_sptr image_data ) const
{
  if ( ! image_data )
  {
    throw kwiver::vital::invalid_value("Input image pointer is NULL");
  }

  vil_image_view< vxl_byte > img = kwiver::arrows::vxl::image_container::vital_to_vxl( image_data->get_image() );
  ::vidtk::timestamp ts( d->m_frame_number * 0.1, d->m_frame_number );
  d->m_frame_number++;
  vil_image_view< bool > mask;
  ::vidtk::image_to_image_homography i2i;
  i2i.set_identity( true );
  ::vidtk::image_to_plane_homography i2p;
  ::vidtk::image_to_utm_homography i2u;
  ::vidtk::plane_to_image_homography p2i;
  ::vidtk::plane_to_utm_homography p2u;
  ::vidtk::image_to_plane_homography i2p2;
  ::vidtk::video_modality modality;
  ::vidtk::shot_break_flags sbf;
  ::vidtk::gui_frame_info gui;

  d->m_detector->input_image( img );
  d->m_detector->input_timestamp( ts );
  d->m_detector->input_mask_image( mask );
  d->m_detector->input_src_to_ref_homography( i2i );
  d->m_detector->input_src_to_wld_homography( i2p );
  d->m_detector->input_src_to_utm_homography( i2u );
  d->m_detector->input_wld_to_src_homography( p2i );
  d->m_detector->input_wld_to_utm_homography( p2u );
  d->m_detector->input_ref_to_wld_homography( i2p2 );
  d->m_detector->input_world_units_per_pixel( 0.5 );
  d->m_detector->input_video_modality( modality );
  d->m_detector->input_shot_break_flags( sbf );
  d->m_detector->input_gui_feedback( gui );
  d->m_detector->step2();

  std::vector< ::vidtk::image_object_sptr > image_objects = d->m_detector->output_image_objects();

  std::vector< vital::detected_object_sptr > detected_objects;
  for ( unsigned i = 0; i < image_objects.size(); ++i )
  {
    vgl_box_2d< unsigned > const& bbox = image_objects[i]->get_bbox();
    vital::detected_object::bounding_box pbox( vital::vector_2d( bbox.min_x(), bbox.min_y() ), vital::vector_2d( bbox.max_x(), bbox.max_y() ) );
    vital::object_type_sptr classification( new  vital::object_type( this->d->m_labels, std::vector< double > ( 1, 1 ) ) );
    detected_objects.push_back( vital::detected_object_sptr( new vital::detected_object( pbox, 1.0, classification ) ) );
  }

  return vital::detected_object_set_sptr( new vital::detected_object_set( detected_objects, this->d->m_labels ) );
} // vidtk_change_detector::detect

} } }     //end namespace
