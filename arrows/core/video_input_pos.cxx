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

#include "video_input_pos.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/exceptions.h>
#include <vital/util/data_stream_reader.h>

#include <vital/video_metadata/video_metadata.h>
#include <vital/video_metadata/pos_metadata_io.h>

#include <kwiversys/SystemTools.hxx>

#include <fstream>


namespace kwiver {
namespace arrows {
namespace core {

class video_input_pos::priv
{
public:
  priv()
    : c_meta_extension( ".pos" )
    , d_at_eov( false )
  { }

  // Configuration values
  std::string c_meta_directory;
  std::string c_meta_extension;
  std::string c_image_list_file;

  // local state
  bool d_at_eov;

  std::vector < kwiver::vital::path_t > d_metadata_files;
  std::vector < kwiver::vital::path_t >::const_iterator d_current_file;
  std::vector < kwiver::vital::path_t >::const_iterator d_end;
  kwiver::vital::timestamp::frame_t d_frame_number;

  vital::video_metadata_sptr d_metadata;
};


// ------------------------------------------------------------------
video_input_pos
::video_input_pos()
  : d( new video_input_pos::priv )
{
  attach_logger( "video_input_pos" );

  set_capability( vital::algo::video_input::HAS_EOV, true );
  set_capability( vital::algo::video_input::HAS_FRAME_NUMBERS, true );
  set_capability( vital::algo::video_input::HAS_FRAME_TIME, true );
  set_capability( vital::algo::video_input::HAS_METADATA, true );

  set_capability( vital::algo::video_input::HAS_FRAME_DATA, false );
  set_capability( vital::algo::video_input::HAS_ABSOLUTE_FRAME_TIME, false ); // MAYBE
  set_capability( vital::algo::video_input::HAS_TIMEOUT, false );
}


// ------------------------------------------------------------------
video_input_pos
::~video_input_pos()
{
}


// ------------------------------------------------------------------
vital::config_block_sptr
video_input_pos
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = vital::algo::video_input::get_configuration();

  config->set_value( "metadata_directory", d->c_meta_directory,
                     "Name of directory containing metadata files." );

  config->set_value( "metadata_extension", d->c_meta_extension,
                     "File extension of metadata files." );

  return config;
}


// ------------------------------------------------------------------
void
video_input_pos
::set_configuration( vital::config_block_sptr in_config )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  d->c_meta_directory = config->get_value<std::string>(
    "metadata_directory", d->c_meta_directory );

  d->c_meta_extension = config->get_value<std::string>(
    "metadata_extension", d->c_meta_extension );
}


// ------------------------------------------------------------------
bool
video_input_pos
::check_configuration( vital::config_block_sptr config ) const
{
  return true;
}


// ------------------------------------------------------------------
void
video_input_pos
::open( std::string image_list_name )
{
  typedef kwiversys::SystemTools ST;

  // open file and read lines
  std::ifstream ifs( image_list_name.c_str() );
  if ( ! ifs )
  {
    throw kwiver::vital::invalid_file( image_list_name, "Could not open file" );
  }

  kwiver::vital::data_stream_reader stream_reader( ifs );

  // verify and get file names in a list
  std::string line;
  while ( stream_reader.getline( line ) )
  {
    // Get base name from file
    std::string resolved_file = d->c_meta_directory;
    resolved_file += "/" + ST::GetFilenameWithoutLastExtension( line )
                     + d->c_meta_extension;
    if ( ! ST::FileExists( resolved_file ) )
    {
      LOG_DEBUG( logger(), "Could not find file " << resolved_file
                 <<". This frame will not have any metadata." );
      resolved_file.clear(); // indicate that the metadata file could not be found
    }

    d->d_metadata_files.push_back( resolved_file );
  } // end while

  d->d_current_file = d->d_metadata_files.begin();
  d->d_frame_number = 1;

  d->d_end = d->d_metadata_files.end(); // set default end marker
}


// ------------------------------------------------------------------
void
video_input_pos
::close()
{
}


// ------------------------------------------------------------------
bool
video_input_pos
::end_of_video() const
{
  return d->d_at_eov;
}


// ------------------------------------------------------------------
bool
video_input_pos
::good() const
{
  // This could use a more nuanced approach
  return true;
}


// ------------------------------------------------------------------
bool
video_input_pos
::next_frame( kwiver::vital::timestamp& ts,   // returns timestamp
              uint32_t                  timeout ) // not supported
{
  // Check for at end of data
  if ( d->d_at_eov )
  {
    return false;
  }

  if ( d->d_current_file == d->d_end )
  {
    d->d_at_eov = true;
    return false;
  }

  // reset current metadata packet.
  d->d_metadata = nullptr;

  if ( ! d->d_current_file->empty() )
  {
    // Open next file in the list
    d->d_metadata = vital::read_pos_file( *d->d_current_file );
  }

  // Return timestamp
  ts = kwiver::vital::timestamp();
  ts.set_frame( d->d_frame_number );
  if ( d->d_metadata && d->d_metadata->has( vital::VITAL_META_GPS_SEC ) )
  {
    double gps_sec = d->d_metadata->find( vital::VITAL_META_GPS_SEC ).as_double();
    // TODO: also use gps_week and convert to UTC to get abosolute time
    // or subtract off first frame time to get time relative to start
    ts.set_time_seconds( gps_sec );
  }
  d->d_metadata->set_timestamp( ts );

  // update timestamp
  ++d->d_frame_number;
  ++d->d_current_file;

  return true;
}


// ------------------------------------------------------------------
kwiver::vital::image_container_sptr
video_input_pos
::frame_image()
{
  // Could return a blank image, but we do not know a good size;
  return 0;
}


// ------------------------------------------------------------------
kwiver::vital::video_metadata_vector
video_input_pos
::frame_metadata()
{
  kwiver::vital::video_metadata_vector vect;
  if ( d->d_metadata )
  {
    vect.push_back( d->d_metadata );
  }

  return vect;
}

} } }     // end namespace
