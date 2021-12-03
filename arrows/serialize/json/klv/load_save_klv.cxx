// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/serialize/json/klv/load_save_klv.h>

#include <arrows/klv/klv_0104_new.h>
#include <arrows/klv/klv_0601_new.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>

#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/cereal.hpp>
#include <vital/internal/cereal/types/vector.hpp>
#include <vital/util/visit.h>

#include <vital/range/iota.h>

using namespace ::kwiver::arrows::klv;
namespace kv = ::kwiver::vital;
namespace kvr = ::kwiver::vital::range;

using save_archive = ::cereal::JSONOutputArchive;
using load_archive = ::cereal::JSONInputArchive;

namespace cereal {

namespace {

// ----------------------------------------------------------------------------
// Contexts: To communicate to lower-level nodes e.g. which local set they are
// in.

// ----------------------------------------------------------------------------
struct save_context
{
  klv_tag_traits_lookup const* lookup = nullptr;
};

// ----------------------------------------------------------------------------
struct load_context
{
  klv_tag_traits_lookup const* lookup = nullptr;
};

// ----------------------------------------------------------------------------
// Type traits: To store the correspondence between strings and C++ types.

// ----------------------------------------------------------------------------
struct value_type_traits
{
  std::type_info const& type;
  std::string name;
};

// ----------------------------------------------------------------------------
std::vector< value_type_traits > const&
type_traits()
{
  static std::vector< value_type_traits > const traits = {
    { typeid( void ),
      "empty" },
    { typeid( double ),
      "float" },
    { typeid( int64_t ),
      "signed integer" },
    { typeid( uint64_t ),
      "unsigned integer" },
    { typeid( std::string ),
      "string" },
    { typeid( klv_local_set ),
      "local set" },
    { typeid( klv_universal_set ),
      "universal set" },
    { typeid( klv_blob ),
      "unparsed bytes" },
    { typeid( klv_0601_control_command ),
      "control command pack" },
    { typeid( klv_0601_frame_rate ),
      "frame rate pack" },
    { typeid( klv_0601_icing_detected ),
      "icing detected enumeration" },
    { typeid( klv_0601_operational_mode ),
      "operational mode enumeration" },
    { typeid( klv_0601_platform_status ),
      "platform status enumeration" },
    { typeid( klv_0601_sensor_control_mode ),
      "sensor control mode enumeration" },
    { typeid( klv_0601_sensor_fov_name ),
      "sensor fov name enumeration" },
    { typeid( klv_1108_assessment_point ),
      "assessment point enumeration" },
    { typeid( klv_1108_compression_profile ),
      "compression profile enumeration" },
    { typeid( klv_1108_compression_type ),
      "compression type enumeration" },
    { typeid( klv_1108_metric_implementer ),
      "metric implementer pack" },
    { typeid( klv_1108_metric_period_pack ),
      "metric period pack" },
    { typeid( klv_1108_window_corners_pack ),
      "window corners pack" }, };

  return traits;
}

// ----------------------------------------------------------------------------
value_type_traits const&
unknown_type_trait()
{
  static auto const result = value_type_traits{ typeid( void ), "unknown" };
  return result;
}

// ----------------------------------------------------------------------------
value_type_traits const&
type_traits_of( std::type_info const& type )
{
  static auto const map =
    ( [](){
        std::map< std::type_index, value_type_traits const* > result;
        for( auto const& entry : type_traits() )
        {
          result.emplace( entry.type, &entry );
        }
        return result;
      } )();
  auto const it = map.find( type );
  if( it == map.end() )
  {
    return unknown_type_trait();
  }
  return *it->second;
}

// ----------------------------------------------------------------------------
value_type_traits const&
type_traits_of( std::string const& name )
{
  static auto const map =
    ( [](){
        std::map< std::string, value_type_traits const* > result;
        for( auto const& entry : type_traits() )
        {
          result.emplace( entry.name, &entry );
        }
        return result;
      } )();
  auto const it = map.find( name );
  if( it == map.end() )
  {
    return unknown_type_trait();
  }
  return *it->second;
}

// ----------------------------------------------------------------------------
// Load and save for keys.

// ----------------------------------------------------------------------------
void
save_uds_key( save_archive& archive, klv_uds_key const& key,
              save_context& context )
{
  archive.startNode();
  archive( make_nvp( "type", std::string{ "universal key" } ) );

  if( context.lookup )
  {
    archive( make_nvp( "name", context.lookup->by_uds_key( key ).name() ) );
  }

  {
    std::stringstream ss;
    ss << key;
    archive( make_nvp( "hex", ss.str() ) );
  }

  {
    std::vector< uint8_t > bytes = { key.cbegin(), key.cend() };
    archive( make_nvp( "value", bytes ) );
  }

  archive.finishNode();
}

// ----------------------------------------------------------------------------
klv_uds_key
load_uds_key( load_archive& archive, load_context& context )
{
  archive.startNode();

  std::vector< uint8_t > bytes;
  archive( make_nvp( "value", bytes ) );
  if( bytes.size() != klv_uds_key::length )
  {
    throw std::runtime_error(
            "JSON KLV import: UDS key has incorrect number of bytes" );
  }
  archive.finishNode();
  return klv_uds_key{ bytes.cbegin() };
}

// ----------------------------------------------------------------------------
void
save_lds_key( save_archive& archive, klv_lds_key const& key,
              save_context& context )
{
  archive.startNode();
  archive( make_nvp( "type", std::string{ "local tag" } ) );
  if( context.lookup )
  {
    archive( make_nvp( "name", context.lookup->by_tag( key ).name() ) );
  }
  archive( make_nvp( "value", key ) );
  archive.finishNode();
}

// ----------------------------------------------------------------------------
klv_lds_key
load_lds_key( load_archive& archive, load_context& context )
{
  archive.startNode();

  klv_lds_key result;
  archive( make_nvp( "value", result ) );
  archive.finishNode();
  return result;
}

// ----------------------------------------------------------------------------
// Forward-declared for recursive use in visitors.

// ----------------------------------------------------------------------------
void save_value( save_archive& archive, klv_value const& value,
                 save_context& context );

// ----------------------------------------------------------------------------
klv_value load_value( load_archive& archive, load_context& context );

// ----------------------------------------------------------------------------
// Visitors: To route a value to the appropriate saver/loader for its type.

// ----------------------------------------------------------------------------
struct save_value_visitor
{
  template < class T,
             typename std::enable_if< !std::is_enum< T >::value,
                                      bool >::type = true >
  void
  operator()() const
  {
    archive( make_nvp( "value", value.get< T >() ) );
  }

  template < class T,
             typename std::enable_if< std::is_enum< T >::value,
                                      bool >::type = true >
  void
  operator()() const
  {
    auto const& typed_value = value.get< T >();
    {
      std::stringstream ss;
      ss << typed_value;
      archive( make_nvp( "name", ss.str() ) );
    }
    archive( make_nvp( "value", static_cast< uint64_t >( typed_value ) ) );
  }

  save_archive& archive;
  klv_value const& value;
  save_context& context;
};

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_local_set >( ) const {
  auto const& local_set = value.get< klv_local_set >();
  archive( make_nvp( "size", local_set.size() ) );
  archive.setNextName( "value" );
  archive.startNode();
  archive.makeArray();
  for( auto const& entry : local_set )
  {
    archive.startNode();
    archive.setNextName( "key" );
    save_lds_key( archive, entry.first, context );
    archive.setNextName( "value" );
    auto const outer_context = context;
    if( context.lookup )
    {
      context.lookup = context.lookup->by_tag( entry.first ).subtag_lookup();
    }
    save_value( archive, entry.second, context );
    context = outer_context;
    archive.finishNode();
  }
  archive.finishNode();
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_universal_set >( ) const {
  auto const& universal_set = value.get< klv_universal_set >();
  archive( make_nvp( "size", universal_set.size() ) );
  archive.setNextName( "value" );
  archive.startNode();
  archive.makeArray();
  for( auto const& entry : universal_set )
  {
    archive.startNode();
    archive.setNextName( "key" );
    save_uds_key( archive, entry.first, context );
    archive.setNextName( "value" );
    auto const outer_context = context;
    if( context.lookup )
    {
      context.lookup =
        context.lookup->by_uds_key( entry.first ).subtag_lookup();
    }
    save_value( archive, entry.second, context );
    context = outer_context;
    archive.finishNode();
  }
  archive.finishNode();
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_blob >( ) const {
  auto const& blob = value.get< klv_blob >();
  {
    std::stringstream ss;
    ss << blob;
    archive( make_nvp( "hex", ss.str() ) );
  }
  archive( make_nvp( "value", *blob ) );
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_0601_frame_rate >( ) const
{
  auto const& typed_value = value.get< klv_0601_frame_rate >();
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "numerator", typed_value.numerator ) );
  archive( make_nvp( "denominator", typed_value.denominator ) );
  archive.finishNode();
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_0601_control_command >( )
const {
  auto const& typed_value = value.get< klv_0601_control_command >();
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "id", typed_value.id ) );
  archive( make_nvp( "string", typed_value.string ) );
  archive( make_nvp( "timestamp", typed_value.timestamp ) );
  archive.finishNode();
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_1108_metric_period_pack >( )
const {
  auto const& typed_value = value.get< klv_1108_metric_period_pack >();
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "timestamp", typed_value.timestamp ) );
  archive( make_nvp( "offset", typed_value.offset ) );
  archive.finishNode();
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_1108_window_corners_pack >( )
const {
  auto const& typed_value = value.get< klv_1108_window_corners_pack >();
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "min_x", typed_value.bbox.min_x() ) );
  archive( make_nvp( "min_y", typed_value.bbox.min_y() ) );
  archive( make_nvp( "max_x", typed_value.bbox.max_x() ) );
  archive( make_nvp( "max_y", typed_value.bbox.max_y() ) );
  archive.finishNode();
}

// ----------------------------------------------------------------------------
template <> void save_value_visitor::operator()< klv_1108_metric_implementer >( )
const {
  auto const& typed_value = value.get< klv_1108_metric_implementer >();
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "organization", typed_value.organization ) );
  archive( make_nvp( "subgroup", typed_value.subgroup ) );
  archive.finishNode();
}

// ----------------------------------------------------------------------------
void
save_value( save_archive& archive, klv_value const& value,
            save_context& context )
{
  archive.startNode();

  auto const& trait = type_traits_of( value.type() );
  archive( make_nvp( "type", trait.name ) );
  if( value.empty() )
  {
    archive.finishNode();
    return;
  }
  if( value.length_hint() )
  {
    archive( make_nvp( "length", value.length_hint() ) );
  }
  kv::visit_types<
    save_value_visitor,
    int64_t,
    uint64_t,
    std::string,
    double,
    klv_local_set,
    klv_universal_set,
    klv_blob,
    klv_0601_icing_detected,
    klv_0601_frame_rate,
    klv_0601_control_command,
    klv_0601_operational_mode,
    klv_0601_platform_status,
    klv_0601_sensor_control_mode,
    klv_0601_sensor_fov_name,
    klv_1108_assessment_point,
    klv_1108_metric_period_pack,
    klv_1108_window_corners_pack,
    klv_1108_compression_type,
    klv_1108_compression_profile,
    klv_1108_metric_implementer
    >( { archive, value, context }, value.type() );

  archive.finishNode();
}

// ----------------------------------------------------------------------------
struct load_value_visitor
{
  template < class T,
             typename std::enable_if< !std::is_enum< T >::value,
                                      bool >::type = true >
  void
  operator()() const
  {
    T result;
    archive( make_nvp( "value", result ) );
    value = std::move( result );
  }

  template < class T,
             typename std::enable_if< std::is_enum< T >::value,
                                      bool >::type = true >
  void
  operator()() const
  {
    uint64_t result;
    archive( make_nvp( "value", result ) );
    value = std::move( static_cast< T >( result ) );
  }

  load_archive& archive;
  klv_value& value;
  load_context& context;
};

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_local_set >( ) const {
  klv_local_set result;
  size_t size;
  archive( make_nvp( "size", size ) );
  archive.setNextName( "value" );
  archive.startNode();
  for( auto i : kvr::iota( size ) )
  {
    (void) i;
    archive.startNode();
    archive.setNextName( "key" );
    auto const entry_key = load_lds_key( archive, context );
    archive.setNextName( "value" );
    auto const outer_context = context;
    if( context.lookup )
    {
      context.lookup = context.lookup->by_tag( entry_key ).subtag_lookup();
    }
    auto const entry_value = load_value( archive, context );
    context = outer_context;
    archive.finishNode();
    result.add( entry_key, entry_value );
  }
  archive.finishNode();
  value = std::move( result );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_universal_set >( ) const {
  klv_universal_set result;
  size_t size;
  archive( make_nvp( "size", size ) );
  archive.setNextName( "value" );
  archive.startNode();
  for( auto i : kvr::iota( size ) )
  {
    (void) i;
    archive.startNode();
    archive.setNextName( "key" );
    auto const entry_key = load_uds_key( archive, context );
    archive.setNextName( "value" );
    auto const outer_context = context;
    if( context.lookup )
    {
      context.lookup = context.lookup->by_uds_key( entry_key ).subtag_lookup();
    }
    auto const entry_value = load_value( archive, context );
    context = outer_context;
    archive.finishNode();
    result.add( entry_key, entry_value );
  }
  archive.finishNode();
  value = std::move( result );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_blob >( ) const {
  klv_blob result;
  archive( make_nvp( "value", *result ) );
  value = std::move( result );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_0601_frame_rate >( ) const
{
  klv_0601_frame_rate result;
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "numerator", result.numerator ) );
  archive( make_nvp( "denominator", result.denominator ) );
  archive.finishNode();
  value = std::move( result );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_0601_control_command >( )
const {
  klv_0601_control_command result;
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "id", result.id ) );
  archive( make_nvp( "string", result.string ) );
  archive( make_nvp( "timestamp", result.timestamp ) );
  archive.finishNode();
  value = std::move( result );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_1108_metric_period_pack >( )
const {
  klv_1108_metric_period_pack result;
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "timestamp", result.timestamp ) );
  archive( make_nvp( "offset", result.offset ) );
  archive.finishNode();
  value = std::move( result );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_1108_window_corners_pack >( )
const {
  archive.setNextName( "value" );
  archive.startNode();
  uint16_t min_x, min_y, max_x, max_y;
  archive( make_nvp( "min_x", min_x ) );
  archive( make_nvp( "min_y", min_y ) );
  archive( make_nvp( "max_x", max_x ) );
  archive( make_nvp( "max_y", max_y ) );
  archive.finishNode();
  value = std::move( klv_1108_window_corners_pack{ { min_x, min_y,
                       max_x, max_y } } );
}

// ----------------------------------------------------------------------------
template <> void load_value_visitor::operator()< klv_1108_metric_implementer >( )
const {
  klv_1108_metric_implementer result;
  archive.setNextName( "value" );
  archive.startNode();
  archive( make_nvp( "organization", result.organization ) );
  archive( make_nvp( "subgroup", result.subgroup ) );
  archive.finishNode();
  value = std::move( result );
}

// ----------------------------------------------------------------------------
klv_value
load_value( load_archive& archive, load_context& context )
{
  archive.startNode();

  std::string type_name;
  archive( make_nvp( "type", type_name ) );

  size_t length_hint = 0;
  try
  {
    archive( make_nvp( "length", length_hint ) );
  }
  catch ( std::runtime_error const& )
  {
  }

  auto const& trait = type_traits_of( type_name );
  if( trait.type == typeid( void ) )
  {
    // Unrecognized or empty type
    archive.finishNode();
    return {};
  }

  klv_value value;
  kv::visit_types<
    load_value_visitor,
    int64_t,
    uint64_t,
    std::string,
    double,
    klv_local_set,
    klv_universal_set,
    klv_blob,
    klv_0601_icing_detected,
    klv_0601_frame_rate,
    klv_0601_control_command,
    klv_0601_operational_mode,
    klv_0601_platform_status,
    klv_0601_sensor_control_mode,
    klv_0601_sensor_fov_name,
    klv_1108_assessment_point,
    klv_1108_metric_period_pack,
    klv_1108_window_corners_pack,
    klv_1108_compression_type,
    klv_1108_compression_profile,
    klv_1108_metric_implementer
    >( { archive, value, context }, trait.type );

  archive.finishNode();
  value.set_length_hint( length_hint );
  return value;
}

// ----------------------------------------------------------------------------
// Highest-level savers/loaders.

// ----------------------------------------------------------------------------
void
save_packet( save_archive& archive, klv_packet const& packet )
{
  save_context context;
  archive.startNode();

  archive( make_nvp( "type", std::string{ "packet" } ) );

  archive.setNextName( "key" );
  context.lookup = &klv_lookup_packet_traits();
  save_uds_key( archive, packet.key, context );

  archive.setNextName( "value" );
  context.lookup = context.lookup->by_uds_key( packet.key ).subtag_lookup();
  save_value( archive, packet.value, context );

  archive.finishNode();
}

// ----------------------------------------------------------------------------
klv_packet
load_packet( load_archive& archive )
{
  load_context context;

  archive.startNode();

  archive.setNextName( "key" );
  context.lookup = &klv_lookup_packet_traits();

  auto const key = load_uds_key( archive, context );

  archive.setNextName( "value" );
  context.lookup = context.lookup->by_uds_key( key ).subtag_lookup();

  auto const value = load_value( archive, context );

  archive.finishNode();

  return { key, value };
}

} // namespace

// ----------------------------------------------------------------------------
void
save( save_archive& archive,
      std::vector< klv_packet > const& packets )
{
  archive( make_nvp( "size", packets.size() ) );
  archive.setNextName( "data" );
  archive.startNode();
  archive.makeArray();
  for( auto const& packet : packets )
  {
    save_packet( archive, packet );
  }
  archive.finishNode();
}

// ----------------------------------------------------------------------------
void
load( load_archive& archive,
      std::vector< klv_packet >& packets )
{
  size_t size;
  archive( make_nvp( "size", size ) );
  packets.resize( size );
  archive.setNextName( "data" );
  archive.startNode();
  for( auto& packet : packets )
  {
    packet = load_packet( archive );
  }
  archive.finishNode();
}

} // namespace cereal
