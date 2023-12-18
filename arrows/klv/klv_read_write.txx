// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the templated basic KLV read/write functions.

#ifndef KWIVER_ARROWS_KLV_KLV_READ_WRITE_TXX_
#define KWIVER_ARROWS_KLV_KLV_READ_WRITE_TXX_

#include "klv_read_write.h"

#include <vital/exceptions.h>

#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>

#include <cmath>
#include <cstring>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
// Macros for verifying type attributes
// Message-less static_assert not available in C++11

#define KLV_ASSERT_INT( T ) \
  static_assert( std::is_integral_v< T >, "must be an integer type" )

#define KLV_ASSERT_UINT( T )                                                \
  KLV_ASSERT_INT( T );                                                      \
  static_assert( std::is_unsigned_v< T >, "must be an unsigned type" )

#define KLV_ASSERT_SINT( T )                                           \
  KLV_ASSERT_INT( T );                                                 \
  static_assert( std::is_signed_v< T >, "must be a signed type" )

// ----------------------------------------------------------------------------
// Return number of bits required to store the given signed or unsigned int.
template < class T >
constexpr size_t
_int_bit_length( T value );

// ----------------------------------------------------------------------------
// Return whether left-shifting by given amount would overflow type T.
template < int shift_amount, typename T >
constexpr bool
_left_shift_overflow( T value );

// ----------------------------------------------------------------------------
// Minimum integer representable using the given number of bytes
template < class T >
constexpr T _int_min( size_t length );

// ----------------------------------------------------------------------------
// Maximum integer representable using the given number of bytes
template < class T >
constexpr T _int_max( size_t length );

// ----------------------------------------------------------------------------
// Returns the IMAP representation of positive or negative infinity.
KWIVER_ALGO_KLV_EXPORT
uint64_t
_imap_infinity( bool sign_bit, size_t length );

// ----------------------------------------------------------------------------
// Returns the IMAP representation of positive or negative quiet NaN.
KWIVER_ALGO_KLV_EXPORT
uint64_t
_imap_quiet_nan( bool sign_bit, size_t length );

// ----------------------------------------------------------------------------
// Returns the IMAP representation of positive or negative signaling NaN.
KWIVER_ALGO_KLV_EXPORT
uint64_t
_imap_signal_nan( bool sign_bit, size_t length );

// ----------------------------------------------------------------------------
// Helper struct
struct KWIVER_ALGO_KLV_EXPORT _imap_terms
{
  double forward_scale;
  double backward_scale;
  double zero_offset;
};

// ----------------------------------------------------------------------------
// Calculates the derived terms needed for both IMAP reading and writing.
KWIVER_ALGO_KLV_EXPORT
_imap_terms
_calculate_imap_terms( vital::interval< double > const& interval, size_t length );

// ----------------------------------------------------------------------------
// Throws invalid_value if arguments don't make sense
KWIVER_ALGO_KLV_EXPORT
void
_check_range_precision( vital::interval< double > const& interval, double precision );

// ----------------------------------------------------------------------------
// Throws invalid_value if arguments don't make sense
KWIVER_ALGO_KLV_EXPORT
void
_check_range_length( vital::interval< double > const& interval, size_t length );

// ----------------------------------------------------------------------------
template < class T >
T
klv_read_int( klv_read_iter_t& data, size_t length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_INT( T );

  if( sizeof( T ) < length )
  {
    VITAL_THROW( kwiver::vital::metadata_type_overflow,
                 "integer will overflow given type" );
  }

  // Avoid integer underflow in later expressions
  if( !length )
  {
    return 0;
  }

  // We have to work with unsigned for proper behavior of << >> operators
  using UnsignedT = typename std::make_unsigned< T >::type;

  // Functor to insert each successive byte into the output value
  auto const accumulator = []( UnsignedT value, uint8_t byte ){
                             return ( value << 8 ) | byte;
                           };

  // Reduce span to final result
  auto result =
    std::accumulate( data, data + length, static_cast< UnsignedT >( 0 ),
                     accumulator );

  // Extend sign bit
  UnsignedT const result_sign_bit = 1ull << ( 8 * length - 1 );
  if constexpr( std::is_signed_v< T > )
  {
    if( sizeof( T ) != length && ( result_sign_bit & result ) )
    {
      result |= ~UnsignedT{ 0 } << ( 8 * length );
    }
  }

  data += length;

  return result;
}

// ----------------------------------------------------------------------------
template < class T >
void
klv_write_int( T value, klv_write_iter_t& data, size_t length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_INT( T );

  auto const value_length = klv_int_length( value );
  if( value_length > length )
  {
    VITAL_THROW( kwiver::vital::metadata_type_overflow,
                 "integer not representable using given length" );
  }

  using UnsignedT = typename std::make_unsigned< T >::type;

  auto const unsigned_value = static_cast< UnsignedT >( value );

  for( size_t i = 0; i < length; ++i, ++data )
  {
    auto const shift_amount = ( length - i - 1 ) * 8;
    *data =
      static_cast< uint8_t >( 0xFF & ( unsigned_value >> shift_amount ) );
  }
}

// ----------------------------------------------------------------------------
template < class T >
size_t
klv_int_length( T value )
{
  return ( _int_bit_length( value ) + 7 ) / 8;
}

// ----------------------------------------------------------------------------
template < class T >
T
klv_read_ber( klv_read_iter_t& data, size_t max_length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_UINT( T );

  if( !max_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "BER decoding overruns end of data buffer" );
  }

  // Short form - first bit is 0, remaining bits are the value itself
  if( !( 0x80 & *data ) )
  {
    auto const result = *data;
    ++data;
    return result;
  }

  // Long form - first bit is 1, remaining bits are the length of the
  // following
  size_t const total_length = ( 0x7F & *data ) + 1;

  if( total_length > max_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "BER decoding overruns end of data buffer" );
  }

  auto const rewind = data;
  try
  {
    return klv_read_int< T >( ++data, total_length - 1 );
  }
  catch ( const kwiver::vital::metadata_type_overflow& e )
  {
    data = rewind;

    throw e;
  }
}

// ----------------------------------------------------------------------------
template < class T >
void
klv_write_ber( T value, klv_write_iter_t& data, size_t max_length )
{
  KLV_ASSERT_UINT( T );

  auto const value_length = klv_ber_length( value );
  if( value_length > max_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "BER encoding overruns end of data buffer" );
  }

  if( value < 128 )
  {
    *data = static_cast< uint8_t >( value );
    ++data;
  }
  else
  {
    *data = static_cast< uint8_t >( ( 0x7F & ( value_length - 1 ) ) | 0x80 );
    klv_write_int( value, ++data, value_length - 1 );
  }
}

// ----------------------------------------------------------------------------
template < class T >
size_t
klv_ber_length( T value )
{
  KLV_ASSERT_UINT( T );
  return ( value > 127 ) ? klv_int_length( value ) + 1 : 1;
}

// ----------------------------------------------------------------------------
template < class T >
T
klv_read_ber_oid( klv_read_iter_t& data, size_t max_length )
{
  KLV_ASSERT_UINT( T );

  auto const rewind = data;

  // Highest bit set = not final byte
  T value = 0;
  for( uint8_t prev_byte = 0x80; prev_byte & 0x80; ++data )
  {
    if( !max_length )
    {
      data = rewind;
      VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                   "BER-OID decoding overruns end of data buffer" );
    }
    --max_length;

    if( _left_shift_overflow< 7, T >( value ) )
    {
      data = rewind;
      VITAL_THROW( kwiver::vital::metadata_type_overflow,
                   "BER-OID value will overflow given type" );
    }

    value <<= 7;
    value |= 0x7F & ( prev_byte = *data );
  }

  return value;
}

// ----------------------------------------------------------------------------
template < class T >
void
klv_write_ber_oid( T value, klv_write_iter_t& data, size_t max_length )
{
  KLV_ASSERT_UINT( T );

  auto value_length = klv_ber_oid_length( value );
  if( value_length > max_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "BER-OID encoding overruns end of data buffer" );
  }

  if( !value )
  {
    *data = 0;
    ++data;
    return;
  }

  while( value_length )
  {
    auto const shift_amount = static_cast< uint8_t >( --value_length * 7 );
    auto const top_bit = static_cast< uint8_t >( value_length ? 0x80 : 0x00 );
    auto const bottom_bits =
      static_cast< uint8_t >( ( value >> shift_amount ) & 0x7F );
    *data = top_bit | bottom_bits;
    ++data;
  }
}

// ----------------------------------------------------------------------------
template < class T >
size_t
klv_ber_oid_length( T value )
{
  KLV_ASSERT_UINT( T );
  return ( _int_bit_length( value ) + 6 ) / 7;
}

// ----------------------------------------------------------------------------
template < class T >
double
klv_read_flint(
  vital::interval< double > const& interval, klv_read_iter_t& data, size_t length )
{
  KLV_ASSERT_INT( T );

  _check_range_length( interval, length );

  // Before read to avoid moving iterator
  if constexpr( std::is_signed_v< T > )
  {
    if( interval.lower() != -interval.upper() )
    {
      throw std::logic_error( "range must be symmetrical around zero" );
    }
  }

  auto const int_value = klv_read_int< T >( data, length );
  auto const float_value = static_cast< double >( int_value );

  if constexpr( std::is_signed_v< T > )
  {
    // Special invalid / out-of-range value
    if( int_value == _int_min< T >( length ) )
    {
      return std::numeric_limits< double >::quiet_NaN();
    }

    auto const scale = interval.upper() / _int_max< T >( length );
    return float_value * scale;
  }
  else
  {
    auto const scale = interval.span() / _int_max< T >( length );
    return float_value * scale + interval.lower();
  }
}

// ----------------------------------------------------------------------------
template < class T >
void
klv_write_flint( double value, vital::interval< double > const& interval,
                 klv_write_iter_t& data, size_t length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_INT( T );

  _check_range_length( interval, length );

  // Check before NaN
  if constexpr( std::is_signed_v< T > )
  {
    if( interval.lower() != -interval.upper() )
    {
      throw std::logic_error( "range must be symmetrical around zero" );
    }
  }

  auto const min_int = _int_min< T >( length );
  auto const max_int = _int_max< T >( length );

  // Check for NaN
  if( std::isnan( value ) )
  {
    klv_write_int( min_int, data, length );
    return;
  }

  if constexpr( std::is_signed_v< T > )
  {
    auto const scale = max_int / interval.upper();
    auto const float_value = value * scale;
    auto int_value = static_cast< T >( std::round( float_value ) );

    if( !interval.contains( value, true, true ) )
    {
      // Special invalid / out-of-range value
      int_value = min_int;
    }
    else if( float_value >= max_int )
    {
      // Ensure floating-point rounding doesn't result in integer overflow
      int_value = max_int;
    }
    else if( float_value <= -max_int )
    {
      // Ensure floating-point rounding doesn't result in integer underflow
      int_value = -max_int;
    }
    klv_write_int( int_value, data, length );
  }
  else
  {
    auto const scale = max_int / interval.span();
    auto const float_value = ( value - interval.lower() ) * scale;
    auto int_value = static_cast< T >( std::round( float_value ) );

    // Clamp the value to max/min range. This tests float_value instead of
    // int_value because int_value may have over/underflowed due to rounding
    // errors
    if( float_value >= max_int )
    {
      int_value = max_int;
    }
    else if( float_value <= min_int )
    {
      int_value = min_int;
    }
    klv_write_int( int_value, data, length );
  }
}

// ----------------------------------------------------------------------------
template < class T >
constexpr size_t
_int_bit_length( T value )
{
  KLV_ASSERT_INT( T );

  if( value == 0 )
  {
    return 1;
  }

  // Transform signed number into equivalent-length unsigned number
  using UnsignedT = typename std::make_unsigned< T >::type;

  auto unsigned_value = static_cast< UnsignedT >( value );
  if constexpr( std::is_signed_v< T > )
  {
    unsigned_value = ( ( value < 0 ) ? ~unsigned_value : unsigned_value ) << 1;
  }

  // Find out number of bits needed
  size_t i = 0;
  for(; unsigned_value; ++i )
  {
    unsigned_value >>= 1;
  }
  return i;
}

// ----------------------------------------------------------------------------
template < class T >
constexpr T
_int_min( size_t length )
{
  KLV_ASSERT_INT( T );

  if( !length )
  {
    return 0;
  }

  if constexpr( std::is_signed_v< T > )
  {
    return -static_cast< T >( ( 0x80ull << ( ( length - 1 ) * 8 ) ) - 1 ) - 1;
  }
  else
  {
    return 0;
  }
}

// ----------------------------------------------------------------------------
template < class T >
constexpr T
_int_max( size_t length )
{
  KLV_ASSERT_INT( T );

  if( !length )
  {
    return 0;
  }

  if constexpr( std::is_signed_v< T > )
  {
    return ( 0x80ull << ( ( length - 1 ) * 8 ) ) - 1;
  }
  else
  {
    T value = 0;
    for( size_t i = 0; i < length; ++i )
    {
      value |= static_cast< T >( 0xFF ) << ( i * 8 );
    }
    return value;
  }
}

// ----------------------------------------------------------------------------
template < int shift_amount, typename T >
constexpr bool
_left_shift_overflow( T value )
{
  KLV_ASSERT_UINT( T );

  constexpr auto retained_bits = ( 8 * sizeof( T ) ) - shift_amount;
  constexpr auto mask = static_cast< T >( ~0ull << retained_bits );

  return ( value & mask ) != 0;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
