// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV data formats.

#include "klv_data_format.h"

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
uint16_t
klv_data_format
::calculate_checksum( VITAL_UNUSED klv_read_iter_t data,
                      VITAL_UNUSED size_t length ) const
{
  return 0;
}

// ----------------------------------------------------------------------------
uint16_t
klv_data_format
::read_checksum( VITAL_UNUSED klv_read_iter_t data,
                 VITAL_UNUSED size_t length ) const
{
  return 0;
}

// ----------------------------------------------------------------------------
void
klv_data_format
::write_checksum( VITAL_UNUSED uint16_t checksum,
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
::length_of_typed( klv_blob const& value,
                   VITAL_UNUSED size_t length_hint ) const
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
::length_of_typed( std::string const& value,
                   VITAL_UNUSED size_t length_hint ) const
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
::length_of_typed( uint64_t const& value,
                   VITAL_UNUSED size_t length_hint ) const
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
::length_of_typed( int64_t const& value,
                   VITAL_UNUSED size_t length_hint ) const
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
::length_of_typed( uint64_t const& value,
                   VITAL_UNUSED size_t length_hint ) const
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
::length_of_typed( uint64_t const& value,
                   VITAL_UNUSED size_t length_hint ) const
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
double
klv_float_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_float( data, length );
}

// ----------------------------------------------------------------------------
void
klv_float_format
::write_typed( double const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_float( value, data, length );
}

// ----------------------------------------------------------------------------
std::ostream&
klv_float_format
::print_typed( std::ostream& os, double const& value,
               size_t length_hint ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : length_hint;
  auto const digits = ( length == 4 ) ? ( FLT_DIG + 1 ) : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value;

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
double
klv_sflint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_flint< int64_t >( m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
void
klv_sflint_format
::write_typed( double const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_flint< int64_t >( value, m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
std::ostream&
klv_sflint_format
::print_typed( std::ostream& os, double const& value,
               size_t length_hint ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : length_hint;
  auto const digits = length ? bits_to_decimal_digits( length * 8 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value;

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
klv_uflint_format
::klv_uflint_format( double minimum, double maximum, size_t fixed_length )
  : klv_data_format_< data_type >{ fixed_length }, m_minimum{ minimum },
    m_maximum{ maximum }
{}

// ----------------------------------------------------------------------------
double
klv_uflint_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_flint< uint64_t >( m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
void
klv_uflint_format
::write_typed( double const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_flint< uint64_t >( value, m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
std::ostream&
klv_uflint_format
::print_typed( std::ostream& os, double const& value,
               size_t length_hint ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : length_hint;
  auto const digits = length ? bits_to_decimal_digits( length * 8 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value;

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
std::string
klv_uflint_format
::description() const
{
  std::stringstream ss;
  ss  << "unsigned integer of " << length_description() << " mapped to range "
      << "( " << m_minimum << ", " << m_maximum << " )";
  return ss.str();
}

// ----------------------------------------------------------------------------
klv_imap_format
::klv_imap_format( double minimum, double maximum )
  : klv_data_format_< data_type >{ 0 }, m_minimum{ minimum },
    m_maximum{ maximum } {}

// ----------------------------------------------------------------------------
double
klv_imap_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  return klv_read_imap( m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
void
klv_imap_format
::write_typed( double const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_imap( value, m_minimum, m_maximum, data, length );
}

// ----------------------------------------------------------------------------
std::ostream&
klv_imap_format
::print_typed( std::ostream& os, double const& value,
               size_t length_hint ) const
{
  auto const flags = os.flags();

  // Print the number of digits corresponding to the precision of the format
  auto const length = m_fixed_length ? m_fixed_length : length_hint;
  auto const digits = length ? bits_to_decimal_digits( length * 8 - 1 )
                             : ( DBL_DIG + 1 );
  os << std::setprecision( digits ) << value;

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

} // namespace klv

} // namespace arrows

} // namespace kwiver
