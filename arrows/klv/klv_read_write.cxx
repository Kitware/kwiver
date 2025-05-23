// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the non-templated basic KLV read/write functions.

#include "klv_read_write.txx"

#include <stdexcept>
#include <algorithm>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
void
_check_range( vital::interval< double > const& interval )
{
  if( !std::isfinite( interval.lower() ) )
  {
    throw std::logic_error( "minimum must be finite" );
  }

  if( !std::isfinite( interval.upper() ) )
  {
    throw std::logic_error( "maximum must be finite" );
  }

  if( std::isinf( interval.span() ) )
  {
    VITAL_THROW( kv::metadata_type_overflow,
                 "span too large for double type" );
  }

  if( !( interval.lower() < interval.upper() ) )
  {
    throw std::logic_error( "minimum must be less than maximum" );
  }
}

} // namespace

// ----------------------------------------------------------------------------
size_t
_bits_to_decimal_digits( size_t bits )
{
  static auto const factor = std::log10( 2.0 );
  return static_cast< size_t >( std::ceil( bits * factor ) );
}

// ----------------------------------------------------------------------------
void
_check_range_precision(
  vital::interval< double > const& interval, double precision )
{
  _check_range( interval );

  if( !std::isfinite( precision ) )
  {
    throw std::logic_error( "precision must be finite" );
  }

  if( !( precision < interval.span() ) )
  {
    throw std::logic_error( "precision must be less than min-max span" );
  }
}

// ----------------------------------------------------------------------------
void
_check_range_length( vital::interval< double > const& interval, size_t length )
{
  _check_range( interval );

  if( !length )
  {
    throw std::logic_error( "length must not be zero" );
  }

  if( length > sizeof( uint64_t ) )
  {
    VITAL_THROW( kv::metadata_type_overflow,
                 "value too large for native type" );
  }
}

// ----------------------------------------------------------------------------
double
klv_read_float( klv_read_iter_t& data, size_t length )
{
  static_assert( std::numeric_limits< float  >::is_iec559 &&
                 std::numeric_limits< double >::is_iec559,
                 "non-IEEE-754 platform is not supported" );
  if( length == sizeof( float ) )
  {
    static_assert( sizeof( float ) == sizeof( uint32_t ),
                   "non-32-bit float not supported" );
    auto const int_value = klv_read_int< uint32_t >( data, length );
    float float_value;
    std::memcpy( &float_value, &int_value, sizeof( float_value ) );
    return float_value;
  }
  else if( length == sizeof( double ) )
  {
    static_assert( sizeof( double ) == sizeof( uint64_t ),
                   "non-64-bit double not supported" );
    auto const int_value = klv_read_int< uint64_t >( data, length );
    double float_value;
    std::memcpy( &float_value, &int_value, sizeof( float_value ) );
    return float_value;
  }
  VITAL_THROW( kwiver::vital::invalid_value,
               "length must be sizeof(float) or sizeof(double)" );
}

// ----------------------------------------------------------------------------
void
klv_write_float( double value, klv_write_iter_t& data, size_t length )
{
  static_assert( std::numeric_limits< float  >::is_iec559 &&
                 std::numeric_limits< double >::is_iec559,
                 "non-IEEE-754 platform is not supported" );
  if( length == sizeof( float ) )
  {
    static_assert( sizeof( float ) == sizeof( uint32_t ),
                   "non-32-bit float not supported" );
    auto const float_value = static_cast< float >( value );
    uint32_t int_value;
    std::memcpy( &int_value, &float_value, sizeof( int_value ) );
    klv_write_int( int_value, data, length );
  }
  else if( length == sizeof( double ) )
  {
    static_assert( sizeof( double ) == sizeof( uint64_t ),
                   "non-64-bit double not supported" );
    auto const float_value = static_cast< double >( value );
    uint64_t int_value;
    std::memcpy( &int_value, &float_value, sizeof( int_value ) );
    klv_write_int( int_value, data, length );
  }
  else
  {
    VITAL_THROW( kwiver::vital::invalid_value,
                 "length must be sizeof(float) or sizeof(double)" );
  }
}

// ----------------------------------------------------------------------------
std::string
klv_read_string( klv_read_iter_t& data, size_t length )
{
  auto const s = std::string( data, data + length );
  data += length;

  // "\0" means empty string
  // We avoid constructing a temp string object to compare against
  return ( s.size() == 1 && s[ 0 ] == '\0' ) ? "" : s;
}

// ----------------------------------------------------------------------------
void
klv_write_string(
  std::string const& value, klv_write_iter_t& data, size_t max_length )
{
  if( klv_string_length( value ) > max_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "string will overrun end of data buffer" );
  }

  // Empty string represented as "\0"
  if( value.empty() )
  {
    *data = '\0';
    ++data;
    return;
  }

  // "\0" is reserved for empty string
  // We avoid constructing a temp string object to compare against
  if( value.size() == 1 && value[ 0 ] == '\0' )
  {
    VITAL_THROW( kwiver::vital::metadata_type_overflow,
                 "the string \"\\0\" cannot be written to KLV stream" );
  }

  data = std::copy( value.cbegin(), value.cend(), data );
}

// ----------------------------------------------------------------------------
size_t
klv_flint_length( vital::interval< double > const& interval, double precision )
{
  _check_range_precision( interval, precision );

  // Based on IMAP, but without the one extra bit IMAP uses for NaN, Inf
  auto const length_bits = std::ceil( std::log2( interval.span() ) ) -
                           std::floor( std::log2( precision ) );
  return static_cast< size_t >( std::ceil( length_bits / 8.0 ) );
}

// ----------------------------------------------------------------------------
double
klv_flint_precision( vital::interval< double > const& interval, size_t length )
{
  _check_range_length( interval, length );

  // Based on IMAP, but without the one extra bit IMAP uses for NaN, Inf
  auto const length_bits = length * 8.0;
  return std::exp2( std::log2( interval.span() ) - length_bits );
}

// ----------------------------------------------------------------------------
size_t
klv_string_length( std::string const& value )
{
  // "\0" is reserved for empty string
  // We avoid constructing a temp string object to compare against
  if( value.size() == 1 && value[ 0 ] == '\0' )
  {
    VITAL_THROW( kwiver::vital::metadata_type_overflow,
                 "the string \"\\0\" cannot be written to KLV stream" );
  }

  return std::max< size_t >( value.size(), 1 );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
