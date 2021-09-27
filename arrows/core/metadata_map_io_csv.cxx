// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of metadata writing to csv

#include "metadata_map_io_csv.h"

#include <vital/any.h>
#include <vital/exceptions/algorithm.h>
#include <vital/exceptions/io.h>
#include <vital/types/geo_point.h>
#include <vital/types/geo_polygon.h>
#include <vital/types/geodesy.h>
#include <vital/util/string.h>
#include <vital/util/tokenize.h>

#include <vital/range/iota.h>

#include <iterator>
#include <string>
#include <typeinfo>
#include <vector>

namespace kv = kwiver::vital;
namespace kvr = kv::range;

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class metadata_map_io_csv::priv
{
public:
  // Quote csv field if needed
  void write_csv_item( std::string const& csv_field,
                       std::ostream& fout );
  // Correctly write a metadata item as one or more fields
  void write_csv_item( kv::metadata_item const& metadata,
                       std::ostream& fout );
  // Quote csv header item as needed, and explode types as needed
  void write_csv_header( kv::vital_metadata_tag const& csv_field,
                         std::ostream& fout,
                         std::string const& field_name,
                         std::string const& field_override );

  kv::metadata_traits md_traits;
  bool write_remaining_columns{ true };
  bool write_enum_names{ false };
  std::string names_string;
  std::vector< std::string > column_names;
  std::string overrides_string;
  std::vector< std::string > column_overrides;
  uint64_t every_n_microseconds;
  uint64_t every_n_frames;
};

// ----------------------------------------------------------------------------
void
metadata_map_io_csv::priv
::write_csv_item( std::string const& csv_field,
                  std::ostream& fout )
{
  // TODO handle other pathalogical characters such as quotes or newlines
  if( csv_field.find( ',' ) != std::string::npos )
  {
    fout << '"' << csv_field << "\",";
  }
  else
  {
    fout << csv_field << ",";
  }
}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv::priv
::write_csv_item( kv::metadata_item const& metadata,
                  std::ostream& fout )
{
  constexpr auto crs = kwiver::vital::SRID::lat_lon_WGS84;
  if( metadata.type() == typeid( kv::geo_point ) )
  {
    auto const& data = metadata.data();
    kv::geo_point point = kv::any_cast< kv::geo_point >( data );
    kv::geo_point::geo_3d_point_t loc = point.location( crs );

    fout << loc( 0 ) << "," << loc( 1 ) << "," << loc( 2 ) << ",";
  }
  else if( metadata.type() == typeid( kv::geo_polygon ) )
  {
    auto const& data = metadata.data();
    kv::geo_polygon poly = kv::any_cast< kv::geo_polygon >( data );
    kv::polygon verts = poly.polygon( crs );

    for( size_t n = 0; n < verts.num_vertices(); ++n )
    {
      auto const& v = verts.at( n );
      fout << v[ 0 ] << "," << v[ 1 ] << ",";
    }
  }
  else if( metadata.type() == typeid( bool ) )
  {
    auto const& data = metadata.data();
    auto const truth = kv::any_cast< bool >( data );
    fout << ( truth ? "true," : "false," );
  }
  else if( metadata.type() == typeid( std::string ) )
  {
    auto const& data = metadata.data();
    auto const string = kv::any_cast< std::string >( data );
    fout << '"' << string << "\",";
  }
  else
  {
    write_csv_item( kv::metadata::format_string( metadata.as_string() ),
                    fout );
  }
}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv::priv
::write_csv_header( kv::vital_metadata_tag const& csv_field,
                    std::ostream& fout,
                    std::string const& field_name,
                    std::string const& field_override )
{
  if( !field_override.empty() )
  {
    fout << "\"" << field_override << "\",";
  }
  else if( csv_field == kv::VITAL_META_UNKNOWN )
  {
    fout << "\"" << field_name << "\",";
  }
  else if( csv_field == kv::VITAL_META_SENSOR_LOCATION )
  {
    fout << "\"Sensor Geodetic lon (EPSG:4326)\","
            "\"Sensor Geodetic lat (EPSG:4326)\","
            "\"Sensor Geodetic altitude (meters)\",";
  }
  else if( csv_field == kv::VITAL_META_FRAME_CENTER )
  {
    fout << "\"Geodetic Frame Center lon (EPSG:4326)\","
            "\"Geodetic Frame Center lat (EPSG:4326)\","
            "\"Geodetic Frame Center elevation (meters)\",";
  }
  else if( csv_field == kv::VITAL_META_TARGET_LOCATION )
  {
    fout << "\"Target Geodetic Location lon (EPSG:4326)\","
            "\"Target Geodetic Location lat (EPSG:4326)\","
            "\"Target Geodetic Location elevation (meters)\",";
  }
  else if( csv_field == kv::VITAL_META_CORNER_POINTS )
  {
    fout << "\"Upper left corner point lon (EPSG:4326)\","
            "\"Upper left corner point lat (EPSG:4326)\","
            "\"Upper right corner point lon (EPSG:4326)\","
            "\"Upper right corner point lat (EPSG:4326)\","
            "\"Lower right corner point lon (EPSG:4326)\","
            "\"Lower right corner point lat (EPSG:4326)\","
            "\"Lower left corner point lon (EPSG:4326)\","
            "\"Lower left corner point lat (EPSG:4326)\",";
  }
  else
  {
    // Quote all other data either as the enum name or description
    if( write_enum_names )
    {
      fout << '"' << md_traits.tag_to_enum_name( csv_field ) << "\",";
    }
    else
    {
      fout << '"' << md_traits.tag_to_name( csv_field ) << "\",";
    }
  }
}

// ----------------------------------------------------------------------------
metadata_map_io_csv
::metadata_map_io_csv()
  : d_{ new priv }
{
  attach_logger( "arrows.core.metadata_map_io" );
}

// ----------------------------------------------------------------------------
metadata_map_io_csv
::~metadata_map_io_csv()
{
}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv
::set_configuration( vital::config_block_sptr config )
{
  d_->write_remaining_columns = config->get_value< bool >(
    "write_remaining_columns" );
  d_->write_enum_names = config->get_value< bool >( "write_enum_names" );
  d_->every_n_microseconds =
    config->has_value( "every_n_microseconds" )
    ? config->get_value< uint64_t >( "every_n_microseconds" ) : 0;
  d_->every_n_frames =
    config->has_value( "every_n_frames" )
    ? config->get_value< uint64_t >( "every_n_frames" ) : 0;

  auto const split_and_trim =
    []( std::string const& s ) -> std::vector< std::string > {
      std::vector< std::string > result;
      kwiver::vital::tokenize( s, result, "," );
      std::for_each( result.begin(), result.end(),
                     kwiver::vital::string_trim );
      return result;
    };

  d_->names_string = config->get_value< std::string >( "column_names" );
  d_->column_names = split_and_trim( d_->names_string );
  d_->overrides_string = config->get_value< std::string >( "column_overrides" );
  d_->column_overrides = split_and_trim( d_->overrides_string );
  d_->column_overrides.resize( d_->column_names.size() );
}

// ----------------------------------------------------------------------------
bool
metadata_map_io_csv
::check_configuration( vital::config_block_sptr config ) const
{
  return !( config->has_value( "every_n_microseconds" ) &&
            config->has_value( "every_n_frames" ) );
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
metadata_map_io_csv
::get_configuration() const
{
  // get base config from base class
  auto config = algorithm::get_configuration();

  config->set_value( "column_names", d_->names_string,
                     "Comma-separated values specifying column order. Can "
                     "either be the enum names, e.g. VIDEO_KEY_FRAME or the "
                     "description, e.g. 'Is frame a key frame'" );
  config->set_value( "column_overrides", d_->overrides_string,
                     "Comma-separated values overriding the final column names"
                     "as they appear in the output file. Order matches up with"
                     "column_names." );
  config->set_value( "write_enum_names", d_->write_enum_names,
                     "Write enum names rather than descriptive names" );
  config->set_value( "write_remaining_columns", d_->write_remaining_columns,
                     "Write columns present in the metadata but not in the "
                     "manually-specified list." );
  config->set_value( "every_n_microseconds", d_->every_n_microseconds,
                     "Minimum time between successive rows of output. Packets "
                     "more frequent than this will be ignored. If nonzero, "
                     "packets without a timestamp are also ignored." );
  config->set_value( "every_n_frames", d_->every_n_frames,
                     "Number of frames to skip between successive rows of "
                     "output, plus one. A value of 1 will print one packet for "
                     "every frame, while a value of 0 will print all packets "
                     "for every frame." );
  return config;
}

// ----------------------------------------------------------------------------
kv::metadata_map_sptr
metadata_map_io_csv
::load_( VITAL_UNUSED std::istream& fin, std::string const& filename ) const
{
  throw kv::file_write_exception( filename, "not implemented" );
}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv
::save_( std::ostream& fout,
         kv::metadata_map_sptr data,
         std::string const& filename ) const
{
  if( !fout )
  {
    VITAL_THROW( kv::file_write_exception, filename,
                 "Insufficient permissions or moved file" );
  }

  // Accumulate the unique metadata IDs
  std::set< kv::vital_metadata_tag > present_metadata_ids;

  for( auto const& frame_data : data->metadata() )
  {
    for( auto const& metadata_packet : frame_data.second )
    {
      for( auto const& metadata_item : *metadata_packet )
      {
        auto const type_id = metadata_item.first;
        if( type_id != kv::VITAL_META_VIDEO_URI )
        {
          present_metadata_ids.insert( type_id );
        }
      }
    }
  }

  struct metadata_info
  {
    kv::vital_metadata_tag id;
    std::string name;
    std::string str;
  };

  std::vector< metadata_info > infos;
  for( auto const i : kvr::iota( d_->column_names.size() ) )
  {
    auto const& name = d_->column_names[ i ];
    auto const& str = d_->column_overrides[ i ];
    kv::vital_metadata_tag trait_id;
    if( ( trait_id = d_->md_traits.enum_name_to_tag( name ) ) !=
        kv::VITAL_META_UNKNOWN ||
        ( trait_id = d_->md_traits.name_to_tag( name ) ) !=
        kv::VITAL_META_UNKNOWN )
    {
      // Avoid duplicating present columns
      present_metadata_ids.erase( trait_id );
    }

    infos.push_back( { trait_id, name, str } );
  }

  // Determine whether to write columns present in the metadata but not
  // explicitly provided
  if( d_->write_remaining_columns )
  {
    for( auto const& id : present_metadata_ids )
    {
      infos.push_back( { id, "", "" } );
    }
  }

  // Write out the csv header
  fout << "\"frame ID\",";
  for( auto const& info : infos )
  {
    d_->write_csv_header( info.id, fout, info.name, info.str );
  }

  fout << std::endl;

  if( d_->every_n_microseconds && d_->every_n_frames )
  {
    throw kv::algorithm_configuration_exception(
      this->type_name(), this->impl_name(),
      "options 'every_n_microseconds' and 'every_n_frames' are incompatible" );
  }

  uint64_t next_timestamp = d_->every_n_microseconds;
  uint64_t next_frame = 1;
  for( auto const& frame_data : data->metadata() )
  {
    for( auto const& metadata_packet : frame_data.second )
    {
      // Write only at the specified frequency
      auto const timestamp = metadata_packet->timestamp();
      if( d_->every_n_microseconds )
      {
        if( !timestamp.has_valid_time() ||
            timestamp.get_time_usec() < next_timestamp )
        {
          continue;
        }
        next_timestamp +=
          ( ( timestamp.get_time_usec() - next_timestamp ) /
            d_->every_n_microseconds + 1 ) * d_->every_n_microseconds;
      }
      if( d_->every_n_frames )
      {
        if( !timestamp.has_valid_frame() ||
            timestamp.get_frame() < next_frame )
        {
          continue;
        }
        next_frame +=
          ( ( timestamp.get_frame() - next_frame ) /
            d_->every_n_frames + 1 ) * d_->every_n_frames;
      }

      // Write the frame number
      fout << frame_data.first << ",";
      for( auto const& info : infos )
      {
        if( metadata_packet->has( info.id ) )
        {
          d_->write_csv_item( metadata_packet->find( info.id ), fout );
        }
        // Write an empty field
        else
        {
          fout << ",";
        }
      }
      fout << "\n";
    }
  }
  fout << std::flush;
}

} // namespace core

} // namespace arrows

} // namespace kwiver
