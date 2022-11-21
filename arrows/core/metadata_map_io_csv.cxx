// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of metadata writing to csv

#include "metadata_map_io_csv.h"

#include <vital/exceptions/algorithm.h>
#include <vital/exceptions/io.h>
#include <vital/types/geo_point.h>
#include <vital/types/geo_polygon.h>
#include <vital/types/geodesy.h>
#include <vital/util/string.h>
#include <vital/util/tokenize.h>

#include <vital/range/iota.h>

#include <any>
#include <iomanip>
#include <iterator>
#include <stdexcept>
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
  void write_csv_item( kv::vital_metadata_tag tag,
                       kv::metadata_value const& value,
                       std::ostream& fout );

  bool write_remaining_columns{ true };
  bool write_enum_names{ false };
  std::string names_string;
  std::vector< std::string > column_names;
  std::string overrides_string;
  std::vector< std::string > column_overrides;
  uint64_t every_n_microseconds;
  uint64_t every_n_frames;
};

namespace {

// ----------------------------------------------------------------------------
struct write_visitor {
  template< class T >
  void
  operator()( T const& data ) const
  {
    if( std::is_arithmetic< T >::value )
    {
      os << data << ',';
    }
    else
    {
      // TODO: handle pathalogical characters such as quotes or newlines
      os << "\"" << data << "\",";
    }
  }

  std::ostream& os;
};

// ----------------------------------------------------------------------------
template<>
void
write_visitor::operator()< bool >( bool const& data ) const
{
  os << ( data ? "true," : "false," );
}

// ----------------------------------------------------------------------------
template<>
void
write_visitor::operator()< std::string >( std::string const& data ) const
{
  // TODO: handle other pathalogical characters such as quotes or newlines
  os << "\"" << data << "\",";
}

// ----------------------------------------------------------------------------
template<>
void
write_visitor::operator()< kv::geo_point >( kv::geo_point const& ) const
{
  throw std::logic_error( "geo_point should have been split" );
}

// ----------------------------------------------------------------------------
template<>
void
write_visitor::operator()< kv::geo_polygon >( kv::geo_polygon const& ) const
{
  throw std::logic_error( "geo_polygon should have been split" );
}

// ----------------------------------------------------------------------------
// Get the number of simple values (e.g. numbers) required to express the given
// type
size_t
get_column_count( std::type_info const& type )
{
  static std::map< std::type_index, size_t > const map = {
    { typeid( kv::geo_point ), 3 },   // (lon, lat, alt)
    { typeid( kv::geo_polygon ), 8 }, // 4x(lon, lat)
  };

  auto const it = map.find( type );
  return ( it != map.end() ) ? it->second : 1;
}

// ----------------------------------------------------------------------------
// Get the special name for a particular subvalue, if it exists
std::string const*
get_special_column_name( kv::vital_metadata_tag tag, size_t index )
{
  static std::map< kv::vital_metadata_tag,
                   std::vector< std::string > > const map = {
    { kv::VITAL_META_SENSOR_LOCATION,
      { "Sensor Geodetic Longitude (EPSG:4326)",
        "Sensor Geodetic Latitude (EPSG:4326)",
        "Sensor Geodetic Altitude (meters)", } },
    { kv::VITAL_META_FRAME_CENTER,
      { "Geodetic Frame Center Longitude (EPSG:4326)",
        "Geodetic Frame Center Latitude (EPSG:4326)",
        "Geodetic Frame Center Elevation (meters)", } },
    { kv::VITAL_META_TARGET_LOCATION,
      { "Target Geodetic Location Longitude (EPSG:4326)",
        "Target Geodetic Location Latitude (EPSG:4326)",
        "Target Geodetic Location Elevation (meters)", } },
    { kv::VITAL_META_CORNER_POINTS,
      { "Upper Left Corner Longitude (EPSG:4326)",
        "Upper Left Corner Latitude (EPSG:4326)",
        "Upper Right Corner Longitude (EPSG:4326)",
        "Upper Right Corner Latitude (EPSG:4326)",
        "Lower Right Corner Longitude (EPSG:4326)",
        "Lower Right Corner Latitude (EPSG:4326)",
        "Lower Left Corner Longitude (EPSG:4326)",
        "Lower Left Corner Latitude (EPSG:4326)", } },
  };

  auto const it = map.find( tag );
  return ( it != map.end() ) ? &it->second.at( index ) : nullptr;
}

// ----------------------------------------------------------------------------
// Get the name to be used as the header title for the given subvalue.
std::string
get_column_name( kv::vital_metadata_tag tag, size_t index, bool use_enum_name )
{
  auto const& traits = kv::tag_traits_by_tag( tag );
  auto const column_count = get_column_count( traits.type() );
  std::stringstream ss;
  if( use_enum_name )
  {
    ss << traits.enum_name();
    if( column_count > 1 )
    {
      ss << '.' << index;
    }
  }
  else
  {
    auto const special_name = get_special_column_name( tag, index );
    if( special_name )
    {
      ss << *special_name;
    }
    else
    {
      ss << traits.name();
      if( column_count > 1 )
      {
        ss << '.' << index;
      }
    }
  }

  return ss.str();
}

// ----------------------------------------------------------------------------
struct subvalue_visitor
{
  template< class T >
  kv::metadata_value
  operator()( T const& value ) const
  {
    return value;
  }

  size_t index;
};

// ----------------------------------------------------------------------------
template<>
kv::metadata_value
subvalue_visitor
::operator()< kv::geo_point >( kv::geo_point const& value ) const
{
  return value.location( kv::SRID::lat_lon_WGS84 )( index );
}

// ----------------------------------------------------------------------------
template<>
kv::metadata_value
subvalue_visitor
::operator()< kv::geo_polygon >( kv::geo_polygon const& value ) const
{
  return value.polygon( kv::SRID::lat_lon_WGS84 ).at( index / 2 )( index % 2 );
}

// ----------------------------------------------------------------------------
// Retreive the indexed subvalue from the given value.
kv::metadata_value
get_subvalue( kv::metadata_value const& value, size_t index )
{
  return kv::visit( subvalue_visitor{ index }, value );
}

// ----------------------------------------------------------------------------
struct column_id
{
  kv::vital_metadata_tag tag;
  size_t index;

  bool
  operator<( column_id const& other ) const
  {
    if( tag < other.tag ) { return true; }
    if( tag > other.tag ) { return false; }
    return index < other.index;
  }
};

// ----------------------------------------------------------------------------
// Determine what subvalue is being requested via the given string.
column_id
parse_column_id( std::string const& s )
{
  // Format of s will be: NAME.INDEX or just NAME (index defaults to 0)
  // NAME will be either enum_name or regular name of a vital tag
  column_id result = { kv::VITAL_META_UNKNOWN, 0 };
  auto const separator_pos = s.rfind( '.' );
  auto name = s;
  if( separator_pos != s.npos )
  {
    try
    {
      result.index = std::stoi( s.substr( separator_pos + 1 ) );
      name = s.substr( 0, separator_pos );
    }
    catch( std::invalid_argument const& e )
    {
      // Maybe there was a period in the name?
    }
  }
  if( ( result.tag = kv::tag_traits_by_enum_name( name ).tag() ) ==
      kv::VITAL_META_UNKNOWN )
  {
    result.tag = kv::tag_traits_by_name( name ).tag();
  }
  return result;
}

}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv::priv
::write_csv_item( kv::vital_metadata_tag tag,
                  kv::metadata_value const& value,
                  std::ostream& fout )
{
  if( tag == kv::VITAL_META_VIDEO_MICROSECONDS )
  {
    // Print as hh:mm:ss.ssssss
    auto const microseconds = kv::get< uint64_t >( value );
    auto const seconds = ( microseconds / 1000000 );
    auto const minutes = ( seconds / 60 );
    auto const hours = ( minutes / 60 );
    auto const flags = fout.flags();
    fout << std::setfill( '0' );
    fout << std::setw( 2 ) << hours << ':';
    fout << std::setw( 2 ) << ( minutes % 60 ) << ':';
    fout << std::setw( 2 ) << ( seconds % 60 ) << '.';
    fout << std::setw( 6 ) << ( microseconds % 1000000 ) << ',';
    fout.flags( flags );
  }
  else
  {
    kv::visit( write_visitor{ fout }, value );
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
  d_->write_remaining_columns =
    config->get_value< bool >( "write_remaining_columns", true );
  d_->write_enum_names =
    config->get_value< bool >( "write_enum_names", false );
  d_->every_n_microseconds =
    config->get_value< uint64_t >( "every_n_microseconds", 0 );
  d_->every_n_frames =
    config->get_value< uint64_t >( "every_n_frames", 0 );

  auto const split_and_trim =
    []( std::string const& s ) -> std::vector< std::string > {
      if( s.empty() )
      {
        return {};
      }

      std::vector< std::string > result;
      kwiver::vital::tokenize( s, result, "," );
      std::for_each( result.begin(), result.end(),
                     kwiver::vital::string_trim );
      return result;
    };

  d_->names_string = config->get_value< std::string >( "column_names", "" );
  d_->column_names = split_and_trim( d_->names_string );
  d_->overrides_string =
    config->get_value< std::string >( "column_overrides", "" );
  d_->column_overrides = split_and_trim( d_->overrides_string );
  d_->column_overrides.resize( d_->column_names.size() );
}

// ----------------------------------------------------------------------------
bool
metadata_map_io_csv
::check_configuration( vital::config_block_sptr config ) const
{
  return !( config->get_value< uint64_t >( "every_n_microseconds", 0 ) &&
            config->get_value< uint64_t >( "every_n_frames", 0 ) );
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
                     "description, e.g. 'Is frame a key frame'. For composite "
                     "data types, index using '.', e.g. 'SENSOR_LOCATION.2' "
                     "for sensor altitude." );
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
                     "Minimum time between successive rows of output. Frames "
                     "more frequent than this will be ignored. If nonzero, "
                     "frames without a timestamp are also ignored." );
  config->set_value( "every_n_frames", d_->every_n_frames,
                     "Number of frames to skip between successive rows of "
                     "output, plus one. A value of 1 will print every frame." );
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
  std::set< column_id > remaining_ids;

  for( auto const& frame_data : data->metadata() )
  {
    for( auto const& metadata_packet : frame_data.second )
    {
      for( auto const& metadata_item : *metadata_packet )
      {
        auto const tag = metadata_item.first;
        auto const& type = metadata_item.second->type();
        if( tag != kv::VITAL_META_VIDEO_URI &&
            tag != kv::VITAL_META_VIDEO_FRAME_NUMBER )
        {
          for( auto const i : kvr::iota( get_column_count( type ) ) )
          {
            remaining_ids.insert( { tag, i } );
          }
        }
      }
    }
  }

  struct column_info
  {
    column_id id;
    std::string name;
  };

  std::vector< column_info > infos;
  for( auto const i : kvr::iota( d_->column_names.size() ) )
  {
    auto const& name = d_->column_names[ i ];
    auto const& name_override = d_->column_overrides[ i ];

    column_info info;
    info.id = parse_column_id( name );
    info.name =
      name_override.empty()
        ? get_column_name( info.id.tag, info.id.index, d_->write_enum_names )
        : name_override;

    if( info.id.tag != kv::VITAL_META_UNKNOWN )
    {
      remaining_ids.erase( info.id );
    }

    infos.push_back( info );
  }

  // Determine whether to write columns present in the metadata but not
  // explicitly provided
  if( d_->write_remaining_columns )
  {
    for( auto const& id : remaining_ids )
    {
      auto const name =
        get_column_name( id.tag, id.index, d_->write_enum_names );
      infos.push_back( { id, name } );
    }
  }

  // Write out the csv header
  fout << "\"Frame ID\",";
  for( auto const& info : infos )
  {
    fout << "\"" << info.name << "\",";
  }
  fout << std::endl;

  if( d_->every_n_microseconds && d_->every_n_frames )
  {
    throw kv::algorithm_configuration_exception(
      this->type_name(), this->impl_name(),
      "options 'every_n_microseconds' and 'every_n_frames' are incompatible" );
  }

  int64_t next_timestamp = d_->every_n_microseconds;
  int64_t next_frame = 1;
  for( auto const& frame_data : data->metadata() )
  {
    // Write only at the specified frequency
    auto const timestamp =
      frame_data.second.size()
      ? frame_data.second.at( 0 )->timestamp()
      : kv::timestamp{};
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

    for( auto const& metadata_packet : frame_data.second )
    {
      // Write the frame number
      fout << frame_data.first << ",";
      for( auto const& info : infos )
      {
        if( metadata_packet->has( info.id.tag ) )
        // Write field data
        {
          auto const& item = metadata_packet->find( info.id.tag );
          d_->write_csv_item( item.tag(),
                              get_subvalue( item.data(), info.id.index ),
                              fout );
        }
        else
        // Write empty fields
        {
          fout << ',';
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
