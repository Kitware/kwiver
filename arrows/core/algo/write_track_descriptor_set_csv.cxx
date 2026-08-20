// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of track descriptor set csv output

#include "write_track_descriptor_set_csv.h"

#include <vital/vital_config.h>

#include <time.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class write_track_descriptor_set_csv::priv
{
public:
  priv( write_track_descriptor_set_csv& parent )
    : parent( parent ),
      m_first( true ),
      m_delim( "," ),
      m_sub_delim( " " )
  {}

  write_track_descriptor_set_csv& parent;

  // Configuration values
  bool c_write_raw_descriptor() { return parent.c_write_raw_descriptor; }
  bool c_write_world_loc() { return parent.c_write_world_loc; }

  ~priv() {}

  // Local values
  bool m_first;
  std::string m_delim;
  std::string m_sub_delim;
};

// ----------------------------------------------------------------------------
void
write_track_descriptor_set_csv
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.core.write_track_descriptor_set_csv" );
}

write_track_descriptor_set_csv
::~write_track_descriptor_set_csv()
{}

// ----------------------------------------------------------------------------
bool
write_track_descriptor_set_csv
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
write_track_descriptor_set_csv
::write_set(
  const kwiver::vital::track_descriptor_set_sptr set,
  [[maybe_unused]] const std::string& source_id )
{
  if( d->m_first )
  {
    // Write file header(s)
    stream() << "# 1:descriptor_uid, "
             << "2:descriptor_type, "
             << "3:track_reference_size, "
             << "4:track_references, "
             << "5:descriptor_size, "
             << "6:descriptor_data_vector, "
             << "7:history_size, "
             << "8:history_vector"
             << std::endl;

    d->m_first = false;
  }

  // Get detections from set
  for( auto desc : *set )
  {
    if( !desc )
    {
      continue;
    }

    stream() << desc->get_uid().value() << d->m_delim;
    stream() << desc->get_type() << d->m_delim;

    stream() << desc->get_track_ids().size() << d->m_delim;
    for( auto id : desc->get_track_ids() )
    {
      stream() << id << d->m_sub_delim;
    }
    stream() << d->m_delim;

    if( d->c_write_raw_descriptor() )
    {
      auto raw_sptr = desc->get_descriptor();
      stream() << raw_sptr->size() << d->m_delim;
      for( auto* value_ptr = raw_sptr->raw_data();
           value_ptr != raw_sptr->raw_data() + raw_sptr->size();
           value_ptr++ )
      {
        stream() << *value_ptr << d->m_sub_delim;
      }
      stream() << d->m_delim;
    }
    else
    {
      stream() << 0 << d->m_delim << " " << d->m_delim;
    }

    // Process classifications if there are any
    stream() << desc->get_history().size() << d->m_delim;
    for( auto h : desc->get_history() )
    {
      stream() << h.get_timestamp().get_frame() << d->m_sub_delim;
      stream() << h.get_timestamp().get_time_usec() << d->m_sub_delim;

      stream() << h.get_image_location().min_x() << d->m_sub_delim;
      stream() << h.get_image_location().min_y() << d->m_sub_delim;
      stream() << h.get_image_location().max_x() << d->m_sub_delim;
      stream() << h.get_image_location().max_y() << d->m_sub_delim;

      if( d->c_write_world_loc() )
      {
        stream() << h.get_world_location().min_x() << d->m_sub_delim;
        stream() << h.get_world_location().min_y() << d->m_sub_delim;
        stream() << h.get_world_location().max_x() << d->m_sub_delim;
        stream() << h.get_world_location().max_y() << d->m_sub_delim;
      }
    }
    stream() << std::endl;
  }
}

} // namespace core

} // namespace arrows

}     // end namespace
