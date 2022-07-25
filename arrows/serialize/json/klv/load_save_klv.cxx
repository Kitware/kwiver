// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/klv/load_save_klv.h>
#include <arrows/serialize/json/load_save.h>

#include <arrows/klv/klv_all.h>

#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/cereal.hpp>
#include <vital/internal/cereal/external/base64.hpp>

#include <vital/range/iota.h>
#include <vital/util/visit.h>

#include <stdexcept>

using namespace kwiver::arrows::klv;
namespace kv = kwiver::vital;

using save_archive = cereal::JSONOutputArchive;
using load_archive = cereal::JSONInputArchive;

namespace cereal {

namespace {

// ----------------------------------------------------------------------------
// List of types supported by this JSON exporter/importer.
using klv_type_list =
  kv::variant<
    double,
    int64_t,
    klv_0102_country_coding_method,
    klv_0102_security_classification,
    klv_0601_airbase_locations,
    klv_0601_control_command,
    klv_0601_country_codes,
    klv_0601_frame_rate,
    klv_0601_icing_detected,
    klv_0601_image_horizon_locations,
    klv_0601_image_horizon_pixel_pack,
    klv_0601_location_dlp,
    klv_0601_operational_mode,
    klv_0601_payload_record,
    klv_0601_platform_status,
    klv_0601_sensor_control_mode,
    klv_0601_sensor_fov_name,
    klv_0601_view_domain_interval,
    klv_0601_view_domain,
    klv_0601_wavelength_record,
    klv_0601_waypoint_record,
    klv_0601_weapon_general_status,
    klv_0601_weapons_store,
    klv_0806_aoi_type,
    klv_0806_user_defined_data_type_id,
    klv_0806_user_defined_data_type,
    klv_0806_user_defined_data,
    klv_0903_detection_status,
    klv_0903_fpa_index,
    klv_0903_location_pack,
    klv_0903_pixel_run,
    klv_0903_rho_pack,
    klv_0903_sigma_pack,
    klv_0903_velocity_pack,
    klv_0903_vtarget_pack,
    klv_0903_vtrackitem_pack,
    klv_1002_enumerations,
    klv_1002_section_data_pack,
    klv_1010_sdcc_flp,
    klv_1107_slant_range_pedigree,
    klv_1108_assessment_point,
    klv_1108_compression_profile,
    klv_1108_compression_type,
    klv_1108_metric_implementer,
    klv_1108_metric_period_pack,
    klv_1108_window_corners_pack,
    klv_1202_transformation_type,
    klv_1204_device_id_type,
    klv_1204_miis_id,
    klv_1206_image_plane,
    klv_1206_look_direction,
    klv_1303_apa,
    klv_1303_mdap< double >,
    klv_1303_mdap< uint64_t >,
    klv_blob,
    klv_lengthy< double >,
    klv_local_set,
    klv_universal_set,
    klv_uuid,
    std::set< klv_0601_generic_flag_data_bit >,
    std::set< klv_0601_positioning_method_source_bit >,
    std::set< klv_0601_weapon_engagement_status_bit >,
    std::string,
    std::vector< klv_0601_payload_record >,
    std::vector< klv_0601_wavelength_record >,
    std::vector< klv_0601_waypoint_record >,
    std::vector< klv_0601_weapons_store >,
    std::vector< klv_0903_location_pack >,
    std::vector< klv_0903_pixel_run >,
    std::vector< klv_0903_vtarget_pack >,
    std::vector< klv_local_set >,
    std::vector< uint16_t >,
    std::vector< uint64_t >,
    uint64_t
  >;

// ----------------------------------------------------------------------------
// Strings which encode which data format was used in an SDCC-FLP.

// ----------------------------------------------------------------------------
std::vector< std::pair< std::string, std::type_info const& > > const
format_names = {
  { "float", typeid( klv_float_format ) },
  { "imap", typeid( klv_imap_format ) }
};

// ----------------------------------------------------------------------------
std::string
find_format_name( std::type_info const& type )
{
  for( auto const& entry : format_names )
  {
    if( entry.second == type )
    {
      return entry.first;
    }
  }
  throw std::out_of_range( "no name assigned to given format" );
}

// ----------------------------------------------------------------------------
std::type_info const&
find_format_type( std::string const& name )
{
  for( auto const& entry : format_names )
  {
    if( entry.first == name )
    {
      return entry.second;
    }
  }
  throw std::out_of_range( "no format assigned to given name" );
}

// ----------------------------------------------------------------------------
// Several template helpers to aid in code brevity.

// ----------------------------------------------------------------------------
// TODO (C++14): Use STL version instead.
template< bool Value, class T = bool >
using enable_if_t = typename std::enable_if< Value, T >::type;

// ----------------------------------------------------------------------------
template< class T1, class T2 >
using enable_if_same_t = enable_if_t< std::is_same< T1, T2 >::value >;

// ----------------------------------------------------------------------------
template< class T >
struct is_cerealizable {
  static constexpr bool value = std::is_arithmetic< T >::value ||
                                std::is_same< T, std::string >::value ||
                                std::is_same< T, std::nullptr_t >::value;
};

// ----------------------------------------------------------------------------
template< class T >
using is_cerealizable_t = enable_if_t< is_cerealizable< T >::value >;

// ----------------------------------------------------------------------------
// Base class which provides utility functions to both exporter and importer.
// Abstracts away much of the reliance on cereal as a backend, so most of the
// logic of the actual exporter/importer classes can remain untouched if we
// decide to switch to a better JSON backend at some point.
template < class Archive >
class klv_json_base
{
protected:
  class scoped_lookup
  {
  public:
    scoped_lookup( klv_tag_traits_lookup const*& value,
                   klv_tag_traits_lookup const* new_value )
      : old_value{ value }, value{ value }
    {
      value = new_value;
    }

    ~scoped_lookup()
    {
      value = old_value;
    }

  private:
    klv_tag_traits_lookup const* old_value;
    klv_tag_traits_lookup const*& value;
  };

  class scoped_object
  {
  public:
    scoped_object( Archive& archive ) : m_archive( archive )
    {
      m_archive.startNode();
    }

    ~scoped_object()
    {
      m_archive.finishNode();
    }

  private:
    Archive& m_archive;
  };

  class scoped_array
  {
  public:
    scoped_array( Archive& archive ) : m_archive( archive )
    {
      m_archive.startNode();
      m_archive.makeArray();
    }

    ~scoped_array()
    {
      m_archive.finishNode();
    }

  private:
    Archive& m_archive;
  };

  klv_json_base( Archive& archive ) : m_archive( archive ), m_lookup{ nullptr }
  {}

  // Return internal cereal archive. Avoid excessive use.
  Archive&
  archive() const
  {
    return m_archive;
  }

  // Return a lookup object to find tag traits in the current context.
  klv_tag_traits_lookup const*
  lookup()
  {
    return m_lookup;
  }

  // Set the active lookup object. The value reverts when the returned object
  // goes out of scope.
  scoped_lookup
  push_lookup( klv_tag_traits_lookup const* lookup )
  {
    return scoped_lookup{ m_lookup, lookup };
  }

  // Declare that we have entered a new JSON object. Further reads/writes will
  // happen inside that object until the returned object's destructor goes out
  // of scope.
  scoped_object
  push_object()
  {
    return { m_archive };
  }

  // Declare that we have entered a new JSON array. Further reads/writes will
  // happen inside that object until the returned object's destructor goes out
  // of scope. As of right now, use push_object() instead when importing.
  scoped_array
  push_array()
  {
    return { m_archive };
  }

  // Get the next name (key in a key-value pair) to be written out.
  std::string const&
  next_name()
  {
    return m_next_name;
  }

  // Set the next name (key in a key-value pair) to be written out.
  void
  set_next_name( std::string const& name )
  {
    m_next_name = name;
    m_archive.setNextName( m_next_name.c_str() );
  }

private:
  Archive& m_archive;
  klv_tag_traits_lookup const* m_lookup;
  std::string m_next_name;
};

// ----------------------------------------------------------------------------
// Replace all underscores with hyphens in a string.
std::string hyphenify( std::string input )
{
  for( auto& c : input )
  {
    if( c == '_' )
    {
      c = '-';
    }
  }
  return input;
}

// ----------------------------------------------------------------------------
// Saves the member of a struct (mandatorily named 'value') with the given name.
#define SAVE_MEMBER( X ) save( hyphenify( #X ), value.X )

// ----------------------------------------------------------------------------
// Exports KLV objects. Relies heavily on template/type deduction to keep code
// relatively clean.
class klv_json_saver : public klv_json_base< save_archive >
{
public:
  klv_json_saver( save_archive& archive, bool verbose = true )
    : klv_json_base< save_archive >{ archive }, m_verbose{ verbose }
  {}

  template < class T, is_cerealizable_t< T > = true >
  void save( T const& value )
  {
    archive()( value );
  }

  template< class T,
            typename std::enable_if< std::is_enum< T >::value,
                                     bool >::type = true >
  void save( T value )
  {
    auto const object_scope = push_object();
    save( "integer", static_cast< uint64_t >( value ) );
    if( m_verbose )
    {
      std::stringstream ss;
      ss << value;
      save( "string", ss.str() );
    }
  }

  template< class T, typename std::enable_if< !std::is_enum< T >::value &&
                                              !is_cerealizable< T >::value,
                                              bool >::type = true >
  void save( T ) = delete; // Error? You must implement a new save() overload.

  template < class T >
  void save( std::string const& name, T const& value )
  {
    set_next_name( name );
    save( value );
  }

  template< class T >
  void save( std::vector< T > const& value )
  {
    auto const array_scope = push_array();
    for( auto const& item : value )
    {
      save( item );
    }
  }

  void save_base64( std::string const& name,
                    std::vector< uint8_t > const& value )
  {
    set_next_name( name );
    save_base64( value );
  }

  void save_base64( std::vector< uint8_t > const& value )
  {
    save( base64::encode( value.data(), value.size() ) );
  }

  template< class T >
  void save( std::set< T > const& value )
  {
    auto const array_scope = push_array();
    for( auto const& item : value )
    {
      save( item );
    }
  }

  void save( char const* value )
  {
    archive()( std::string{ value } );
  }


  void save( klv_timed_packet const& packet )
  {
    if( packet.timestamp.has_valid_frame() )
    {
      save( "frame", packet.timestamp.get_frame() );
    }
    else
    {
      save( "frame", nullptr );
    }

    if( packet.timestamp.has_valid_time() )
    {
      save( "microseconds", packet.timestamp.get_time_usec() );
    }
    else
    {
      save( "microseconds", nullptr );
    }

    save( "stream-index", packet.stream_index );

    save( packet.packet );
  }

  void save( klv_packet const& packet )
  {
    auto const outer_lookup = push_lookup( &klv_lookup_packet_traits() );
    save( "key", packet.key );
    auto const inner_lookup =
      push_lookup( lookup()->by_uds_key( packet.key ).subtag_lookup() );
    save( "value", packet.value );
  }

  // Named separately since klv_lds_key is just an integer, so can't have its
  // own overload of save() separate from regular integers
  void save_lds_key( klv_lds_key key )
  {
    auto const object_scope = push_object();
    save( "integer", key );
    if( m_verbose && lookup() )
    {
      save( "string", lookup()->by_tag( key ).name() );
    }
  }

  void save_lds_key( std::string const& name, klv_lds_key key )
  {
    set_next_name( name );
    save_lds_key( key );
  }

  void save( klv_uds_key const& key )
  {
    auto const object_scope = push_object();
    save_base64( "bytes", { key.cbegin(), key.cend() } );
    if( m_verbose )
    {
      std::stringstream ss;
      ss << key;
      save( "hex", ss.str() );
      if( lookup() )
      {
        save( "string", lookup()->by_uds_key( key ).name() );
      }
    }
  }

  void save( klv_value const& value )
  {
    if( !value.valid() )
    {
      save( nullptr );
      if( !value.empty() )
      {
        auto const name = next_name();
        save( name + "-unparsed-bytes", value.get< klv_blob >() );
      }
      return;
    }

    auto const visitor = klv_value_visitor{ *this, value };
    try
    {
      kv::visit_variant_types< klv_type_list >( visitor, value.type() );
    }
    catch( std::out_of_range const& e )
    {
      LOG_ERROR(
        kv::get_logger( "klv" ),
        "json export for type "
        << "`" << value.type_name() << "` "
        << "has not been implemented" );
      save( nullptr );
    }
  }

  void save( klv_local_set const& value )
  {
    auto const array_scope = push_array();
    for( auto const& entry : value )
    {
      auto const entry_scope = push_object();
      save_lds_key( "key", entry.first );
      if( lookup() && lookup()->by_tag( entry.first ).subtag_lookup() )
      {
        auto const lookup_scope =
          push_lookup( lookup()->by_tag( entry.first ).subtag_lookup() );
        save( "value", entry.second );
      }
      else
      {
        save( "value", entry.second );
      }
    }
  }

  void save( klv_universal_set const& value )
  {
    auto const array_scope = push_array();
    for( auto const& entry : value )
    {
      auto const entry_scope = push_object();
      save( "key", entry.first );
      if( lookup() && lookup()->by_uds_key( entry.first ).subtag_lookup() )
      {
        auto const inner_lookup_scope =
          push_lookup( lookup()->by_uds_key( entry.first ).subtag_lookup() );
        save( "value", entry.second );
      }
      else
      {
        save( "value", entry.second );
      }
    }
  }

  void save( klv_float_format const& )
  {}

  void save( klv_imap_format const& value )
  {
    save( "lower-bound", value.minimum() );
    save( "upper-bound", value.maximum() );
  }

  void save( klv_data_format const& value )
  {
    auto const object_scope = push_object();
    save( "type", find_format_name( typeid( value ) ) );
    kv::visit_types<
      klv_data_format_visitor,
      klv_float_format,
      klv_imap_format
      >( { *this, value }, typeid( value ) );

    if( value.fixed_length() )
    {
      save( "length", value.fixed_length() );
    }
    else
    {
      save( "length", nullptr );
    }
  }

  void save( klv_blob const& value )
  {
    save_base64( *value );
  }

  template< class T >
  void save( klv_lengthy< T > const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( length );
    SAVE_MEMBER( value );
  }

  template< class T >
  void save( kv::optional< T > const& value )
  {
    if( value )
    {
      save( *value );
    }
    else
    {
      save( nullptr );
    }
  }

  template< class T >
  void save( std::shared_ptr< T > const& value )
  {
    if( value )
    {
      save( *value );
    }
    else
    {
      save( nullptr );
    }
  }

  template< class T >
  void save( kv::interval< T > const& value )
  {
    auto const object_scope = push_object();
    save( "lower-bound", value.lower() );
    save( "upper-bound", value.upper() );
  }

  void save( klv_0601_airbase_locations const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( take_off_location );
    SAVE_MEMBER( recovery_location );
  }

  void save( klv_0601_control_command const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( id );
    SAVE_MEMBER( string );
    SAVE_MEMBER( timestamp );
  }

  void save( klv_0601_country_codes const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( coding_method );
    SAVE_MEMBER( overflight_country );
    SAVE_MEMBER( operator_country );
    SAVE_MEMBER( country_of_manufacture );
  }

  void save( klv_0601_frame_rate const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( numerator );
    SAVE_MEMBER( denominator );
  }

  void save( klv_0601_image_horizon_locations const& value )
  {
    // No object scope so it's flat with the rest of
    // klv_0601_image_horizon_pixel_pack
    SAVE_MEMBER( latitude0 );
    SAVE_MEMBER( longitude0 );
    SAVE_MEMBER( latitude1 );
    SAVE_MEMBER( longitude1 );
  }

  void save( klv_0601_image_horizon_pixel_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( x0 );
    SAVE_MEMBER( y0 );
    SAVE_MEMBER( x1 );
    SAVE_MEMBER( y1 );
    if( value.locations )
    {
      save( value.locations );
    }
  }

  void save( klv_0601_location_dlp const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( latitude );
    SAVE_MEMBER( longitude );
    SAVE_MEMBER( altitude );
  }

  void save( klv_0601_payload_record const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( id );
    SAVE_MEMBER( type );
    SAVE_MEMBER( name );
  }

  void save( klv_0601_view_domain_interval const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( start );
    SAVE_MEMBER( range );
    SAVE_MEMBER( semi_length );
  }

  void save( klv_0601_view_domain const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( azimuth );
    SAVE_MEMBER( elevation );
    SAVE_MEMBER( roll );
  }

  void save( klv_0601_wavelength_record const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( id );
    SAVE_MEMBER( min );
    SAVE_MEMBER( max );
    SAVE_MEMBER( name );
  }

  void save( klv_0601_waypoint_record const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( id );
    SAVE_MEMBER( order );
    SAVE_MEMBER( info );
    SAVE_MEMBER( location );
  }

  void save( klv_0601_weapons_store const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( station_id );
    SAVE_MEMBER( hardpoint_id );
    SAVE_MEMBER( carriage_id );
    SAVE_MEMBER( store_id );
    SAVE_MEMBER( general_status );
    SAVE_MEMBER( engagement_status );
    SAVE_MEMBER( weapon_type );
  }

  void save( klv_0806_user_defined_data_type_id const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( type );
    SAVE_MEMBER( id );
  }

  void save( klv_0806_user_defined_data const& value )
  {
    save_base64( value.bytes );
  }

  void save( klv_0903_fpa_index const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( row );
    SAVE_MEMBER( column );
  }

  void save( klv_0903_location_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( latitude );
    SAVE_MEMBER( longitude );
    SAVE_MEMBER( altitude );
    SAVE_MEMBER( sigma );
    SAVE_MEMBER( rho );
  }

  void save( klv_0903_pixel_run const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( index );
    SAVE_MEMBER( length );
  }

  void save( klv_0903_rho_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( east_north );
    SAVE_MEMBER( east_up );
    SAVE_MEMBER( north_up );
  }

  void save( klv_0903_sigma_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( east );
    SAVE_MEMBER( north );
    SAVE_MEMBER( up );
  }

  void save( klv_0903_velocity_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( east );
    SAVE_MEMBER( north );
    SAVE_MEMBER( up );
    SAVE_MEMBER( sigma );
    SAVE_MEMBER( rho );
  }

  void save( klv_0903_vtarget_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( id );
    SAVE_MEMBER( set );
  }

  void save( klv_0903_vtrackitem_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( id );
    SAVE_MEMBER( set );
  }

  void save( klv_1002_enumerations const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( compression_method );
    SAVE_MEMBER( data_type );
    SAVE_MEMBER( source );
  }

  void save( klv_1002_section_data_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( section_x );
    SAVE_MEMBER( section_y );
    SAVE_MEMBER( measurements );
    SAVE_MEMBER( uncertainty );
    SAVE_MEMBER( plane_x_scale );
    SAVE_MEMBER( plane_y_scale );
    SAVE_MEMBER( plane_constant );
  }

  void save( klv_1010_sdcc_flp const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( members );
    SAVE_MEMBER( sigma );
    SAVE_MEMBER( rho );
    SAVE_MEMBER( sigma_length );
    SAVE_MEMBER( rho_length );
    SAVE_MEMBER( sigma_uses_imap );
    SAVE_MEMBER( rho_uses_imap );
    SAVE_MEMBER( long_parse_control );
    SAVE_MEMBER( sparse );
  }

  void save( klv_1108_metric_implementer const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( organization );
    SAVE_MEMBER( subgroup );
  }

  void save( klv_1108_metric_period_pack const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( timestamp );
    SAVE_MEMBER( offset );
  }

  void save( klv_1108_window_corners_pack const& value )
  {
    auto const object_scope = push_object();
    save( "min-x", value.bbox.min_x() );
    save( "min-y", value.bbox.min_y() );
    save( "max-x", value.bbox.max_x() );
    save( "max-y", value.bbox.max_y() );
  }

  void save( klv_1204_miis_id const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( version );
    SAVE_MEMBER( sensor_id_type );
    SAVE_MEMBER( platform_id_type );
    SAVE_MEMBER( sensor_id );
    SAVE_MEMBER( platform_id );
    SAVE_MEMBER( window_id );
    SAVE_MEMBER( minor_id );
  }

  template< class T >
  void save( klv_1303_mdap< T > const& value )
  {
    auto const object_scope = push_object();
    SAVE_MEMBER( sizes );
    SAVE_MEMBER( elements );
    SAVE_MEMBER( element_size );
    SAVE_MEMBER( apa );
    SAVE_MEMBER( apa_params_length );
    SAVE_MEMBER( imap_params );
  }

  void save( klv_uuid const& value )
  {
    auto const object_scope = push_object();
    save_base64( "bytes", { value.bytes.begin(), value.bytes.end() } );
    if( m_verbose )
    {
      std::stringstream ss;
      ss << value;
      save( "hex", ss.str() );
    }
  }

  private:
  struct klv_value_visitor {
    template< class T >
    void operator()() const
    {
      saver.save( value.get< T >() );
    }

    klv_json_saver& saver;
    klv_value const& value;
  };

  struct klv_data_format_visitor {
    template< class T >
    void operator()() const
    {
      saver.save( dynamic_cast< T const& >( value ) );
    }

    klv_json_saver& saver;
    klv_data_format const& value;
  };

  bool m_verbose;
};

// ----------------------------------------------------------------------------
// Shortens the wordy template header needed to overload load().
#define LOAD_TEMPLATE( TYPE ) \
  template< class T, enable_if_same_t< T, TYPE > = true >

// ----------------------------------------------------------------------------
// Shortens the wordy template header needed to overload load() with some kind
// of templated container.
#define LOAD_CONTAINER_TEMPLATE( TYPE ) \
  template< class T, \
            enable_if_same_t< T, TYPE< typename T::value_type > > = true >

// ----------------------------------------------------------------------------
// Loads a value with the given type into the given variable name.
#define LOAD_VALUE( NAME, T ) auto NAME = load< T >( hyphenify( #NAME ) )

// ----------------------------------------------------------------------------
// Loads a value into the given variable name. Type is deduced by assuming NAME
// is also the name of a data member of the struct T.
#define LOAD_MEMBER( NAME ) \
  auto NAME = load< decltype( T::NAME ) >( hyphenify( #NAME ) )

// ----------------------------------------------------------------------------
// Imports KLV objects. Relies heavily on templates to keep code relatively
// clean, and to work around the lack of return-type-based function overloading
// in C++.
struct klv_json_loader : public klv_json_base< load_archive >
{
  klv_json_loader( load_archive& archive )
    : klv_json_base< load_archive >{ archive }
  {}

  // Return the size of the array we are in.
  size_t array_size()
  {
    size_type size;
    archive().loadSize( size );
    return static_cast< size_t >( size );
  }

  // Return the KLV tag traits lookup object, throwing an error if none exists.
  klv_tag_traits_lookup const& assert_lookup()
  {
    if( !lookup() )
    {
      throw std::logic_error( "type not provided for klv json import" );
    }
    return *lookup();
  }

  // Return true if the node we are reading has a value of null.
  bool load_null() {
    try
    {
      load< nullptr_t >();
      return true;
    }
    catch( std::runtime_error const& )
    {
      return false;
    }
  }

  template < class T, is_cerealizable_t< T > = true >
  T load()
  {
    T value;
    archive()( value );
    return value;
  }

  template< class T, enable_if_t< std::is_enum< T >::value, bool > = true >
  T load()
  {
    auto const object_scope = push_object();
    return static_cast< T >( load< uint64_t >( "integer" ) );
  }

  template < class T >
  T load( std::string const& name )
  {
    set_next_name( name );
    return load< T >();
  }

  std::vector< uint8_t > load_base64( std::string const& name )
  {
    set_next_name( name );
    return load_base64();
  }

  std::vector< uint8_t > load_base64()
  {
    auto const string = base64::decode( load< std::string >() );
    return { string.begin(), string.end() };
  }

  LOAD_CONTAINER_TEMPLATE( std::vector )
  T load()
  {
    auto const object_scope = push_object();
    auto const size = array_size();
    T result;
    result.reserve( size );
    for( size_t i = 0; i < size; ++i )
    {
      result.emplace_back( std::move( load< typename T::value_type >() ) );
    }
    return result;
  }

  LOAD_CONTAINER_TEMPLATE( std::set )
  T load()
  {
    auto const object_scope = push_object();
    auto const size = array_size();
    T result;
    for( size_t i = 0; i < size; ++i )
    {
      result.emplace( std::move( load< typename T::value_type >() ) );
    }
    return result;
  }

  LOAD_CONTAINER_TEMPLATE( kv::optional )
  T load()
  {
    if( load_null() )
    {
      return kv::nullopt;
    }

    return load< typename T::value_type >();
  }

  LOAD_TEMPLATE( std::shared_ptr< klv_data_format > )
  T load()
  {
    if( load_null() )
    {
      return nullptr;
    }

    auto const object_scope = push_object();
    auto const& type = find_format_type( load< std::string >( "type" ) );
    return kv::visit_types_return<
      T,
      klv_data_format_visitor,
      klv_float_format,
      klv_imap_format
      >( { *this }, type );
  }

  LOAD_CONTAINER_TEMPLATE( kv::interval )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_VALUE( lower_bound, typename T::value_type );
    LOAD_VALUE( upper_bound, typename T::value_type );
    return { lower_bound, upper_bound };
  }

  LOAD_CONTAINER_TEMPLATE( klv_lengthy )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( length );
    LOAD_MEMBER( value );
    return { std::move( value ), std::move( length ) };
  }

  LOAD_TEMPLATE( klv_timed_packet )
  T load()
  {
    LOAD_VALUE( frame, kv::optional< int64_t > );
    LOAD_VALUE( microseconds, kv::optional< int64_t > );
    LOAD_VALUE( packet, klv_packet );
    LOAD_VALUE( stream_index, uint64_t );
    kv::timestamp ts;
    if( frame )
    {
      ts.set_frame( *frame );
    }
    if( microseconds )
    {
      ts.set_time_usec( *microseconds );
    }
    return { std::move( packet ), ts, stream_index };
  }

  LOAD_TEMPLATE( klv_packet )
  T load()
  {
    auto const outer_lookup = push_lookup( &klv_lookup_packet_traits() );
    LOAD_VALUE( key, klv_uds_key );
    auto const& traits = assert_lookup().by_uds_key( key );
    auto const inner_lookup = push_lookup( traits.subtag_lookup() );
    auto value = load( "value", traits.type() );
    return { std::move( key ), std::move( value ) };
  }

  // Named separately since klv_lds_key is just an integer, so can't have its
  // own overload of load() separate from regular integers
  klv_lds_key load_lds_key()
  {
    auto const object_scope = push_object();
    return load< uint64_t >( "integer" );
  }

  klv_lds_key load_lds_key( std::string const& name )
  {
    set_next_name( name );
    return load_lds_key();
  }

  LOAD_TEMPLATE( klv_uds_key )
  T load()
  {
    auto const object_scope = push_object();
    auto const bytes = load_base64( "bytes" );
    if( bytes.size() != klv_uds_key::length )
    {
      throw std::runtime_error( "uds key has incorrect number of bytes" );
    }
    return klv_uds_key{ bytes.data() };
  }

  klv_value load( std::type_info const& type )
  {
    if( load_null() )
    {
      try
      {
        auto bytes = load< klv_blob >( next_name() + "-unparsed-bytes" );
        return bytes;
      }
      catch( std::runtime_error const& )
      {
        return {};
      }
    }

    auto const visitor = klv_value_visitor{ *this };
    try
    {
      return kv::visit_variant_types_return< klv_value, klv_type_list >(
        visitor, type );
    }
    catch( std::out_of_range const& e )
    {
      LOG_ERROR(
        kv::get_logger( "klv" ),
        "json import for type "
        << "`" << kv::demangle( type.name() ) << "` "
        << "has not been implemented" );
      return {};
    }
  }

  klv_value load( std::string const& name, std::type_info const& type )
  {
    set_next_name( name );
    return load( type );
  }

  LOAD_TEMPLATE( klv_blob )
  T load()
  {
    return { load_base64() };
  }

  LOAD_TEMPLATE( klv_local_set )
  T load()
  {
    auto const object_scope = push_object();
    auto const size = array_size();
    T result;
    for( size_t i = 0; i < size; ++i )
    {
      auto const entry_scope = push_object();
      auto const key = load_lds_key( "key" );
      klv_value value;
      auto const& type = assert_lookup().by_tag( key ).type();
      if( assert_lookup().by_tag( key ).subtag_lookup() )
      {
        auto const lookup_scope =
          push_lookup( assert_lookup().by_tag( key ).subtag_lookup() );
        value = load( "value", type );
      }
      else
      {
        value = load( "value", type );
      }
      result.add( key, std::move( value ) );
    }
    return result;
  }

  LOAD_TEMPLATE( klv_universal_set )
  T load()
  {
    auto const object_scope = push_object();
    auto const size = array_size();
    T result;
    for( size_t i = 0; i < size; ++i )
    {
      auto const entry_scope = push_object();
      auto const key = load< klv_uds_key >( "key" );
      klv_value value;
      auto const& type = assert_lookup().by_uds_key( key ).type();
      if( assert_lookup().by_uds_key( key ).subtag_lookup() )
      {
        auto const lookup_scope =
          push_lookup( assert_lookup().by_uds_key( key ).subtag_lookup() );
        value = load( "value", type );
      }
      else
      {
        value = load( "value", type );
      }
      result.add( key, std::move( value ) );
    }
    return result;
  }

  LOAD_TEMPLATE( klv_float_format )
  T load()
  {
    LOAD_VALUE( length, kv::optional< size_t > );
    return length ? T{ *length } : T{};
  }

  LOAD_TEMPLATE( klv_imap_format )
  T load()
  {
    LOAD_VALUE( lower_bound, double );
    LOAD_VALUE( upper_bound, double );
    LOAD_VALUE( length, kv::optional< size_t > );
    return length ? T{ lower_bound, upper_bound, *length }
                  : T{ lower_bound, upper_bound };
  }

  LOAD_TEMPLATE( klv_0601_airbase_locations )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( take_off_location );
    LOAD_MEMBER( recovery_location );
    return { std::move( take_off_location ),
             std::move( recovery_location ) };
  }

  LOAD_TEMPLATE( klv_0601_control_command )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( id );
    LOAD_MEMBER( string );
    LOAD_MEMBER( timestamp );
    return { std::move( id ),
             std::move( string ),
             std::move( timestamp ) };
  }

  LOAD_TEMPLATE( klv_0601_country_codes )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( coding_method );
    LOAD_MEMBER( overflight_country );
    LOAD_MEMBER( operator_country );
    LOAD_MEMBER( country_of_manufacture );
    return { std::move( coding_method ),
             std::move( overflight_country ),
             std::move( operator_country ),
             std::move( country_of_manufacture ) };
  }

  LOAD_TEMPLATE( klv_0601_frame_rate )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( numerator );
    LOAD_MEMBER( denominator );
    return { std::move( numerator ),
             std::move( denominator ) };
  }

  LOAD_TEMPLATE( klv_0601_image_horizon_locations )
  T load()
  {
    // No object scope so it's flat with the rest of
    // klv_0601_image_horizon_pixel_pack
    LOAD_MEMBER( latitude0 );
    LOAD_MEMBER( longitude0 );
    LOAD_MEMBER( latitude1 );
    LOAD_MEMBER( longitude1 );
    return { std::move( latitude0 ),
             std::move( longitude0 ),
             std::move( latitude1 ),
             std::move( longitude1 ) };
  }

  LOAD_TEMPLATE( klv_0601_image_horizon_pixel_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( x0 );
    LOAD_MEMBER( y0 );
    LOAD_MEMBER( x1 );
    LOAD_MEMBER( y1 );
    decltype( klv_0601_image_horizon_pixel_pack::locations ) locations;
    try
    {
      locations = load< klv_0601_image_horizon_locations >();
    }
    catch( std::runtime_error const& )
    {}

    return { std::move( x0 ),
             std::move( y0 ),
             std::move( x1 ),
             std::move( y1 ),
             std::move( locations ) };
  }

  LOAD_TEMPLATE( klv_0601_location_dlp )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( latitude );
    LOAD_MEMBER( longitude );
    LOAD_MEMBER( altitude );
    return { std::move( latitude ),
             std::move( longitude ),
             std::move( altitude ) };
  }

  LOAD_TEMPLATE( klv_0601_payload_record )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( id );
    LOAD_MEMBER( type );
    LOAD_MEMBER( name );
    return { std::move( id ),
             std::move( type ),
             std::move( name ) };
  }

  LOAD_TEMPLATE( klv_0601_view_domain_interval )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( start );
    LOAD_MEMBER( range );
    LOAD_MEMBER( semi_length );
    return { std::move( start ),
             std::move( range ),
             std::move( semi_length ) };
  }

  LOAD_TEMPLATE( klv_0601_view_domain )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( azimuth );
    LOAD_MEMBER( elevation );
    LOAD_MEMBER( roll );
    return { std::move( azimuth ),
             std::move( elevation ),
             std::move( roll ) };
  }

  LOAD_TEMPLATE( klv_0601_wavelength_record )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( id );
    LOAD_MEMBER( min );
    LOAD_MEMBER( max );
    LOAD_MEMBER( name );
    return { std::move( id ),
             std::move( min ),
             std::move( max ),
             std::move( name ) };
  }

  LOAD_TEMPLATE( klv_0601_waypoint_record )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( id );
    LOAD_MEMBER( order );
    LOAD_MEMBER( info );
    LOAD_MEMBER( location );
    return { std::move( id ),
             std::move( order ),
             std::move( info ),
             std::move( location ) };
  }

  LOAD_TEMPLATE( klv_0601_weapons_store )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( station_id );
    LOAD_MEMBER( hardpoint_id );
    LOAD_MEMBER( carriage_id );
    LOAD_MEMBER( store_id );
    LOAD_MEMBER( general_status );
    LOAD_MEMBER( engagement_status );
    LOAD_MEMBER( weapon_type );
    return { std::move( station_id ),
             std::move( hardpoint_id ),
             std::move( carriage_id ),
             std::move( store_id ),
             std::move( general_status ),
             std::move( engagement_status ),
             std::move( weapon_type ) };
  }

  LOAD_TEMPLATE( klv_0806_user_defined_data_type_id )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( type );
    LOAD_MEMBER( id );
    return { std::move( type ),
             std::move( id ) };
  }

  LOAD_TEMPLATE( klv_0806_user_defined_data )
  T load()
  {
    return { load_base64() };
  }

  LOAD_TEMPLATE( klv_0903_fpa_index )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( row );
    LOAD_MEMBER( column );
    return { std::move( row ),
             std::move( column ) };
  }

  LOAD_TEMPLATE( klv_0903_location_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( latitude );
    LOAD_MEMBER( longitude );
    LOAD_MEMBER( altitude );
    LOAD_MEMBER( sigma );
    LOAD_MEMBER( rho );
    return { std::move( latitude ),
             std::move( longitude ),
             std::move( altitude ),
             std::move( sigma ),
             std::move( rho ) };
  }

  LOAD_TEMPLATE( klv_0903_pixel_run )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( index );
    LOAD_MEMBER( length );
    return { std::move( index ),
             std::move( length ) };
  }

  LOAD_TEMPLATE( klv_0903_rho_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( east_north );
    LOAD_MEMBER( east_up );
    LOAD_MEMBER( north_up );
    return { std::move( east_north ),
             std::move( east_up ),
             std::move( north_up ) };
  }

  LOAD_TEMPLATE( klv_0903_sigma_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( east );
    LOAD_MEMBER( north );
    LOAD_MEMBER( up );
    return { std::move( east ),
             std::move( north ),
             std::move( up ) };
  }

  LOAD_TEMPLATE( klv_0903_velocity_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( east );
    LOAD_MEMBER( north );
    LOAD_MEMBER( up );
    LOAD_MEMBER( sigma );
    LOAD_MEMBER( rho );
    return { std::move( east ),
             std::move( north ),
             std::move( up ),
             std::move( sigma ),
             std::move( rho ) };
  }

  LOAD_TEMPLATE( klv_0903_vtarget_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( id );
    LOAD_MEMBER( set );
    return { std::move( id ),
             std::move( set ) };
  }

  LOAD_TEMPLATE( klv_0903_vtrackitem_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( id );
    LOAD_MEMBER( set );
    return { std::move( id ),
             std::move( set ) };
  }

  LOAD_TEMPLATE( klv_1002_enumerations )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( compression_method );
    LOAD_MEMBER( data_type );
    LOAD_MEMBER( source );
    return { std::move( compression_method ),
             std::move( data_type ),
             std::move( source ) };
  }

  LOAD_TEMPLATE( klv_1002_section_data_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( section_x );
    LOAD_MEMBER( section_y );
    LOAD_MEMBER( measurements );
    LOAD_MEMBER( uncertainty );
    LOAD_MEMBER( plane_x_scale );
    LOAD_MEMBER( plane_y_scale );
    LOAD_MEMBER( plane_constant );
    return { std::move( section_x ),
             std::move( section_y ),
             std::move( measurements ),
             std::move( uncertainty ),
             std::move( plane_x_scale ),
             std::move( plane_y_scale ),
             std::move( plane_constant ) };
  }

  LOAD_TEMPLATE( klv_1010_sdcc_flp )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( members );
    LOAD_MEMBER( sigma );
    LOAD_MEMBER( rho );
    LOAD_MEMBER( sigma_length );
    LOAD_MEMBER( rho_length );
    LOAD_MEMBER( sigma_uses_imap );
    LOAD_MEMBER( rho_uses_imap );
    LOAD_MEMBER( long_parse_control );
    LOAD_MEMBER( sparse );
    return { std::move( members ),
             std::move( sigma ),
             std::move( rho ),
             std::move( sigma_length ),
             std::move( rho_length ),
             std::move( sigma_uses_imap ),
             std::move( rho_uses_imap ),
             std::move( long_parse_control ),
             std::move( sparse ) };
  }

  LOAD_TEMPLATE( klv_1108_metric_implementer )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( organization );
    LOAD_MEMBER( subgroup );
    return { std::move( organization ),
             std::move( subgroup ) };
  }


  LOAD_TEMPLATE( klv_1108_metric_period_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( timestamp );
    LOAD_MEMBER( offset );
    return { std::move( timestamp ),
             std::move( offset ) };
  }

  LOAD_TEMPLATE( klv_1108_window_corners_pack )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_VALUE( min_x, uint16_t );
    LOAD_VALUE( min_y, uint16_t );
    LOAD_VALUE( max_x, uint16_t );
    LOAD_VALUE( max_y, uint16_t );
    return { { std::move( min_x ),
               std::move( min_y ),
               std::move( max_x ),
               std::move( max_y ) } };
  }

  LOAD_TEMPLATE( klv_1204_miis_id )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( version );
    LOAD_MEMBER( sensor_id_type );
    LOAD_MEMBER( platform_id_type );
    LOAD_MEMBER( sensor_id );
    LOAD_MEMBER( platform_id );
    LOAD_MEMBER( window_id );
    LOAD_MEMBER( minor_id );
    return { std::move( version ),
             std::move( sensor_id_type ),
             std::move( platform_id_type ),
             std::move( sensor_id ),
             std::move( platform_id ),
             std::move( window_id ),
             std::move( minor_id ) };
  }

  LOAD_CONTAINER_TEMPLATE( klv_1303_mdap )
  T load()
  {
    auto const object_scope = push_object();
    LOAD_MEMBER( sizes );
    LOAD_MEMBER( elements );
    LOAD_MEMBER( element_size );
    LOAD_MEMBER( apa );
    LOAD_MEMBER( apa_params_length );
    LOAD_MEMBER( imap_params );
    return { std::move( sizes ),
             std::move( elements ),
             std::move( element_size ),
             std::move( apa ),
             std::move( apa_params_length ),
             std::move( imap_params ) };
  }

  LOAD_TEMPLATE( klv_uuid )
  T load()
  {
    auto const object_scope = push_object();
    auto const bytes = load_base64( "bytes" );
    if( bytes.size() != 16 )
    {
      throw std::runtime_error( "uuid has incorrect number of bytes" );
    }

    klv_uuid value;
    std::copy( bytes.cbegin(), bytes.cend(), value.bytes.begin() );
    return value;
  }

  private:
  struct klv_value_visitor {
    template< class T >
    klv_value operator()() const
    {
      return loader.template load< T >();
    }

    klv_json_loader& loader;
  };

  struct klv_data_format_visitor {
    template< class T >
    std::shared_ptr< klv_data_format > operator()() const
    {
      return std::make_shared< T >( std::move( loader.load< T >() ) );
    }

    klv_json_loader& loader;
  };
};

} // namespace

// ----------------------------------------------------------------------------
void
save( save_archive& archive, klv_packet const& packet )
{
  klv_json_saver saver{ archive };
  saver.save( packet );
}

// ----------------------------------------------------------------------------
void
load( load_archive& archive, klv_packet& packet )
{
  klv_json_loader loader{ archive };
  packet = loader.load< klv_packet >();
}

// ----------------------------------------------------------------------------
void
save( save_archive& archive, klv_timed_packet const& packet )
{
  klv_json_saver saver{ archive };
  saver.save( packet );
}

// ----------------------------------------------------------------------------
void
load( load_archive& archive, klv_timed_packet& packet )
{
  klv_json_loader loader{ archive };
  packet = loader.load< klv_timed_packet >();
}

} // namespace cereal
