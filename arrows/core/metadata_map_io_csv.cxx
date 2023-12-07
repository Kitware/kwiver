// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of metadata writing to csv

#include "metadata_map_io_csv.h"

#include <arrows/core/csv_io.h>

#include <vital/any.h>
#include <vital/exceptions/algorithm.h>
#include <vital/exceptions/io.h>
#include <vital/types/geo_point.h>
#include <vital/types/geo_polygon.h>
#include <vital/types/geodesy.h>
#include <vital/util/string.h>
#include <vital/util/tokenize.h>

#include <vital/range/iota.h>

#include <iomanip>
#include <iterator>
#include <regex>
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
  std::optional< kv::metadata_value > read_csv_item(
    core::csv_reader& csv_is,
    kv::vital_metadata_tag tag );

  void write_csv_item(
    core::csv_writer& csv_os,
    kv::vital_metadata_tag tag,
    kv::metadata_value const& value );

  bool write_remaining_columns{ true };
  bool write_enum_names{ false };
  std::string names_string;
  std::vector< std::string > column_names;
  std::string overrides_string;
  std::vector< std::string > column_overrides;
  uint64_t every_n_microseconds{ 0 };
  uint64_t every_n_frames{ 0 };
};

namespace {

constexpr auto crs = kv::SRID::lat_lon_WGS84;

// ----------------------------------------------------------------------------
struct read_visitor {
  template< class T >
  std::optional< kv::metadata_value >
  operator()() const
  {
    if constexpr(
      std::is_same_v< T, kv::geo_point > ||
      std::is_same_v< T, kv::geo_polygon > )
    {
      throw std::logic_error( "Complex type given to csv field reader" );
    }
    else
    {
      if( auto const value = csv_is.read< std::optional< T > >() )
      {
        return *value;
      }
      return std::nullopt;
    }
  }

  core::csv_reader& csv_is;
};

// ----------------------------------------------------------------------------
struct write_visitor {
  template< class T >
  void
  operator()( T const& data ) const
  {
    if constexpr(
      std::is_same_v< T, kv::geo_point > ||
      std::is_same_v< T, kv::geo_polygon > )
    {
      throw std::logic_error( "Complex type given to csv field writer" );
    }
    else
    {
      csv_os << data;
    }
  }

  core::csv_writer& csv_os;
};

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
struct column_id
{
  kv::vital_metadata_tag tag;
  size_t index;

  bool
  operator<( column_id const& other ) const
  {
    return std::tie( tag, index ) < std::tie( other.tag, other.index );
  }

  bool
  operator==( column_id const& other ) const
  {
    return std::tie( tag, index ) == std::tie( other.tag, other.index );
  }
};

// ----------------------------------------------------------------------------
struct special_column_name
{
  column_id id;
  std::string name;
};

// ----------------------------------------------------------------------------
std::vector< special_column_name > const&
special_column_names()
{
  static std::vector< special_column_name > const names = {
    { { kv::VITAL_META_SENSOR_LOCATION, 0 },
      "Sensor Geodetic Longitude (EPSG:4326)" },
    { { kv::VITAL_META_SENSOR_LOCATION, 1 },
      "Sensor Geodetic Latitude (EPSG:4326)" },
    { { kv::VITAL_META_SENSOR_LOCATION, 2 },
      "Sensor Geodetic Altitude (meters)" },

    { { kv::VITAL_META_TARGET_LOCATION, 0 },
      "Target Geodetic Location Longitude (EPSG:4326)" },
    { { kv::VITAL_META_TARGET_LOCATION, 1 },
      "Target Geodetic Location Latitude (EPSG:4326)" },
    { { kv::VITAL_META_TARGET_LOCATION, 2 },
      "Target Geodetic Location Altitude (meters)" },

    { { kv::VITAL_META_FRAME_CENTER, 0 },
      "Geodetic Frame Center Longitude (EPSG:4326)" },
    { { kv::VITAL_META_FRAME_CENTER, 1 },
      "Geodetic Frame Center Longitude (EPSG:4326)" },
    { { kv::VITAL_META_FRAME_CENTER, 2 },
      "Geodetic Frame Center Altitude (meters)" },

    { { kv::VITAL_META_CORNER_POINTS, 0 },
      "Upper Left Corner Longitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 1 },
      "Upper Left Corner Latitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 2 },
      "Upper Right Corner Longitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 3 },
      "Upper Right Corner Latitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 4 },
      "Lower Right Corner Longitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 5 },
      "Lower Right Corner Latitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 6 },
      "Lower Left Corner Longitude (EPSG:4326)" },
    { { kv::VITAL_META_CORNER_POINTS, 7 },
      "Lower Left Corner Latitude (EPSG:4326)" },
  };

  return names;
}

// ----------------------------------------------------------------------------
// Get the special name for a particular subvalue, if it exists.
std::string const*
get_special_column_name( column_id const& id )
{
  for( auto const& entry : special_column_names() )
  {
    if( entry.id == id )
    {
      return &entry.name;
    }
  }
  return nullptr;
}

// ----------------------------------------------------------------------------
// Get the subvalue for a particular special column name, if it exists.
column_id const*
get_special_column_id( std::string const& name )
{
  for( auto const& entry : special_column_names() )
  {
    if( entry.name == name )
    {
      return &entry.id;
    }
  }
  return nullptr;
}

// ----------------------------------------------------------------------------
// Get the name to be used as the header title for the given subvalue.
std::string
get_column_name( column_id const& id, bool use_enum_name )
{
  auto const& traits = kv::tag_traits_by_tag( id.tag );
  auto const column_count = get_column_count( traits.type() );
  std::stringstream ss;
  if( use_enum_name )
  {
    ss << traits.enum_name();
    if( column_count > 1 )
    {
      ss << '.' << id.index;
    }
  }
  else
  {
    auto const special_name = get_special_column_name( id );
    if( special_name )
    {
      ss << *special_name;
    }
    else
    {
      ss << traits.name();
      if( column_count > 1 )
      {
        ss << '.' << id.index;
      }
    }
  }

  return ss.str();
}

// ----------------------------------------------------------------------------
// Determine what subvalue is being requested via the given string.
column_id
parse_column_id( std::string const& s )
{
  if( auto const special_id = get_special_column_id( s ) )
  {
    return *special_id;
  }

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
  return value.location( crs )( index );
}

// ----------------------------------------------------------------------------
template<>
kv::metadata_value
subvalue_visitor
::operator()< kv::geo_polygon >( kv::geo_polygon const& value ) const
{
  return value.polygon( crs ).at( index / 2 )( index % 2 );
}

// ----------------------------------------------------------------------------
// Retreive the indexed subvalue from the given value.
kv::metadata_value
get_subvalue( kv::metadata_value const& value, size_t index )
{
  return std::visit( subvalue_visitor{ index }, value );
}

// ----------------------------------------------------------------------------
struct set_subvalue_visitor
{
  template< class T >
  void
  operator()() const
  {
    constexpr auto nan = std::numeric_limits< double >::quiet_NaN();
    if constexpr( std::is_same_v< T, kv::geo_point > )
    {
      static T const default_value{ kv::vector_3d{ nan, nan, nan }, crs };
      auto original_value =
        metadata.has( column.tag )
        ? metadata.find( column.tag ).get< T >()
        : default_value;
      auto internal_value = original_value.location( crs );
      internal_value( column.index ) = std::get< double >( value );
      original_value.set_location( internal_value, crs );
      metadata.add( column.tag, original_value );
    }
    else if constexpr( std::is_same_v< T, kv::geo_polygon > )
    {
      static T const default_value{
        std::vector( 4, kv::vector_2d{ nan, nan } ), crs };
      auto original_value =
        metadata.has( column.tag )
        ? metadata.find( column.tag ).get< T >()
        : default_value;
      auto internal_value = original_value.polygon( crs ).get_vertices();
      internal_value.at( column.index / 2 )( column.index % 2 ) =
        std::get< double >( value );
      original_value.set_polygon( internal_value, crs );
      metadata.add( column.tag, original_value );
    }
    else
    {
      metadata.add( column.tag, value );
    }
  }

  column_id const& column;
  kv::metadata_value const& value;
  kv::metadata& metadata;
};

} // namespace <anonymous>

// ----------------------------------------------------------------------------
std::optional< kv::metadata_value >
metadata_map_io_csv::priv
::read_csv_item( core::csv_reader& csv_is, kv::vital_metadata_tag tag )
{
  if( tag == kv::VITAL_META_VIDEO_MICROSECONDS )
  {
    auto const maybe_s = csv_is.read< std::optional< std::string > >();
    if( !maybe_s )
    {
      return std::nullopt;
    }
    auto const& s = *maybe_s;

    static std::regex const pattern( "(\\d{2}):(\\d{2}):(\\d{2}).(\\d{6})" );
    std::smatch match;
    if( !std::regex_match( s, match, pattern ) )
    {
      return std::nullopt;
    }
    uint64_t microseconds = 0;
    auto const convert =
      [ &match, &microseconds ]( size_t index, uint64_t factor ) {
        microseconds *= factor;
        microseconds += std::stoull( match.str( index ) );
      };
    convert( 1, 0 );
    convert( 2, 60 );
    convert( 3, 60 );
    convert( 4, 1000000 );
    return microseconds;
  }
  else
  {
    auto const* type = &kv::tag_traits_by_tag( tag ).type();
    if( *type == typeid( kv::geo_point ) || *type == typeid( kv::geo_polygon ) )
    {
      type = &typeid( double );
    }

    return kv::visit_metadata_types_return<
      std::optional< kv::metadata_value >, read_visitor >( { csv_is }, *type );
  }
}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv::priv
::write_csv_item(
  core::csv_writer& csv_os,
  kv::vital_metadata_tag tag,
  kv::metadata_value const& value )
{
  if( tag == kv::VITAL_META_VIDEO_MICROSECONDS )
  {
    // Print as hh:mm:ss.ssssss
    auto const microseconds = std::get< uint64_t >( value );
    auto const seconds = ( microseconds / 1000000 );
    auto const minutes = ( seconds / 60 );
    auto const hours = ( minutes / 60 );
    std::stringstream ss;
    ss << std::setfill( '0' );
    ss << std::setw( 2 ) << hours << ':';
    ss << std::setw( 2 ) << ( minutes % 60 ) << ':';
    ss << std::setw( 2 ) << ( seconds % 60 ) << '.';
    ss << std::setw( 6 ) << ( microseconds % 1000000 );
    csv_os << ss.str();
  }
  else
  {
    std::visit( write_visitor{ csv_os }, value );
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
::load_( std::istream& is, std::string const& filename ) const
{
  // Check that output file is valid
  if( !is )
  {
    VITAL_THROW(
      kv::invalid_file, filename, "Insufficient permissions or moved file" );
  }

  // Initialize reader
  core::csv_reader csv_is{ is };

  // Parse column names
  std::vector< column_id > column_ids{
    { kv::VITAL_META_VIDEO_FRAME_NUMBER, 0 } };
  if( csv_is.read< std::string >() != "Frame ID" )
  {
    VITAL_THROW(
      kv::invalid_file, filename, "First column must be 'Frame ID'" );
  }
  while( !csv_is.is_at_eol() )
  {
    auto const name = csv_is.read< std::string >();
    column_ids.emplace_back( parse_column_id( name ) );
  }

  // Parse remaining lines
  vital::simple_metadata_map::map_metadata_t result;
  while( !csv_is.is_at_eof() )
  {
    csv_is.next_line();

    // Parse each column in turn
    std::map< column_id, kv::metadata_value > values;
    for( auto const& column : column_ids )
    {
      if( auto const value = d_->read_csv_item( csv_is, column.tag ) )
      {
        if( !values.emplace( column, *value ).second )
        {
          LOG_WARN( logger(),
            "Dropping duplicate value for column: "
            << get_column_name( column, true ) );
        }
      }
    }

    // Create an empty metadata packet for this frame
    using frame_number_t =
      vital::type_of_tag< vital::VITAL_META_VIDEO_FRAME_NUMBER >;
    auto const& frame_number_value =
      values.at( { vital::VITAL_META_VIDEO_FRAME_NUMBER, 0 } );
    auto const frame_number = std::get< frame_number_t >( frame_number_value );
    auto& metadata =
      *result.emplace( frame_number, kv::metadata_vector{} ).first->second
      .emplace_back( std::make_shared< kv::metadata >() );

    // Fill that metadata packet with the values, correctly handling
    // multi-column fields
    for( auto const& entry : values )
    {
      auto const& tag_type =
        vital::tag_traits_by_tag( entry.first.tag ).type();
      kv::visit_metadata_types< set_subvalue_visitor >(
        { entry.first, entry.second, metadata }, tag_type );
    }
  }

  return std::make_shared< kv::simple_metadata_map >( result );
}

// ----------------------------------------------------------------------------
void
metadata_map_io_csv
::save_( std::ostream& os,
         kv::metadata_map_sptr data,
         std::string const& filename ) const
{
  if( !os )
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
        ? get_column_name( info.id, d_->write_enum_names )
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
      auto const name = get_column_name( id, d_->write_enum_names );
      infos.push_back( { id, name } );
    }
  }

  // Write out the csv header
  core::csv_writer csv_os{ os };
  csv_os << "Frame ID";
  for( auto const& info : infos )
  {
    csv_os << info.name;
  }
  csv_os << core::csv::endl;

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
      csv_os << frame_data.first;
      for( auto const& info : infos )
      {
        if( metadata_packet->has( info.id.tag ) )
        // Write field data
        {
          auto const& item = metadata_packet->find( info.id.tag );
          auto const subvalue = get_subvalue( item.data(), info.id.index );
          d_->write_csv_item( csv_os, item.tag(), subvalue );
        }
        else
        // Write empty fields
        {
          csv_os << core::csv::skipf;
        }
      }
      csv_os << core::csv::endl;
    }
  }
  os << std::flush;
}

} // namespace core

} // namespace arrows

} // namespace kwiver
