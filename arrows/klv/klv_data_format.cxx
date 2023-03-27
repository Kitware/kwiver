// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV data formats.

#include "klv_data_format.h"

#include "klv_all.h"
#include "klv_blob.h"
#include "klv_checksum.h"
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
::klv_data_format( klv_length_constraints const& length_constraints )
  : m_length_constraints{ length_constraints }
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
klv_checksum_packet_format const*
klv_data_format
::checksum_format() const
{
  return nullptr;
}

// ----------------------------------------------------------------------------
klv_length_constraints const&
klv_data_format
::length_constraints() const
{
  return m_length_constraints;
}

// ----------------------------------------------------------------------------
void
klv_data_format
::set_length_constraints( klv_length_constraints const& length_constraints )
{
  m_length_constraints = length_constraints;
}

// ----------------------------------------------------------------------------
std::string
klv_data_format
::description() const
{
  auto const result = description_();
  if( m_length_constraints.is_free() )
  {
    return result;
  }
  return result + " (Length: " + m_length_constraints.description() + ")";
}

// ----------------------------------------------------------------------------
klv_blob_format
::klv_blob_format( klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }
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
::description_() const
{
  return "Raw Bytes";
}

// ----------------------------------------------------------------------------
klv_uuid_format
::klv_uuid_format() : klv_data_format_< klv_uuid >{ klv_uuid_length() }
{}

// ----------------------------------------------------------------------------
std::string
klv_uuid_format
::description_() const
{
  return "UUID";
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
::klv_string_format( klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }
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
::description_() const
{
  return "String";
}

// ----------------------------------------------------------------------------
klv_bool_format
::klv_bool_format() : klv_data_format_< bool >( 1 )
{}

// ----------------------------------------------------------------------------
std::string
klv_bool_format
::description_() const
{
  return "Boolean";
}

// ----------------------------------------------------------------------------
bool
klv_bool_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_int< uint8_t >( data, length );
}

// ----------------------------------------------------------------------------
void
klv_bool_format
::write_typed( bool const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_int< uint8_t >( value, data, length );
}

// ----------------------------------------------------------------------------
klv_uint_format
::klv_uint_format( klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }
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
::description_() const
{
  return "Unsigned Integer";
}

// ----------------------------------------------------------------------------
klv_sint_format
::klv_sint_format( klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }
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
::description_() const
{
  return "Signed Integer";
}

// ----------------------------------------------------------------------------
klv_ber_format
::klv_ber_format()
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
::description_() const
{
  return "Unsigned Integer (Encoding: BER)";
}

// ----------------------------------------------------------------------------
klv_ber_oid_format
::klv_ber_oid_format()
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
::description_() const
{
  return "Unsigned Integer (Encoding: BER-OID)";
}

// ----------------------------------------------------------------------------
klv_float_format
::klv_float_format( klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }
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
  auto const length = m_length_constraints.fixed_or( value.length );
  auto const digits = ( length == 4 ) ? ( FLT_DIG + 1 ) : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_float_format
::description_() const
{
  return "Float (Encoding: IEEE-754)";
}

// ----------------------------------------------------------------------------
klv_sflint_format
::klv_sflint_format(
  vital::interval< double > const& interval,
  klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }, m_interval{ interval }
{}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_sflint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_flint< int64_t >( m_interval, data, length ), length };
}

// ----------------------------------------------------------------------------
void
klv_sflint_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_flint< int64_t >( value.value, m_interval, data, length );
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
  auto const length = m_length_constraints.fixed_or( value.length );
  auto const digits = length ? bits_to_decimal_digits( length * 8 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_sflint_format
::description_() const
{
  std::stringstream ss;
  ss << "Float (Encoding: Signed Integer) (Range: " << m_interval << ")";
  return ss.str();
}

// ----------------------------------------------------------------------------
vital::interval< double >
klv_sflint_format
::interval() const
{
  return m_interval;
}

// ----------------------------------------------------------------------------
klv_uflint_format
::klv_uflint_format(
  vital::interval< double > const& interval,
  klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }, m_interval{ interval }
{}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_uflint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_flint< uint64_t >( m_interval, data, length ), length };
}

// ----------------------------------------------------------------------------
void
klv_uflint_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_flint< uint64_t >( value.value, m_interval, data, length );
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
  auto const length = m_length_constraints.fixed_or( value.length );
  auto const digits = length ? bits_to_decimal_digits( length * 8 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_uflint_format
::description_() const
{
  std::stringstream ss;
  ss << "Float (Encoding: Unsigned Integer) (Range: " << m_interval << ")";
  return ss.str();
}

// ----------------------------------------------------------------------------
vital::interval< double >
klv_uflint_format
::interval() const
{
  return m_interval;
}

// ----------------------------------------------------------------------------
klv_imap_format
::klv_imap_format( vital::interval< double > const& interval,
                   klv_length_constraints const& length_constraints )
  : klv_data_format_< data_type >{ length_constraints }, m_interval{ interval }
{}

// ----------------------------------------------------------------------------
klv_lengthy< double >
klv_imap_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return { klv_read_imap( m_interval, data, length ), length };
}

// ----------------------------------------------------------------------------
void
klv_imap_format
::write_typed( klv_lengthy< double > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_imap( value.value, m_interval, data, length );
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
  auto const length = m_length_constraints.fixed_or( value.length );
  auto const digits = length ? bits_to_decimal_digits( length * 8 - 1 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value.value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_imap_format
::description_() const
{
  std::stringstream ss;
  ss << "Float (Encoding: IMAP) (Range: " << m_interval << ")";
  return ss.str();
}

// ----------------------------------------------------------------------------
vital::interval< double >
klv_imap_format
::interval() const
{
  return m_interval;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
