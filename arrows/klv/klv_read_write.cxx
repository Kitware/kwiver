// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the non-templated basic KLV read/write functions.

#include "klv_read_write.txx"

#include <stdexcept>

namespace kv = kwiver::vital;
#include <algorithm>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
uint64_t
_imap_infinity( bool sign_bit, size_t length )
{
  auto const identifier = sign_bit ? 0xE8ull : 0xC8ull;
  return length ? identifier << ( ( length - 1 ) * 8 ) : 0;
}

// ----------------------------------------------------------------------------
uint64_t
_imap_quiet_nan( bool sign_bit, size_t length )
{
  auto const identifier = sign_bit ? 0xF0ull : 0xD0ull;
  return length ? identifier << ( ( length - 1 ) * 8 ) : 0;
}

// ----------------------------------------------------------------------------
uint64_t
_imap_signal_nan( bool sign_bit, size_t length )
{
  auto const identifier = sign_bit ? 0xF8ull : 0xD8ull;
  return length ? identifier << ( ( length - 1 ) * 8 ) : 0;
}

// ----------------------------------------------------------------------------
_imap_terms
_calculate_imap_terms( double minimum, double maximum, size_t length )
{
  // ST1201, Section 8.1.2
  auto const float_exponent = std::ceil( std::log2( maximum - minimum ) );
  auto const int_exponent = 8.0 * length - 1.0;

  _imap_terms result = {};
  result.forward_scale = std::exp2( int_exponent - float_exponent );
  result.backward_scale = std::exp2( float_exponent - int_exponent );
  result.zero_offset = ( minimum < 0 && maximum > 0 )
                       ? result.forward_scale * minimum -
                       std::floor( result.forward_scale * minimum )
                       : 0.0;
  return result;
}

// ----------------------------------------------------------------------------
size_t
klv_imap_length( double minimum, double maximum, double precision )
{
  // ST1201, Section 8.1.1
  _check_range_precision( minimum, maximum, precision );

  auto const length_bits = std::ceil( std::log2( maximum - minimum ) ) -
                           std::floor( std::log2( precision ) ) + 1.0;
  return static_cast< size_t >( std::ceil( length_bits / 8.0 ) );
}

// ----------------------------------------------------------------------------
double
klv_imap_precision( double minimum, double maximum, size_t length )
{
  _check_range_length( minimum, maximum, length );

  auto const length_bits = length * 8.0;
  return std::exp2( std::log2( maximum - minimum ) - length_bits + 1 );
}

namespace {

// ----------------------------------------------------------------------------
void
_check_range( double minimum, double maximum )
{
  if( !std::isfinite( minimum ) )
  {
    throw std::logic_error( "minimum must be finite" );
  }

  if( !std::isfinite( maximum ) )
  {
    throw std::logic_error( "maximum must be finite" );
  }

  if( std::isinf( maximum - minimum ) )
  {
    VITAL_THROW( kv::metadata_type_overflow,
                 "span too large for double type" );
  }

  if( !( minimum < maximum ) )
  {
    throw std::logic_error( "minimum must be less than maximum" );
  }
}

} // namespace

// ----------------------------------------------------------------------------
void
_check_range_precision( double minimum, double maximum, double precision )
{
  _check_range( minimum, maximum );

  if( !std::isfinite( precision ) )
  {
    throw std::logic_error( "precision must be finite" );
  }

  if( !( precision < maximum - minimum ) )
  {
    throw std::logic_error( "precision must be less than min-max span" );
  }
}

// ----------------------------------------------------------------------------
void
_check_range_length( double minimum, double maximum, size_t length )
{
  _check_range( minimum, maximum );

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
double
klv_read_imap(
  double minimum, double maximum, klv_read_iter_t& data, size_t length )
{
  // Section 8.1.2
  _check_range_length( minimum, maximum, length );

  auto const int_value = klv_read_int< uint64_t >( data, length );
  auto value = static_cast< double >( int_value );

  // Section 8.2.2
  // Left-shift required to shift a bit from the least significant place to the
  // most significant place
  auto const msb_shift = length * 8 - 1;

  // Most significant bit and any other bit set means this is a special value
  if( int_value & ( 1ull << msb_shift ) && int_value != ( 1ull << msb_shift ) )
  {
    // Third most significant bit = sign bit
    bool const sign_bit = int_value & 1ull << ( msb_shift - 2 );

    // Second, fourth, and fifth most significant bits = special value
    // identifiers
    constexpr uint64_t identifier_mask = 0xB; // 01011
    auto const identifier =
      ( int_value >> ( length * 8 - 5 ) ) & identifier_mask;

    switch( identifier )
    {
      case 0x9: // 1001
        value = std::numeric_limits< double >::infinity();
        break;
      case 0xA: // 1010
        value = std::numeric_limits< double >::quiet_NaN();
        break;
      case 0xB: // 1011
        value = std::numeric_limits< double >::signaling_NaN();
        break;
      default: // Reserved and user-defined values
        return std::numeric_limits< double >::quiet_NaN();
    }

    return sign_bit ? -value : value;
  }

  // Normal value
  auto const terms = _calculate_imap_terms( minimum, maximum, length );
  value = terms.backward_scale * ( value - terms.zero_offset ) + minimum;

  // Return exactly zero if applicable, overriding rounding errors. IMAP
  // specification considers this important
  auto const precision = klv_imap_precision( minimum, maximum, length );
  return ( std::abs( value ) < precision / 2.0 ) ? 0.0 : value;
}

// ----------------------------------------------------------------------------
void
klv_write_imap( double value, double minimum, double maximum,
                klv_write_iter_t& data, size_t length )
{
  // Section 8.1.2, 8.2.1
  _check_range_length( minimum, maximum, length );

  uint64_t int_value = 0;
  if( std::isnan( value ) )
  {
    // We can't robustly tell the difference between quiet and signaling NaNs,
    // so we just assume quiet. Therefore, we will never write signaling NaNs.
    int_value = _imap_quiet_nan( std::signbit( value ), length );
  }
  // Out-of-range values set to infinity
  else if( value < minimum )
  {
    int_value = _imap_infinity( true, length );
  }
  else if( value > maximum )
  {
    int_value = _imap_infinity( false, length );
  }
  // Normal values
  else
  {
    auto const terms = _calculate_imap_terms( minimum, maximum, length );
    int_value =
      static_cast< uint64_t >( terms.forward_scale * ( value - minimum ) +
                               terms.zero_offset );
  }

  klv_write_int( int_value, data, length );
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
klv_flint_length( double minimum, double maximum, double precision )
{
  _check_range_precision( minimum, maximum, precision );

  // Based on IMAP, but without the one extra bit IMAP uses for NaN, Inf
  auto const length_bits = std::ceil( std::log2( maximum - minimum ) ) -
                           std::floor( std::log2( precision ) );
  return static_cast< size_t >( std::ceil( length_bits / 8.0 ) );
}

// ----------------------------------------------------------------------------
double
klv_flint_precision( double minimum, double maximum, size_t length )
{
  _check_range_length( minimum, maximum, length );

  // Based on IMAP, but without the one extra bit IMAP uses for NaN, Inf
  auto const length_bits = length * 8.0;
  return std::exp2( std::log2( maximum - minimum ) - length_bits );
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
