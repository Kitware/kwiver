// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV data formats.

#include "klv_data_format.h"

namespace kwiver {

namespace arrows {

namespace klv {

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

} // namespace klv

} // namespace arrows

} // namespace kwiver
