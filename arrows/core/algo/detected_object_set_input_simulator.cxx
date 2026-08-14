// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation for detected_object_set_input_simulator

#include "detected_object_set_input_simulator.h"

#include <vital/exceptions.h>
#include <vital/util/data_stream_reader.h>
#include <vital/util/tokenize.h>

#include <cstdlib>
#include <sstream>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class detected_object_set_input_simulator::priv
{
public:
  priv( detected_object_set_input_simulator& parent )
    : parent( parent ),
      m_frame_ct( 0 )
  {}

  detected_object_set_input_simulator& parent;
  // configuration values
  double c_center_x() { return parent.c_center_x; }
  double c_center_y() { return parent.c_center_y; }
  double c_height() { return parent.c_height; }
  double c_width() { return parent.c_width; }
  double c_dx() { return parent.c_dx; }
  double c_dy() { return parent.c_dy; }
  int c_max_sets() { return parent.c_max_sets; }
  int c_set_size() { return parent.c_set_size; }
  std::string c_detection_class() { return parent.c_detection_class; }
  std::string c_image_name() { return parent.c_image_name; }

  ~priv() {}

  // --------------------------------------------------------------------------
  // frame counter - not in the configuration parameters
  int m_frame_ct;
};

// ----------------------------------------------------------------------------
void
detected_object_set_input_simulator
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.core.detected_object_set_input_simulator" );
}

detected_object_set_input_simulator::
~detected_object_set_input_simulator()
{}

// ----------------------------------------------------------------------------
bool
detected_object_set_input_simulator
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
detected_object_set_input_simulator
::open( [[maybe_unused]] std::string const& filename )
{}

// ----------------------------------------------------------------------------
bool
detected_object_set_input_simulator
::read_set(
  kwiver::vital::detected_object_set_sptr& detected_set,
  std::string& image_name )
{
  if( d->m_frame_ct >= d->c_max_sets() )
  {
    return false;
  }

  detected_set = std::make_shared< kwiver::vital::detected_object_set >();

  for( int i = 0; i < d->c_set_size(); ++i )
  {
    double ct_adj = d->m_frame_ct + static_cast< double >( i ) /
                    d->c_set_size();

    kwiver::vital::bounding_box_d bbox(
      d->c_center_x() + ct_adj * d->c_dx() - d->c_width() / 2.0,
      d->c_center_y() + ct_adj * d->c_dy() - d->c_height() / 2.0,
      d->c_center_x() + ct_adj * d->c_dx() + d->c_width() / 2.0,
      d->c_center_y() + ct_adj * d->c_dy() + d->c_height() / 2.0 );

    auto dot = std::make_shared< kwiver::vital::detected_object_type >();
    dot->set_score( d->c_detection_class(), 1.0 );

    detected_set->add(
      std::make_shared< kwiver::vital::detected_object >(
        bbox,
        1.0, dot ) );
  }

  ++d->m_frame_ct;

  image_name = d->c_image_name();

  return true;
}

} // namespace core

} // namespace arrows

}     // end namespace
