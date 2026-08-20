// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of detected object set csv output

#include "detected_object_set_output_csv.h"

#include <ctime>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class detected_object_set_output_csv::priv
{
public:
  priv( detected_object_set_output_csv& parent )
    : parent( parent ),
      m_first( true ),
      m_frame_number( 1 )
  {}

  detected_object_set_output_csv& parent;

  // Configuration values
  std::string c_delim() { return parent.c_delim; }

  ~priv() {}

  // Local values
  bool m_first;
  int m_frame_number;
};

// ----------------------------------------------------------------------------
void
detected_object_set_output_csv
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.core.detected_object_set_output_csv" );
}

detected_object_set_output_csv::
~detected_object_set_output_csv()
{}

// ----------------------------------------------------------------------------
bool
detected_object_set_output_csv
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
detected_object_set_output_csv
::write_set(
  const kwiver::vital::detected_object_set_sptr set,
  std::string const& image_name )
{
  if( d->m_first )
  {
    std::time_t rawtime;
    struct tm* timeinfo;

    time( &rawtime );
    timeinfo = localtime( &rawtime );

    char* cp = asctime( timeinfo );
    cp[ strlen( cp ) - 1 ] = 0; // remove trailing newline

    const std::string atime( cp );

    // Write file header(s)
    stream() << "# 1: image-index" << d->c_delim()
             << "2:file-name" << d->c_delim()
             << "3:TL-x" << d->c_delim()
             << "4:TL-y" << d->c_delim()
             << "5:BR-x" << d->c_delim()
             << "6:BR-y" << d->c_delim()
             << "7:confidence" << d->c_delim()
             << "{class-name" << d->c_delim() << "score}" << d->c_delim() <<
      "..."
             << std::endl

      // Provide some provenience to the file. Could have a config
      // parameter that is copied to the file as a configurable
      // comment or marker.
             << "# Written on: " << atime
             << "   by: detected_object_set_output_csv"
             << std::endl;

    d->m_first = false;
  } // end first

  // process all detections
  auto ie = set->cend();
  for( auto det = set->cbegin(); det != ie; ++det )
  {
    const kwiver::vital::bounding_box_d bbox( ( *det )->bounding_box() );
    stream() << d->m_frame_number << d->c_delim()
             << image_name << d->c_delim()
             << bbox.min_x() << d->c_delim() // 2: TL-x
             << bbox.min_y() << d->c_delim() // 3: TL-y
             << bbox.max_x() << d->c_delim() // 4: BR-x
             << bbox.max_y() << d->c_delim() // 5: BR-y
             << ( *det )->confidence()        // 6: confidence value
    ;

    // Process classifications if there are any
    const auto cm( ( *det )->type() );
    if( cm )
    {
      const auto name_list( cm->class_names() );
      for( auto name : name_list )
      {
        // Write out the <name> <score> pair
        stream() << d->c_delim() << name << d->c_delim() << cm->score( name );
      } // end foreach
    }

    stream() << std::endl;
  } // end foreach

  // Put each set on a new frame
  ++d->m_frame_number;
}

} // namespace core

} // namespace arrows

}     // end namespace
