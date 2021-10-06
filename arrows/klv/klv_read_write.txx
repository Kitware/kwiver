// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the templated basic KLV read/write functions.

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

// ---------------------------------------------------------------------------
// Macros for verifying type attributes
// Message-less static_assert not available in C++11

#define KLV_ASSERT_INT( T ) \
  static_assert( std::is_integral< T >::value, "must be an integer type" )

#define KLV_ASSERT_UINT( T )                                                \
  KLV_ASSERT_INT( T );                                                      \
  static_assert( std::is_unsigned< T >::value, "must be an unsigned type" )

#define KLV_ASSERT_SINT( T )                                           \
  KLV_ASSERT_INT( T );                                                 \
  static_assert( std::is_signed< T >::value, "must be a signed type" )

#define KLV_ASSERT_UINT8_ITERATOR( ITER )                              \
  static_assert(                                                       \
    std::is_same< typename std::decay< decltype( *ITER ) >::type,      \
                  uint8_t >::value, "iterator must point to uint8_t" )

// ---------------------------------------------------------------------------
// Return number of bits required to store the given signed or unsigned int.
template < class T >
KWIVER_ALGO_KLV_EXPORT
size_t
_int_bit_length( T value );

// ---------------------------------------------------------------------------
// Return whether left-shifting by given amount would overflow type T.
template < int shift_amount, typename T >
KWIVER_ALGO_KLV_EXPORT
bool
_left_shift_overflow( T value );

// ---------------------------------------------------------------------------
// Minimum integer representable using the given number of bytes
template < class T >
KWIVER_ALGO_KLV_EXPORT
T _int_min( size_t length );

// ---------------------------------------------------------------------------
// Maximum integer representable using the given number of bytes
template < class T >
KWIVER_ALGO_KLV_EXPORT
T _int_max( size_t length );

// ---------------------------------------------------------------------------
// Returns the IMAP representation of positive or negative infinity.
KWIVER_ALGO_KLV_EXPORT
uint64_t
_imap_infinity( bool sign_bit, size_t length );

// ---------------------------------------------------------------------------
// Returns the IMAP representation of positive or negative quiet NaN.
KWIVER_ALGO_KLV_EXPORT
uint64_t
_imap_quiet_nan( bool sign_bit, size_t length );

// ---------------------------------------------------------------------------
// Returns the IMAP representation of positive or negative signaling NaN.
KWIVER_ALGO_KLV_EXPORT
uint64_t
_imap_signal_nan( bool sign_bit, size_t length );

// ---------------------------------------------------------------------------
// Helper struct
struct KWIVER_ALGO_KLV_EXPORT _imap_terms
{
  double forward_scale;
  double backward_scale;
  double zero_offset;
};

// ---------------------------------------------------------------------------
// Calculates the derived terms needed for both IMAP reading and writing.
KWIVER_ALGO_KLV_EXPORT
_imap_terms
_calculate_imap_terms( double minimum, double maximum, size_t length );

// ---------------------------------------------------------------------------
// Throws invalid_value if arguments don't make sense
KWIVER_ALGO_KLV_EXPORT
void
_check_range_precision( double minimum, double maximum, double precision );

// ---------------------------------------------------------------------------
// Throws invalid_value if arguments don't make sense
KWIVER_ALGO_KLV_EXPORT
void
_check_range_length( double minimum, double maximum, size_t length );

// ---------------------------------------------------------------------------
template < class T, class Iterator >
T
klv_read_int( Iterator& data, size_t length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_INT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

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
  if( std::is_signed< T >::value && sizeof( T ) != length &&
      ( result_sign_bit & result ) )
  {
    result |= ~UnsignedT{ 0 } << ( 8 * length );
  }

  data += length;

  return result;
}

// ---------------------------------------------------------------------------
template < class T, class Iterator >
void
klv_write_int( T value, Iterator& data, size_t length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_INT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class T >
size_t
klv_int_length( T value )
{
  return ( _int_bit_length( value ) + 7 ) / 8;
}

// ---------------------------------------------------------------------------
template < class T, class Iterator >
T
klv_read_ber( Iterator& data, size_t max_length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_UINT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class T, class Iterator >
void
klv_write_ber( T value, Iterator& data, size_t max_length )
{
  KLV_ASSERT_UINT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

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
    *data = static_cast< uint8_t >( ( 0x7F & value_length - 1 ) | 0x80 );
    klv_write_int( value, ++data, value_length - 1 );
  }
}

// ---------------------------------------------------------------------------
template < class T >
size_t
klv_ber_length( T value )
{
  KLV_ASSERT_UINT( T );
  return ( value > 127 ) ? klv_int_length( value ) + 1 : 1;
}

// ---------------------------------------------------------------------------
template < class T, class Iterator >
T
klv_read_ber_oid( Iterator& data, size_t max_length )
{
  KLV_ASSERT_UINT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class T, class Iterator >
void
klv_write_ber_oid( T value, Iterator& data, size_t max_length )
{
  KLV_ASSERT_UINT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class T >
size_t
klv_ber_oid_length( T value )
{
  KLV_ASSERT_UINT( T );
  return ( _int_bit_length( value ) + 6 ) / 7;
}

// ---------------------------------------------------------------------------
template < class T, class Iterator >
double
klv_read_flint( double minimum, double maximum, Iterator& data, size_t length )
{
  KLV_ASSERT_INT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

  _check_range_length( minimum, maximum, length );

  // Before read to avoid moving iterator
  if( std::is_signed< T >::value && minimum != -maximum )
  {
    throw std::logic_error( "range must be symmetrical around zero" );
  }

  auto const int_value = klv_read_int< T >( data, length );
  auto const float_value = static_cast< double >( int_value );

  if( std::is_signed< T >::value )
  {
    // Special invalid / out-of-range value
    if( int_value == _int_min< T >( length ) )
    {
      return std::numeric_limits< double >::quiet_NaN();
    }

    auto const scale = maximum / _int_max< T >( length );
    return float_value * scale;
  }
  else
  {
    auto const scale = ( maximum - minimum ) / _int_max< T >( length );
    return float_value * scale + minimum;
  }
}

// ---------------------------------------------------------------------------
template < class T, class Iterator >
void
klv_write_flint( double value, double minimum, double maximum,
                 Iterator& data, size_t length )
{
  // Ensure types are compatible with our assumptions
  KLV_ASSERT_INT( T );
  KLV_ASSERT_UINT8_ITERATOR( data );

  _check_range_length( minimum, maximum, length );

  // Check before NaN
  if( std::is_signed< T >::value && minimum != -maximum )
  {
    throw std::logic_error( "range must be symmetrical around zero" );
  }

  // Check for NaN
  auto const min_int = _int_min< T >( length );
  auto const max_int = _int_max< T >( length );
  if( std::isnan( value ) )
  {
    klv_write_int( min_int, data, length );
    return;
  }

  // C++17: if constexpr
  if( std::is_signed< T >::value )
  {
    auto const scale = max_int / maximum;
    auto const float_value = value * scale;
    auto int_value = static_cast< T >( std::round( float_value ) );

    if( minimum > value || value > maximum )
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
    auto const scale = max_int / ( maximum - minimum );
    auto const float_value = ( value - minimum ) * scale;
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

// ---------------------------------------------------------------------------
template < class Iterator >
double
klv_read_float( Iterator& data, size_t length )
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

// ---------------------------------------------------------------------------
template < class Iterator >
void
klv_write_float( double value, Iterator& data, size_t length )
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

// ---------------------------------------------------------------------------
template < class Iterator >
double
klv_read_imap( double minimum, double maximum, Iterator& data, size_t length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class Iterator >
void
klv_write_imap( double value, double minimum, double maximum, Iterator& data,
                size_t length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class Iterator >
std::string
klv_read_string( Iterator& data, size_t length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

  auto const s = std::string( data, data + length );
  data += length;

  // "\0" means empty string
  // We avoid constructing a temp string object to compare against
  return ( s.size() == 1 && s[ 0 ] == '\0' ) ? "" : s;
}

// ---------------------------------------------------------------------------
template < class Iterator >
void
klv_write_string( std::string const& value, Iterator& data, size_t max_length )
{
  KLV_ASSERT_UINT8_ITERATOR( data );

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

// ---------------------------------------------------------------------------
template < class T >
size_t
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
  if( std::is_signed< T >::value )
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

// ---------------------------------------------------------------------------
template < class T >
T
_int_min( size_t length )
{
  KLV_ASSERT_INT( T );

  if( !length )
  {
    return 0;
  }

  if( std::is_signed< T >::value )
  {
    return -static_cast< T >( ( 0x80ull << ( ( length - 1 ) * 8 ) ) - 1 ) - 1;
  }
  else
  {
    return 0;
  }
}

// ---------------------------------------------------------------------------
template < class T >
T // TODO(C++14): make this constexpr
_int_max( size_t length )
{
  KLV_ASSERT_INT( T );

  if( !length )
  {
    return 0;
  }

  if( std::is_signed< T >::value )
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

// ---------------------------------------------------------------------------
template < int shift_amount, typename T >
bool
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
