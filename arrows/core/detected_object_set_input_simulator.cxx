// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation for detected_object_set_input_simulator
 */

#include "detected_object_set_input_simulator.h"

#include <vital/exceptions.h>
#include <vital/util/data_stream_reader.h>
#include <vital/util/tokenize.h>

#include <cstdlib>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace core {

// ------------------------------------------------------------------
class detected_object_set_input_simulator::priv
{
public:

  priv( detected_object_set_input_simulator* parent )
    : m_parent( parent ),
      m_center_x( 100.0 ),
      m_center_y( 100.0 ),
      m_height( 200.0 ),
      m_width( 200.0 ),
      m_dx( 0.0 ),
      m_dy( 0.0 ),
      m_frame_ct( 0 ),
      m_max_sets( 10 ),
      m_set_size( 4 ),
      m_detection_class( "detection" )
  {
  }

  ~priv() {}

  // -------------------------------------
  detected_object_set_input_simulator* m_parent;

  double m_center_x;
  double m_center_y;
  double m_height;
  double m_width;
  double m_dx;
  double m_dy;
  int m_frame_ct;
  int m_max_sets;
  int m_set_size;
  std::string m_detection_class;
  std::string m_image_name;
};

// ==================================================================
detected_object_set_input_simulator
::detected_object_set_input_simulator()
  : d( new detected_object_set_input_simulator::priv( this ) )
{
  attach_logger( "arrows.core.detected_object_set_input_simulator" );
}

detected_object_set_input_simulator::
~detected_object_set_input_simulator()
{
}

// ------------------------------------------------------------------
vital::config_block_sptr
detected_object_set_input_simulator
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "center_x", d->m_center_x,
                     "Bounding box center x coordinate." );
  config->set_value( "center_y", d->m_center_y,
                     "Bounding box center y coordinate." );
  config->set_value( "height", d->m_height, "Bounding box height." );
  config->set_value( "width", d->m_width, "Bounding box width." );
  config->set_value( "dx", d->m_dx, "Bounding box x translation per frame." );
  config->set_value( "dy", d->m_dy, "Bounding box y translation per frame." );
  config->set_value( "max_sets", d->m_max_sets,
                     "Number of detection sets to generate." );
  config->set_value( "set_size", d->m_set_size,
                     "Number of detection in a set." );
  config->set_value( "detection_class", d->m_detection_class,
                     "Label for detection detected object type" );
  config->set_value( "image_name", d->m_image_name,
                     "Image name to return with each detection set" );

  return config;
}

// ------------------------------------------------------------------
void
detected_object_set_input_simulator
::set_configuration( vital::config_block_sptr config_in )
{
  auto config = get_configuration();
  config->merge_config( config_in );

  d->m_center_x     = config->get_value< double >( "center_x" );
  d->m_center_y     = config->get_value< double >( "center_y" );
  d->m_height       = config->get_value< double >( "height" );
  d->m_width        = config->get_value< double >( "width" );
  d->m_dx           = config->get_value< double >( "dx" );
  d->m_dy           = config->get_value< double >( "dy" );
  d->m_max_sets     = config->get_value< double >( "max_sets" );
  d->m_set_size     = config->get_value< double >( "set_size" );
  d->m_detection_class     = config->get_value< std::string >(
    "detection_class" );
  d->m_image_name   = config->get_value< std::string >( "image_name" );
}

// ------------------------------------------------------------------
bool
detected_object_set_input_simulator
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
detected_object_set_input_simulator
::open( VITAL_UNUSED std::string const& filename )
{
}

// ------------------------------------------------------------------
bool
detected_object_set_input_simulator
::read_set( kwiver::vital::detected_object_set_sptr& detected_set,
            std::string& image_name )
{
  if( d->m_frame_ct >= d->m_max_sets )
  {
    return false;
  }

  detected_set = std::make_shared< kwiver::vital::detected_object_set >();

  for( int i = 0; i < d->m_set_size; ++i )
  {
    double ct_adj = d->m_frame_ct + static_cast< double >( i ) / d->m_set_size;

    kwiver::vital::bounding_box_d bbox(
      d->m_center_x + ct_adj * d->m_dx - d->m_width / 2.0,
      d->m_center_y + ct_adj * d->m_dy - d->m_height / 2.0,
      d->m_center_x + ct_adj * d->m_dx + d->m_width / 2.0,
      d->m_center_y + ct_adj * d->m_dy + d->m_height / 2.0 );

    auto dot = std::make_shared< kwiver::vital::detected_object_type >();
    dot->set_score( d->m_detection_class, 1.0 );

    detected_set->add( std::make_shared< kwiver::vital::detected_object >(
                         bbox,
                         1.0,
                         dot ) );
  }

  ++d->m_frame_ct;

  image_name = d->m_image_name;

  return true;
}

} // namespace core

} // namespace arrows

}     // end namespace
