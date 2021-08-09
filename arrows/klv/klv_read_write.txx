// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the templated basic KLV read/write functions.

#include "klv_read_write.h"

#include <vital/exceptions.h>

#include <limits>
#include <numeric>
#include <string>

#include <cmath>

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
