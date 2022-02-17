// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV data formats.

#include "klv_data_format.h"

#include "klv_0102.h"
#include "klv_0104.h"
#include "klv_0601.h"
#include "klv_0806_aoi_set.h"
#include "klv_0806_poi_set.h"
#include "klv_0806_user_defined_set.h"
#include "klv_0806.h"
#include "klv_0903_algorithm_set.h"
#include "klv_0903_location_pack.h"
#include "klv_0903_ontology_set.h"
#include "klv_0903_vchip_set.h"
#include "klv_0903_vmask_set.h"
#include "klv_0903_vobject_set.h"
#include "klv_0903_vtarget_pack.h"
#include "klv_0903_vtracker_set.h"
#include "klv_0903_vtrackitem_pack.h"
#include "klv_0903.h"
#include "klv_1010.h"
#include "klv_1108_metric_set.h"
#include "klv_1108.h"
#include "klv_1204.h"
#include "klv_blob.h"
#include "klv_packet.h"
#include "klv_series.hpp"

#include <iomanip>

#include <cfloat>

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

size_t
bits_to_decimal_digits( size_t bits )
{
  static auto const factor = std::log10( 2.0 );
  return static_cast< size_t >( std::ceil( bits * factor ) );
}

} // namespace

// ----------------------------------------------------------------------------
klv_data_format
::klv_data_format( size_t fixed_length ) : m_fixed_length{ fixed_length }
{}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::type_name() const
{
  return kwiver::vital::demangle( type().name() );
}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::to_string( klv_value const& value ) const
{
  std::stringstream ss;
  print( ss, value );
  return ss.str();
}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::length_description() const
{
  std::stringstream ss;
  if( m_fixed_length )
  {
    ss << "length " << m_fixed_length;
  }
  else
  {
    ss << "variable length";
  }
  return ss.str();
}

// ----------------------------------------------------------------------------
uint32_t
klv_data_format
::calculate_checksum( VITAL_UNUSED klv_read_iter_t data,
                      VITAL_UNUSED size_t length ) const
{
  return 0;
}

// ----------------------------------------------------------------------------
uint32_t
klv_data_format
::read_checksum( VITAL_UNUSED klv_read_iter_t data,
                 VITAL_UNUSED size_t length ) const
{
  return 0;
}

// ----------------------------------------------------------------------------
void
klv_data_format
::write_checksum( VITAL_UNUSED uint32_t checksum,
                  VITAL_UNUSED klv_write_iter_t& data,
                  VITAL_UNUSED size_t max_length ) const
{}

// ----------------------------------------------------------------------------
size_t
klv_data_format
::checksum_length() const
{
  return 0;
}

// ----------------------------------------------------------------------------
template < class T >
klv_data_format_< T >

::klv_data_format_( size_t fixed_length ) : klv_data_format{ fixed_length }
{}

// ----------------------------------------------------------------------------
template < class T >
klv_data_format_< T >

::~klv_data_format_() {}

// ----------------------------------------------------------------------------
template < class T >
klv_value
klv_data_format_< T >
::read( klv_read_iter_t& data, size_t length ) const
{
  if( !length )
  {
    // Zero length: null / unknown value
    return klv_value{};
  }
  else if( m_fixed_length && length != m_fixed_length )
  {
    // Invalid length
    LOG_WARN( kwiver::vital::get_logger( "klv" ),
              "fixed-length format `" << description() <<
              "` received wrong number of bytes ( " << length << " )" );
  }

  try
  {
    // Try to parse using this data format
    return read_typed( data, length );
  }
  catch ( std::exception const& e )
  {
    // Return blob if parsing failed
    LOG_ERROR( kwiver::vital::get_logger( "klv" ),
               "error occurred during parsing: " << e.what() );
    return klv_read_blob( data, length );
  }
}

// ----------------------------------------------------------------------------
template < class T >
void
klv_data_format_< T >
::write( klv_value const& value, klv_write_iter_t& data,
         size_t max_length ) const
{
  if( value.empty() )
  {
    // Null / unknown value: write nothing
    return;
  }
  else if( !value.valid() )
  {
    // Unparsed value: write raw bytes
    klv_write_blob( value.get< klv_blob >(), data, max_length );
  }
  else
  {
    // Ensure we have enough bytes
    auto const value_length = length_of( value );
    if( value_length > max_length )
    {
      VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                   "write will overflow buffer" );
    }

    // Write the value
    auto const begin = data;
    write_typed( value.get< T >(), data, value_length );

    // Ensure the number of bytes we wrote was how many we said we were going
    // to write
    auto const written_length =
      static_cast< size_t >( std::distance( begin, data ) );
    if( written_length != value_length )
    {
      std::stringstream ss;
      ss        << "format `" << description() << "`: "
                << "written length (" << written_length << ") and "
                << "calculated length (" << value_length <<  ") not equal";
      throw std::logic_error( ss.str() );
    }
  }
}

// ----------------------------------------------------------------------------
template < class T >
size_t
klv_data_format_< T >
::length_of( klv_value const& value ) const
{
  if( value.empty() )
  {
    return 0;
  }
  else if( !value.valid() )
  {
    return value.get< klv_blob >()->size();
  }
  else
  {
    return m_fixed_length
           ? m_fixed_length
           : length_of_typed( value.get< T >() );
  }
}

// ----------------------------------------------------------------------------
template < class T >
size_t
klv_data_format_< T >
::length_of_typed( VITAL_UNUSED T const& value ) const
{
  throw std::logic_error(
    std::string{} + "data format of type `" + type_name() +
    "` must either provide a fixed size or override length_of_typed()" );
}

// ----------------------------------------------------------------------------
template < class T >
std::type_info const&
klv_data_format_< T >
::type() const
{
  return typeid( T );
}

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
klv_data_format_< T >
::print( std::ostream& os, klv_value const& value ) const
{
  return !value.valid()
         ? ( os << value )
         : print_typed( os, value.get< T >() );
}

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
klv_data_format_< T >
::print_typed( std::ostream& os, T const& value ) const
{
  if( std::is_same< T, std::string >::value )
  {
    return os << '"' << value << '"';
  }
  else
  {
    return os << value;
  }
}

// ----------------------------------------------------------------------------
size_t
klv_data_format
::fixed_length() const
{
  return m_fixed_length;
}

// ----------------------------------------------------------------------------
void
klv_data_format
::set_fixed_length( size_t fixed_length )
{
  m_fixed_length = fixed_length;
}

// ----------------------------------------------------------------------------
klv_blob_format
::klv_blob_format( size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }
{}

// ----------------------------------------------------------------------------
klv_blob
klv_blob_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_blob( data, length );
}

// ----------------------------------------------------------------------------
void
klv_blob_format
::write_typed( klv_blob const& value,
               klv_write_iter_t& data, size_t length ) const
{
  return klv_write_blob( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_blob_format
::length_of_typed( klv_blob const& value ) const
{
  return klv_blob_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_blob_format
::description() const
{
  std::stringstream ss;
  ss << "raw bytes of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_uuid_format
::klv_uuid_format() : klv_data_format_< klv_uuid >{ klv_uuid_length() }
{}

// ----------------------------------------------------------------------------
std::string
klv_uuid_format
::description() const
{
  return "UUID of " + length_description();
}

// ----------------------------------------------------------------------------
klv_uuid
klv_uuid_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_uuid( data, length );
}

// ----------------------------------------------------------------------------
void
klv_uuid_format
::write_typed( klv_uuid const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_uuid( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_uuid_format
::length_of_typed( VITAL_UNUSED klv_uuid const& value ) const
{
  return klv_uuid_length();
}

// ----------------------------------------------------------------------------
klv_string_format
::klv_string_format( size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }
{}

// ----------------------------------------------------------------------------
std::string
klv_string_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_string( data, length );
}

// ----------------------------------------------------------------------------
void
klv_string_format
::write_typed( std::string const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_string( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_string_format
::length_of_typed( std::string const& value ) const
{
  return klv_string_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_string_format
::description() const
{
  std::stringstream ss;
  ss << "string of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_uint_format
::klv_uint_format( size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }
{}

// ----------------------------------------------------------------------------
uint64_t
klv_uint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_int< uint64_t >( data, length );
}

// ----------------------------------------------------------------------------
void
klv_uint_format
::write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_int( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_uint_format
::length_of_typed( uint64_t const& value ) const
{
  return klv_int_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_uint_format
::description() const
{
  std::stringstream ss;
  ss << "unsigned integer of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_sint_format
::klv_sint_format( size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }
{}

// ----------------------------------------------------------------------------
int64_t
klv_sint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_int< int64_t >( data, length );
}

// ----------------------------------------------------------------------------
void
klv_sint_format
::write_typed( int64_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_int( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_sint_format
::length_of_typed( int64_t const& value ) const
{
  return klv_int_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_sint_format
::description() const
{
  std::stringstream ss;
  ss << "signed integer of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
template < class T >
klv_enum_format< T >

::klv_enum_format( size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }
{}

// ----------------------------------------------------------------------------
template < class T >
klv_enum_format< T >

::~klv_enum_format() {}

// ----------------------------------------------------------------------------
template < class T >
std::string
klv_enum_format< T >
::description() const
{
  std::stringstream ss;
  ss    << this->type_name() << " enumeration of "
        << this->length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
template < class T >
typename klv_enum_format< T >::data_type

klv_enum_format< T >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return static_cast< data_type >(
    klv_read_int< uint64_t >( data, length ) );
}

// ----------------------------------------------------------------------------
template < class T >
void
klv_enum_format< T >
::write_typed( data_type const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_int( static_cast< uint64_t >( value ), data, length );
}

// ----------------------------------------------------------------------------
template < class T >
size_t
klv_enum_format< T >
::length_of_typed( data_type const& value ) const
{
  return klv_int_length( static_cast< uint64_t >( value ) );
}

// ----------------------------------------------------------------------------
klv_ber_format
::klv_ber_format() : klv_data_format_< data_type >{ 0 }
{}

// ----------------------------------------------------------------------------
uint64_t
klv_ber_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_ber< uint64_t >( data, length );
}

// ----------------------------------------------------------------------------
void
klv_ber_format
::write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_ber( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_ber_format
::length_of_typed( uint64_t const& value ) const
{
  return klv_ber_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_ber_format
::description() const
{
  std::stringstream ss;
  ss << "BER-encoded unsigned integer of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_ber_oid_format
::klv_ber_oid_format()
  : klv_data_format_< data_type >{ 0 }
{}

// ----------------------------------------------------------------------------
uint64_t
klv_ber_oid_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_ber_oid< uint64_t >( data, length );
}

// ----------------------------------------------------------------------------
void
klv_ber_oid_format
::write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_ber_oid< uint64_t >( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_ber_oid_format
::length_of_typed( uint64_t const& value ) const
{
  return klv_ber_oid_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_ber_oid_format
::description() const
{
  std::stringstream ss;
  ss << "BER-OID-encoded unsigned integer of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_float_format
::klv_float_format( size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }
{}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_float_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_float( data, length ), length };
}

// ----------------------------------------------------------------------------
void
klv_float_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_float( value.value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_float_format
::length_of_typed( klv_lengthy< double > const& value ) const
{
  return value.length;
}

// ----------------------------------------------------------------------------
std::ostream&
klv_float_format
::print_typed( std::ostream& os, klv_lengthy< double > const& value ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : value.length;
  auto const digits = ( length == 4 ) ? ( FLT_DIG + 1 ) : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_float_format
::description() const
{
  std::stringstream ss;
  ss << "IEEE-754 floating-point number of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_sflint_format
::klv_sflint_format( double minimum, double maximum, size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }, m_minimum{ minimum },
    m_maximum{ maximum }
{}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_sflint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_flint< int64_t >( m_minimum, m_maximum, data, length ),
           length };
}

// ----------------------------------------------------------------------------
void
klv_sflint_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_flint< int64_t >( value.value, m_minimum, m_maximum,
                              data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_sflint_format
::length_of_typed( klv_lengthy< double > const& value ) const
{
  return value.length;
}

// ----------------------------------------------------------------------------
std::ostream&
klv_sflint_format
::print_typed( std::ostream& os, klv_lengthy< double > const& value ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : value.length;
  auto const digits = length ? bits_to_decimal_digits( length * 8 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_sflint_format
::description() const
{
  std::stringstream ss;
  ss    << "signed integer of " << length_description() << " mapped to range "
        << "( " << m_minimum << ", " << m_maximum << " )";
  return ss.str();
}

// ----------------------------------------------------------------------------
double
klv_sflint_format
::minimum() const
{
  return m_minimum;
}

// ----------------------------------------------------------------------------
double
klv_sflint_format
::maximum() const
{
  return m_maximum;
}

// ----------------------------------------------------------------------------
klv_uflint_format
::klv_uflint_format( double minimum, double maximum, size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }, m_minimum{ minimum },
    m_maximum{ maximum }
{}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_uflint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_flint< uint64_t >( m_minimum, m_maximum, data, length ),
           length };
}

// ----------------------------------------------------------------------------
void
klv_uflint_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_flint< uint64_t >( value.value, m_minimum, m_maximum,
                               data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_uflint_format
::length_of_typed( klv_lengthy< double > const& value ) const
{
  return value.length;
}

// ----------------------------------------------------------------------------
std::ostream&
klv_uflint_format
::print_typed( std::ostream& os, klv_lengthy< double > const& value ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : value.length;
  auto const digits = length ? bits_to_decimal_digits( length * 8 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_uflint_format
::description() const
{
  std::stringstream ss;
  ss    << "unsigned integer of " << length_description() <<
        " mapped to range "
        << "( " << m_minimum << ", " << m_maximum << " )";
  return ss.str();
}

// ----------------------------------------------------------------------------
double
klv_uflint_format
::minimum() const
{
  return m_minimum;
}

// ----------------------------------------------------------------------------
double
klv_uflint_format
::maximum() const
{
  return m_maximum;
}

// ----------------------------------------------------------------------------
klv_imap_format
::klv_imap_format( double minimum, double maximum, size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }, m_minimum{ minimum },
    m_maximum{ maximum } {}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_imap_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_imap( m_minimum, m_maximum, data, length ), length };
}

// ----------------------------------------------------------------------------
void
klv_imap_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_imap( value.value, m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_imap_format
::length_of_typed( klv_lengthy< double > const& value ) const
{
  return value.length;
}

// ----------------------------------------------------------------------------
std::ostream&
klv_imap_format
::print_typed( std::ostream& os, klv_lengthy< double > const& value ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : value.length;
  auto const digits = length ? bits_to_decimal_digits( length * 8 - 1 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_imap_format
::description() const
{
  std::stringstream ss;
  ss    << "IMAP-encoded range ( " << m_minimum << ", " << m_maximum << " ), "
        << "of " << length_description();
  return ss.str();
}

// ----------------------------------------------------------------------------
double
klv_imap_format
::minimum() const
{
  return m_minimum;
}

// ----------------------------------------------------------------------------
double
klv_imap_format
::maximum() const
{
  return m_maximum;
}

#define KLV_INSTANTIATE( T ) \
  template class klv_data_format_< T >;

KLV_INSTANTIATE( double );
KLV_INSTANTIATE( int64_t );
KLV_INSTANTIATE( klv_0601_airbase_locations );
KLV_INSTANTIATE( klv_0601_control_command );
KLV_INSTANTIATE( klv_0601_country_codes );
KLV_INSTANTIATE( klv_0601_frame_rate );
KLV_INSTANTIATE( klv_0601_location_dlp );
KLV_INSTANTIATE( klv_0601_view_domain );
KLV_INSTANTIATE( klv_0601_weapons_general_status );
KLV_INSTANTIATE( klv_0601_weapons_store );
KLV_INSTANTIATE( klv_0806_user_defined_data );
KLV_INSTANTIATE( klv_0806_user_defined_data_type_id );
KLV_INSTANTIATE( klv_0903_algorithm_series );
KLV_INSTANTIATE( klv_0903_fpa_index );
KLV_INSTANTIATE( klv_0903_location_pack );
KLV_INSTANTIATE( klv_0903_location_series );
KLV_INSTANTIATE( klv_0903_ontology_series );
KLV_INSTANTIATE( klv_0903_pixel_run );
KLV_INSTANTIATE( klv_0903_pixel_run_series );
KLV_INSTANTIATE( klv_0903_vchip_series );
KLV_INSTANTIATE( klv_0903_velocity_pack );
KLV_INSTANTIATE( klv_0903_vobject_series );
KLV_INSTANTIATE( klv_0903_vtarget_pack );
KLV_INSTANTIATE( klv_0903_vtarget_series );
KLV_INSTANTIATE( klv_0903_vtrackitem_pack );
KLV_INSTANTIATE( klv_0903_vtrackitem_series );
KLV_INSTANTIATE( klv_1010_sdcc_flp );
KLV_INSTANTIATE( klv_1108_metric_implementer );
KLV_INSTANTIATE( klv_1108_metric_period_pack );
KLV_INSTANTIATE( klv_1108_window_corners_pack );
KLV_INSTANTIATE( klv_1204_miis_id );
KLV_INSTANTIATE( klv_blob );
KLV_INSTANTIATE( klv_local_set );
KLV_INSTANTIATE( klv_uint_series );
KLV_INSTANTIATE( klv_universal_set );
KLV_INSTANTIATE( klv_uuid );
KLV_INSTANTIATE( std::string );
KLV_INSTANTIATE( std::vector< klv_0601_payload_record > );
KLV_INSTANTIATE( std::vector< klv_0601_weapons_store > );
KLV_INSTANTIATE( std::vector< uint16_t > );
KLV_INSTANTIATE( uint64_t );

#define KLV_INSTANTIATE_ENUM( T ) \
  template class klv_enum_format< T >;

KLV_INSTANTIATE_ENUM( klv_0102_country_coding_method );
KLV_INSTANTIATE_ENUM( klv_0102_security_classification );
KLV_INSTANTIATE_ENUM( klv_0601_icing_detected );
KLV_INSTANTIATE_ENUM( klv_0601_operational_mode );
KLV_INSTANTIATE_ENUM( klv_0601_platform_status );
KLV_INSTANTIATE_ENUM( klv_0601_sensor_control_mode );
KLV_INSTANTIATE_ENUM( klv_0601_sensor_fov_name );
KLV_INSTANTIATE_ENUM( klv_0806_aoi_type );
KLV_INSTANTIATE_ENUM( klv_0806_user_defined_data_type );
KLV_INSTANTIATE_ENUM( klv_0903_detection_status );
KLV_INSTANTIATE_ENUM( klv_1108_assessment_point );
KLV_INSTANTIATE_ENUM( klv_1108_compression_profile );
KLV_INSTANTIATE_ENUM( klv_1108_compression_type );

} // namespace klv

} // namespace arrows

} // namespace kwiver
