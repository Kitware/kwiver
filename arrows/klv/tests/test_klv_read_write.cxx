// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test basic KLV read / write functions

#include <arrows/klv/klv_read_write.txx>

#include <tests/test_gtest.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::arrows::klv;
namespace kv = kwiver::vital;

using vec_t = std::vector< uint8_t >;

auto const uint8_min  = std::numeric_limits< uint8_t  >::min();
auto const uint16_min = std::numeric_limits< uint16_t >::min();
auto const uint32_min = std::numeric_limits< uint32_t >::min();
auto const uint64_min = std::numeric_limits< uint64_t >::min();

auto const uint8_max  = std::numeric_limits< uint8_t  >::max();
auto const uint16_max = std::numeric_limits< uint16_t >::max();
auto const uint32_max = std::numeric_limits< uint32_t >::max();
auto const uint64_max = std::numeric_limits< uint64_t >::max();

auto const int8_min  = std::numeric_limits< int8_t  >::lowest();
auto const int16_min = std::numeric_limits< int16_t >::lowest();
auto const int32_min = std::numeric_limits< int32_t >::lowest();
auto const int64_min = std::numeric_limits< int64_t >::lowest();

auto const int8_max  = std::numeric_limits< int8_t  >::max();
auto const int16_max = std::numeric_limits< int16_t >::max();
auto const int32_max = std::numeric_limits< int32_t >::max();
auto const int64_max = std::numeric_limits< int64_t >::max();

auto const float_min  = std::numeric_limits< float >::lowest();
auto const float_max  = std::numeric_limits< float >::max();
auto const float_inf  = std::numeric_limits< float >::infinity();
auto const float_qnan = std::numeric_limits< float >::quiet_NaN();
auto const float_snan = std::numeric_limits< float >::signaling_NaN();

auto const double_min  = std::numeric_limits< double >::lowest();
auto const double_max  = std::numeric_limits< double >::max();
auto const double_inf  = std::numeric_limits< double >::infinity();
auto const double_qnan = std::numeric_limits< double >::quiet_NaN();
auto const double_snan = std::numeric_limits< double >::signaling_NaN();

// ----------------------------------------------------------------------------
template < class T >
void
test_read_int( vec_t const& data, size_t offset, size_t length, T value )
{
  auto it = data.cbegin() + offset;
  EXPECT_EQ( value, klv_read_int< T >( it, length ) );
  EXPECT_EQ( it, data.cbegin() + offset + length );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_read_int_throw( vec_t const& data, size_t offset, size_t length )
{
  auto it = data.cbegin() + offset;
  EXPECT_THROW( klv_read_int< T >( it, length ), E );
  EXPECT_EQ( it, data.cbegin() + offset );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_int_type_overflow( vec_t const& data, size_t offset, size_t length )
{
  test_read_int_throw< T, kv::metadata_type_overflow >( data, offset, length );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_int )
{
  // Each byte unique to ensure ordering is correct
  vec_t const data = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                       0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

  // Unsigned
  CALL_TEST( test_read_int< uint8_t >,  data, 0, 1, 0x00 );
  CALL_TEST( test_read_int< uint16_t >, data, 0, 2, 0x0011 );
  CALL_TEST( test_read_int< uint32_t >, data, 0, 4, 0x00112233 );
  CALL_TEST( test_read_int< uint64_t >, data, 0, 8, 0x0011223344556677 );

  // Signed, positive
  CALL_TEST( test_read_int< int8_t >,  data, 0, 1, 0x00 );
  CALL_TEST( test_read_int< int16_t >, data, 0, 2, 0x0011 );
  CALL_TEST( test_read_int< int32_t >, data, 0, 4, 0x00112233 );
  CALL_TEST( test_read_int< int64_t >, data, 0, 8, 0x0011223344556677 );

  // Signed, negative
  CALL_TEST( test_read_int< int8_t >,  data, 8, 1, 0x88 );
  CALL_TEST( test_read_int< int16_t >, data, 8, 2, 0x8899 );
  CALL_TEST( test_read_int< int32_t >, data, 8, 4, 0x8899aabb );
  CALL_TEST( test_read_int< int64_t >, data, 8, 8, 0x8899aabbccddeeff );

  // Unsigned - smaller than native size
  CALL_TEST( test_read_int< uint8_t  >, data, 0, 0, 0 );
  CALL_TEST( test_read_int< uint16_t >, data, 0, 1, 0x00 );
  CALL_TEST( test_read_int< uint32_t >, data, 0, 3, 0x001122 );
  CALL_TEST( test_read_int< uint64_t >, data, 0, 6, 0x001122334455 );

  // Signed, positive - smaller than native size
  CALL_TEST( test_read_int< int8_t  >, data, 0, 0, 0 );
  CALL_TEST( test_read_int< int16_t >, data, 0, 1, 0x00 );
  CALL_TEST( test_read_int< int32_t >, data, 0, 3, 0x001122 );
  CALL_TEST( test_read_int< int64_t >, data, 0, 6, 0x001122334455 );

  // Signed, negative - smaller than native size
  CALL_TEST( test_read_int< int8_t  >, data, 8, 0, 0 );
  CALL_TEST( test_read_int< int16_t >, data, 8, 1, 0xFF88 );
  CALL_TEST( test_read_int< int32_t >, data, 8, 3, 0xFF8899aa );
  CALL_TEST( test_read_int< int64_t >, data, 8, 6, 0xFFFF8899aabbccdd );

  // Unsigned - bigger than native size
  CALL_TEST( test_read_int_type_overflow< uint8_t  >, data, 0, 2 );
  CALL_TEST( test_read_int_type_overflow< uint16_t >, data, 0, 3 );
  CALL_TEST( test_read_int_type_overflow< uint32_t >, data, 0, 5 );
  CALL_TEST( test_read_int_type_overflow< uint32_t >, data, 0, 9 );

  // Signed - bigger than native size
  CALL_TEST( test_read_int_type_overflow< int8_t  >, data, 0, 2 );
  CALL_TEST( test_read_int_type_overflow< int16_t >, data, 0, 3 );
  CALL_TEST( test_read_int_type_overflow< int32_t >, data, 0, 5 );
  CALL_TEST( test_read_int_type_overflow< int32_t >, data, 0, 9 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_write_int( size_t length, T value )
{
  vec_t data( length, 0xba );
  auto it = data.begin();
  klv_write_int< T >( value, it, length );
  EXPECT_EQ( it, data.begin() + length );
  it = data.begin();
  EXPECT_EQ( value, klv_read_int< T >( it, length ) );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_write_int_throw( size_t length, T value )
{
  vec_t data( length, 0xba );
  auto it = data.begin();
  EXPECT_THROW( klv_write_int< T >( value, it, length ), E );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_write_int_type_overflow( size_t length, T value )
{
  test_write_int_throw< T, kv::metadata_type_overflow >( length, value );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_int )
{
  // Unsigned - arbitrary numbers
  CALL_TEST( test_write_int< uint8_t >,  1, 0x11 );
  CALL_TEST( test_write_int< uint16_t >, 2, 0x1122 );
  CALL_TEST( test_write_int< uint32_t >, 4, 0x11223344 );
  CALL_TEST( test_write_int< uint64_t >, 8, 0x1122334455667788 );

  // Signed, positive - arbitrary numbers
  CALL_TEST( test_write_int< int8_t >,   1, 0x11 );
  CALL_TEST( test_write_int< int16_t >,  2, 0x1122 );
  CALL_TEST( test_write_int< int32_t >,  4, 0x11223344 );
  CALL_TEST( test_write_int< int64_t >,  8, 0x1122334455667788 );

  // Signed, negative - arbitrary numbers
  CALL_TEST( test_write_int< int8_t >,   1, -0x11 );
  CALL_TEST( test_write_int< int16_t >,  2, -0x1122 );
  CALL_TEST( test_write_int< int32_t >,  4, -0x11223344 );
  CALL_TEST( test_write_int< int64_t >,  8, -0x1122334455667788 );

  // Lowest representable value
  CALL_TEST( test_write_int< uint8_t >,  1, uint8_min );
  CALL_TEST( test_write_int< uint16_t >, 2, uint16_min );
  CALL_TEST( test_write_int< uint32_t >, 4, uint32_min );
  CALL_TEST( test_write_int< uint64_t >, 8, uint64_min );
  CALL_TEST( test_write_int< int8_t >,   1, int8_min );
  CALL_TEST( test_write_int< int16_t >,  2, int16_min );
  CALL_TEST( test_write_int< int32_t >,  4, int32_min );
  CALL_TEST( test_write_int< int64_t >,  8, int64_min );

  // Highest representable value
  CALL_TEST( test_write_int< uint8_t >,  1, uint8_max );
  CALL_TEST( test_write_int< uint16_t >, 2, uint16_max );
  CALL_TEST( test_write_int< uint32_t >, 4, uint32_max );
  CALL_TEST( test_write_int< uint64_t >, 8, uint64_max );
  CALL_TEST( test_write_int< int8_t >,   1, int8_max );
  CALL_TEST( test_write_int< int16_t >,  2, int16_max );
  CALL_TEST( test_write_int< int32_t >,  4, int32_max );
  CALL_TEST( test_write_int< int64_t >,  8, int64_max );

  // Unsigned - smaller than native size
  CALL_TEST( test_write_int< uint16_t >, 1, 0x00 );
  CALL_TEST( test_write_int< uint32_t >, 3, 0x001122 );
  CALL_TEST( test_write_int< uint64_t >, 5, 0x0011223344 );

  // Signed, positive - smaller than native size
  CALL_TEST( test_write_int< int16_t >,  1, 0x00 );
  CALL_TEST( test_write_int< int32_t >,  3, 0x001122 );
  CALL_TEST( test_write_int< int64_t >,  5, 0x0011223344 );

  // Signed, negative - smaller than native size
  CALL_TEST( test_write_int< int16_t >,  1, -0x11 );
  CALL_TEST( test_write_int< int32_t >,  3, -0x112233 );
  CALL_TEST( test_write_int< int64_t >,  5, -0x1122334455 );

  // Unsigned - too few bytes allowed
  CALL_TEST( test_write_int_type_overflow< uint16_t >, 1, 0x0100 );
  CALL_TEST( test_write_int_type_overflow< uint32_t >, 3, 0x01000000 );
  CALL_TEST( test_write_int_type_overflow< uint64_t >, 5, 0x010000000000 );

  // Signed, positive - too few bytes allowed
  CALL_TEST( test_write_int_type_overflow< int16_t >, 1, 0x80 );
  CALL_TEST( test_write_int_type_overflow< int32_t >, 3, 0x800000 );
  CALL_TEST( test_write_int_type_overflow< int64_t >, 5, 0x8000000000 );

  // Signed, negative - too few bytes allowed
  CALL_TEST( test_write_int_type_overflow< int16_t >, 1, -0x81 );
  CALL_TEST( test_write_int_type_overflow< int32_t >, 3, -0x800001 );
  CALL_TEST( test_write_int_type_overflow< int64_t >, 5, -0x8000000001 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_int_length( T value, size_t expected_length )
{
  EXPECT_EQ( expected_length, klv_int_length( value ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, int_length )
{
  // Unsigned
  CALL_TEST( test_int_length< uint8_t >,  0x00ull,               1 );
  CALL_TEST( test_int_length< uint8_t >,  0xFFull,               1 );
  CALL_TEST( test_int_length< uint16_t >, 0x0100ull,             2 );
  CALL_TEST( test_int_length< uint16_t >, 0xFFFFull,             2 );
  CALL_TEST( test_int_length< uint32_t >, 0x010000ull,           3 );
  CALL_TEST( test_int_length< uint32_t >, 0xFFFFFFull,           3 );
  CALL_TEST( test_int_length< uint32_t >, 0x01000000ull,         4 );
  CALL_TEST( test_int_length< uint32_t >, 0xFFFFFFFFull,         4 );
  CALL_TEST( test_int_length< uint64_t >, 0x0100000000ull,       5 );
  CALL_TEST( test_int_length< uint64_t >, 0xFFFFFFFFFFull,       5 );
  CALL_TEST( test_int_length< uint64_t >, 0x010000000000ull,     6 );
  CALL_TEST( test_int_length< uint64_t >, 0xFFFFFFFFFFFFull,     6 );
  CALL_TEST( test_int_length< uint64_t >, 0x01000000000000ull,   7 );
  CALL_TEST( test_int_length< uint64_t >, 0xFFFFFFFFFFFFFFull,   7 );
  CALL_TEST( test_int_length< uint64_t >, 0x0100000000000000ull, 8 );
  CALL_TEST( test_int_length< uint64_t >, 0xFFFFFFFFFFFFFFFFull, 8 );

  // Signed - positive
  CALL_TEST( test_int_length< int8_t >,  0x00ll,               1 );
  CALL_TEST( test_int_length< int8_t >,  0x7Fll,               1 );
  CALL_TEST( test_int_length< int16_t >, 0x80ll,               2 );
  CALL_TEST( test_int_length< int16_t >, 0x7FFFll,             2 );
  CALL_TEST( test_int_length< int32_t >, 0x8000ll,             3 );
  CALL_TEST( test_int_length< int32_t >, 0x7FFFFFll,           3 );
  CALL_TEST( test_int_length< int32_t >, 0x800000ll,           4 );
  CALL_TEST( test_int_length< int32_t >, 0x7FFFFFFFll,         4 );
  CALL_TEST( test_int_length< int64_t >, 0x80000000ll,         5 );
  CALL_TEST( test_int_length< int64_t >, 0x7FFFFFFFFFll,       5 );
  CALL_TEST( test_int_length< int64_t >, 0x8000000000ll,       6 );
  CALL_TEST( test_int_length< int64_t >, 0x7FFFFFFFFFFFll,     6 );
  CALL_TEST( test_int_length< int64_t >, 0x800000000000ll,     7 );
  CALL_TEST( test_int_length< int64_t >, 0x7FFFFFFFFFFFFFll,   7 );
  CALL_TEST( test_int_length< int64_t >, 0x80000000000000ll,   8 );
  CALL_TEST( test_int_length< int64_t >, 0x7FFFFFFFFFFFFFFFll, 8 );

  // Signed - negative
  CALL_TEST( test_int_length< int16_t >, -0x80ll,               1 );
  CALL_TEST( test_int_length< int16_t >, -0x81ll,               2 );
  CALL_TEST( test_int_length< int32_t >, -0x8000ll,             2 );
  CALL_TEST( test_int_length< int32_t >, -0x8001ll,             3 );
  CALL_TEST( test_int_length< int32_t >, -0x800000ll,           3 );
  CALL_TEST( test_int_length< int32_t >, -0x800001ll,           4 );
  CALL_TEST( test_int_length< int64_t >, -0x80000000ll,         4 );
  CALL_TEST( test_int_length< int64_t >, -0x80000001ll,         5 );
  CALL_TEST( test_int_length< int64_t >, -0x8000000000ll,       5 );
  CALL_TEST( test_int_length< int64_t >, -0x8000000001ll,       6 );
  CALL_TEST( test_int_length< int64_t >, -0x800000000000ll,     6 );
  CALL_TEST( test_int_length< int64_t >, -0x800000000001ll,     7 );
  CALL_TEST( test_int_length< int64_t >, -0x80000000000000ll,   7 );
  CALL_TEST( test_int_length< int64_t >, -0x80000000000001ll,   8 );
  CALL_TEST( test_int_length< int64_t >, -0x8000000000000000ll, 8 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_ber( T value, vec_t const& data )
{
  auto it = data.begin();
  EXPECT_EQ( value, klv_read_ber< T >( it, data.size() ) );
  EXPECT_EQ( data.end(), it );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_read_ber_throw( size_t length, vec_t const& data )
{
  auto it = data.begin();
  EXPECT_THROW( klv_read_ber< T >( it, length ), E );
  EXPECT_EQ( data.begin(), it );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_ber_buffer_overflow( size_t length, vec_t const& data )
{
  test_read_ber_throw< T, kv::metadata_buffer_overflow >( length, data );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_ber_type_overflow( size_t length, vec_t const& data )
{
  test_read_ber_throw< T, kv::metadata_type_overflow >( length, data );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_ber )
{
  // Short form
  CALL_TEST( test_read_ber< uint8_t >, 0x00, { 0x00 } );
  CALL_TEST( test_read_ber< uint8_t >, 0x42, { 0x42 } );
  CALL_TEST( test_read_ber< uint8_t >, 0x7F, { 0x7F } );

  // Long form
  CALL_TEST( test_read_ber< uint8_t >,  0xFF,       { 0x81, 0xFF } );
  CALL_TEST( test_read_ber< uint16_t >, 0x102,      { 0x82, 0x01, 0x02 } );
  CALL_TEST( test_read_ber< uint32_t >, 0x010203,
             { 0x83, 0x01, 0x02, 0x03 } );
  CALL_TEST( test_read_ber< uint32_t >, 0xFF428012,
             { 0x84, 0xFF, 0x42, 0x80, 0x12 } );

  // Not enough buffer space given
  CALL_TEST( test_read_ber_buffer_overflow< uint32_t >, 1, { 0x81, 0xFF } );
  CALL_TEST( test_read_ber_buffer_overflow< uint32_t >, 2,
             { 0x82, 0xFF, 0x00 } );
  CALL_TEST( test_read_ber_buffer_overflow< uint32_t >, 0, { 0 } );

  // Specified type too small
  CALL_TEST( test_read_ber_type_overflow< uint8_t  >, 3,
             { 0x82, 0x01, 0x00 } );
  CALL_TEST( test_read_ber_type_overflow< uint16_t >, 4,
             { 0x83, 0x01, 0x00, 0x00 } );
  CALL_TEST( test_read_ber_type_overflow< uint32_t >, 6,
             { 0x85, 0x01, 0x00, 0x00, 0x00, 0x00 } );
  CALL_TEST( test_read_ber_type_overflow< uint64_t >, 10,
             { 0x89, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } );
}

// ----------------------------------------------------------------------------
void
test_write_ber( uint64_t value, size_t length )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_ber( value, it, data.size() );
  EXPECT_EQ( it, data.end() );

  // Avoid technically correct but bad-form encoding
  try
  {
    // Length should never be zero
    EXPECT_NE( 0x80, data.at( 0 ) );

    // Avoid leading zero bytes in value
    EXPECT_NE( 0x00, data.at( 1 ) );
  }
  catch ( std::exception const& e )
  {
    // data.at() threw - data doesn't have 0 or 1 index
  }

  it = data.begin();
  EXPECT_EQ( value, klv_read_ber< uint64_t >( it, data.size() ) );
}

// ----------------------------------------------------------------------------
template < class E >
void
test_write_ber_throw( uint64_t value, size_t length )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  EXPECT_THROW( klv_write_ber( value, it, data.size() ), E );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_ber )
{
  // Valid values
  CALL_TEST( test_write_ber, 0X00,              1 );
  CALL_TEST( test_write_ber, 0X42,              1 );
  CALL_TEST( test_write_ber, 0x7F,              1 );
  CALL_TEST( test_write_ber, 0x80,              2 );
  CALL_TEST( test_write_ber, uint8_max,         2 );
  CALL_TEST( test_write_ber, uint8_max  + 1ull, 3 );
  CALL_TEST( test_write_ber, uint16_max,        3 );
  CALL_TEST( test_write_ber, uint16_max + 1ull, 4 );
  CALL_TEST( test_write_ber, uint32_max,        5 );
  CALL_TEST( test_write_ber, uint32_max + 1ull, 6 );
  CALL_TEST( test_write_ber, uint64_max,        9 );

  // Not enough buffer space given
  auto test_buffer_overflow =
    test_write_ber_throw< kv::metadata_buffer_overflow >;
  CALL_TEST( test_buffer_overflow, 0x00,              0 );
  CALL_TEST( test_buffer_overflow, 0x42,              0 );
  CALL_TEST( test_buffer_overflow, 0x7F,              0 );
  CALL_TEST( test_buffer_overflow, 0x80,              1 );
  CALL_TEST( test_buffer_overflow, uint8_max,         1 );
  CALL_TEST( test_buffer_overflow, uint8_max + 1ull,  2 );
  CALL_TEST( test_buffer_overflow, uint16_max,        2 );
  CALL_TEST( test_buffer_overflow, uint16_max + 1ull, 3 );
  CALL_TEST( test_buffer_overflow, uint32_max,        4 );
  CALL_TEST( test_buffer_overflow, uint32_max + 1ull, 5 );
  CALL_TEST( test_buffer_overflow, uint64_max,        8 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_ber_length( T value, size_t expected_length )
{
  EXPECT_EQ( expected_length, klv_ber_length( value ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, ber_length )
{
  CALL_TEST( test_ber_length< uint8_t >,  0x00,               1 );
  CALL_TEST( test_ber_length< uint8_t >,  0x7F,               1 );
  CALL_TEST( test_ber_length< uint8_t >,  0x80,               2 );
  CALL_TEST( test_ber_length< uint8_t >,  0xFF,               2 );
  CALL_TEST( test_ber_length< uint16_t >, 0x0100,             3 );
  CALL_TEST( test_ber_length< uint16_t >, 0xFFFF,             3 );
  CALL_TEST( test_ber_length< uint32_t >, 0x010000,           4 );
  CALL_TEST( test_ber_length< uint32_t >, 0xFFFFFF,           4 );
  CALL_TEST( test_ber_length< uint32_t >, 0x01000000,         5 );
  CALL_TEST( test_ber_length< uint32_t >, 0xFFFFFFFF,         5 );
  CALL_TEST( test_ber_length< uint64_t >, 0x0100000000,       6 );
  CALL_TEST( test_ber_length< uint64_t >, 0xFFFFFFFFFF,       6 );
  CALL_TEST( test_ber_length< uint64_t >, 0x010000000000,     7 );
  CALL_TEST( test_ber_length< uint64_t >, 0xFFFFFFFFFFFF,     7 );
  CALL_TEST( test_ber_length< uint64_t >, 0x01000000000000,   8 );
  CALL_TEST( test_ber_length< uint64_t >, 0xFFFFFFFFFFFFFF,   8 );
  CALL_TEST( test_ber_length< uint64_t >, 0x0100000000000000, 9 );
  CALL_TEST( test_ber_length< uint64_t >, 0xFFFFFFFFFFFFFFFF, 9 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_ber_oid( T value, vec_t const& data )
{
  auto it = data.cbegin();
  EXPECT_EQ( value, klv_read_ber_oid< T >( it, data.size() ) );
  EXPECT_EQ( data.cend(), it );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_read_ber_oid_throw( vec_t const& data )
{
  auto it = data.cbegin();
  EXPECT_THROW( klv_read_ber_oid< T >( it, data.size() ), E );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_ber_oid_buffer_overflow( vec_t const& data )
{
  test_read_ber_oid_throw< T, kv::metadata_buffer_overflow >( data );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_ber_oid_type_overflow( vec_t const& data )
{
  test_read_ber_oid_throw< T, kv::metadata_type_overflow >( data );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_ber_oid )
{
  // Valid values
  CALL_TEST( test_read_ber_oid< uint8_t  >, 0x00,        { 0x00 } );
  CALL_TEST( test_read_ber_oid< uint8_t  >, 0x42,        { 0x42 } );
  CALL_TEST( test_read_ber_oid< uint8_t  >, 0x7F,        { 0x7F } );
  CALL_TEST( test_read_ber_oid< uint8_t  >, 0x80,        { 0x81, 0 } );
  CALL_TEST( test_read_ber_oid< uint8_t  >, 0xFF,        { 0x81, 0x7F } );
  CALL_TEST( test_read_ber_oid< uint16_t >, 0x3FFF,      { 0xFF, 0x7F } );
  CALL_TEST( test_read_ber_oid< uint16_t >, 0x4000,
             { 0x81, 0x80, 0x00 } );
  CALL_TEST( test_read_ber_oid< uint32_t >, 0x1FFFFF,
             { 0xFF, 0xFF, 0x7F } );
  CALL_TEST( test_read_ber_oid< uint32_t >, 0x200000,
             { 0x81, 0x80, 0x80, 0x00 } );
  CALL_TEST( test_read_ber_oid< uint64_t >, 0x7FFFFFFFF,
             { 0xFF, 0xFF, 0xFF, 0xFF, 0x7F } );
  CALL_TEST( test_read_ber_oid< uint64_t >, 0x800000000,
             { 0x81, 0x80, 0x80, 0x80, 0x80, 0x00 } );
  CALL_TEST( test_read_ber_oid< uint64_t >, uint64_max,
             { 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F } );

  // Truncated values
  CALL_TEST( test_read_ber_oid_buffer_overflow< uint64_t >, {} );
  CALL_TEST( test_read_ber_oid_buffer_overflow< uint64_t >, { 0x81 } );
  CALL_TEST( test_read_ber_oid_buffer_overflow< uint64_t >, { 0xFF, 0x81 } );

  // Values too large for native type
  CALL_TEST( test_read_ber_oid_type_overflow< uint8_t  >, { 0x82, 0X00 } );
  CALL_TEST( test_read_ber_oid_type_overflow< uint16_t >,
             { 0x84, 0x80, 0X00 } );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_write_ber_oid( T value, size_t length )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_ber_oid< T >( value, it, length );
  EXPECT_EQ( data.end(), it );
  // Avoid technically correct but bad-form encoding ( leading zero bytes )
  EXPECT_NE( data[ 0 ], 0x80 );
  it = data.begin();
  EXPECT_EQ( value, klv_read_ber_oid< T >( it, length ) );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_write_ber_oid_throw( T value, size_t length )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  EXPECT_THROW( klv_write_ber_oid( value, it, length ), E );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_ber_oid )
{
  // Valid values
  CALL_TEST( test_write_ber_oid< uint8_t >,  0x00,              1 );
  CALL_TEST( test_write_ber_oid< uint8_t >,  0x42,              1 );
  CALL_TEST( test_write_ber_oid< uint8_t >,  0x7F,              1 );
  CALL_TEST( test_write_ber_oid< uint8_t >,  0x80,              2 );
  CALL_TEST( test_write_ber_oid< uint8_t >,  uint8_max,         2 );
  CALL_TEST( test_write_ber_oid< uint16_t >, 0x3FFF,            2 );
  CALL_TEST( test_write_ber_oid< uint16_t >, 0x4000,            3 );
  CALL_TEST( test_write_ber_oid< uint32_t >, 0x0FFFFFFF,        4 );
  CALL_TEST( test_write_ber_oid< uint32_t >, 0x10000000,        5 );
  CALL_TEST( test_write_ber_oid< uint64_t >, 0x0FFFFFFFFFFFFFF, 8 );
  CALL_TEST( test_write_ber_oid< uint64_t >, 0x100000000000000, 9 );
  CALL_TEST( test_write_ber_oid< uint64_t >, uint64_max,        10 );

  // Not enough buffer space given
  auto test_buffer_overflow =
    test_write_ber_oid_throw< uint64_t, kv::metadata_buffer_overflow >;
  CALL_TEST( test_buffer_overflow, 0,              0 );
  CALL_TEST( test_buffer_overflow, ( 1ull << 7 ),  1 );
  CALL_TEST( test_buffer_overflow, ( 1ull << 14 ), 2 );
  CALL_TEST( test_buffer_overflow, ( 1ull << 21 ), 3 );
  CALL_TEST( test_buffer_overflow, ( 1ull << 28 ), 4 );
  CALL_TEST( test_buffer_overflow, ( 1ull << 56 ), 8 );
  CALL_TEST( test_buffer_overflow, ( 1ull << 63 ), 9 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_ber_oid_length( T value, size_t expected_length )
{
  EXPECT_EQ( expected_length, klv_ber_oid_length( value ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, ber_oid_length )
{
  CALL_TEST( test_ber_oid_length< uint8_t >,  0,                  1 );
  CALL_TEST( test_ber_oid_length< uint8_t >,  ( 1ull << 7 ) - 1,  1 );
  CALL_TEST( test_ber_oid_length< uint8_t >,  ( 1ull << 7 ),      2 );
  CALL_TEST( test_ber_oid_length< uint16_t >, ( 1ull << 14 ) - 1, 2 );
  CALL_TEST( test_ber_oid_length< uint16_t >, ( 1ull << 14 ),     3 );
  CALL_TEST( test_ber_oid_length< uint32_t >, ( 1ull << 21 ) - 1, 3 );
  CALL_TEST( test_ber_oid_length< uint32_t >, ( 1ull << 21 ),     4 );
  CALL_TEST( test_ber_oid_length< uint32_t >, ( 1ull << 28 ) - 1, 4 );
  CALL_TEST( test_ber_oid_length< uint32_t >, ( 1ull << 28 ),     5 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 35 ) - 1, 5 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 35 ),     6 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 42 ) - 1, 6 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 42 ),     7 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 49 ) - 1, 7 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 49 ),     8 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 56 ) - 1, 8 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 56 ),     9 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 63 ) - 1, 9 );
  CALL_TEST( test_ber_oid_length< uint64_t >, ( 1ull << 63 ),     10 );
  CALL_TEST( test_ber_oid_length< uint64_t >, uint64_max,         10 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_flint( T int_value, size_t length, double double_value,
                 double minimum, double maximum )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_int< T >( int_value, it, length );
  it = data.begin();

  auto const result = klv_read_flint< T >( minimum, maximum, it, length );
  if( std::isnan( double_value ) )
  {
    // GTest won't print result value on fail, so do it manually
    EXPECT_TRUE( std::isnan( result ) ) << "result: " << result;
    EXPECT_EQ( std::signbit( double_value ), std::signbit( result ) );
  }
  else
  {
    EXPECT_DOUBLE_EQ( double_value, result );
  }
  EXPECT_EQ( data.end(), it );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_read_flint_throw( T int_value, size_t length, double minimum,
                       double maximum )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_int< T >( int_value, it, length );
  it = data.begin();
  EXPECT_THROW( klv_read_flint< T >( minimum, maximum, it, length ), E );
  EXPECT_EQ( data.begin(), it );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_read_flint_logic_error( T int_value, size_t length, double minimum,
                             double maximum )
{
  test_read_flint_throw< T, std::logic_error >( int_value, length, minimum,
                                                maximum );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_flint )
{
  // Decimals provided by Wolfram Alpha's super-precision arithmetic
  // Unsigned values
  CALL_TEST( test_read_flint< uint8_t >,  0x00,               1,
             -1.0,                   -1.0,    1.0 );
  CALL_TEST( test_read_flint< uint8_t >,  0xFF,               1,
             1.0,                   -1.0,    1.0 );
  CALL_TEST( test_read_flint< uint8_t >,  0xA3,               1,
             0.2784313725490196,    -1.0,    1.0 );
  CALL_TEST( test_read_flint< uint8_t >,  0x29,               1,
             -0.6784313725490196,    -1.0,    1.0 );
  CALL_TEST( test_read_flint< uint16_t >, 0xA196,             2,
             1.4716258487830925e10, -2.0e10, 3.5e10 );
  CALL_TEST( test_read_flint< uint32_t >, 0x000000,           3,
             -2.0e10,                -2.0e10, 3.5e10 );
  CALL_TEST( test_read_flint< uint32_t >, 0xFFE345,           3,
             3.4975891707890730e10, -2.0e10, 3.5e10 );
  CALL_TEST( test_read_flint< uint32_t >, 0xA3425468,         4,
             4.8060157894170880e10,  2.0e10, 6.4e10 );
  CALL_TEST( test_read_flint< uint64_t >, 0x0000000001,       5,
             -9.9999999999536158e-6, -1.0e-5, 4.1e-5 );
  CALL_TEST( test_read_flint< uint64_t >, 0xFF001234FFFFFFFF, 8,
             2.7773500442970544e99, -3.0e99, 2.8e99 );
  CALL_TEST( test_read_flint< uint64_t >, 0xFFFFFFFFFFFFFFFF, 8,
             2.0,                    1.0,    2.0 );

  // Signed values
  CALL_TEST( test_read_flint< int8_t >,   0x00ll,               1,
             0.0,                -1.0, 1.0 );
  CALL_TEST( test_read_flint< int8_t >,  -0x7Fll,               1,
             -1.0,                -1.0, 1.0 );
  CALL_TEST( test_read_flint< int8_t >,   0x7Fll,               1,
             1.0,                -1.0, 1.0 );
  CALL_TEST( test_read_flint< int32_t >, -0x7FFFFFll,           3,
             -1.0,                -1.0, 1.0 );
  CALL_TEST( test_read_flint< int32_t >,  0x321CBAll,           3,
             0.3915017117859973, -1.0, 1.0 );
  CALL_TEST( test_read_flint< int32_t >, -0x123ABCll,           3,
             -0.1424174478551683, -1.0, 1.0 );
  CALL_TEST( test_read_flint< int32_t >,  0x7FFFFFll,           3,
             1.0,                -1.0, 1.0 );
  CALL_TEST( test_read_flint< int64_t >,  0x00ll,               5,
             0.0,                -1.0, 1.0 );
  CALL_TEST( test_read_flint< int64_t >, -0x7FFFFFFFFFFFFFFFll, 8,
             -2.0,                -2.0, 2.0 );
  CALL_TEST( test_read_flint< int64_t >,  0x7FFFFFFFFFFFFFFFll, 8,
             2.0,                -2.0, 2.0 );

  // Lowest representable value = NaN
  CALL_TEST( test_read_flint< int8_t >,  int8_min,        1,
             double_qnan, -1.0, 1.0 );
  CALL_TEST( test_read_flint< int64_t >, -0x8000000000ll, 5,
             double_qnan, -1.0, 1.0 );
  CALL_TEST( test_read_flint< int64_t >, int64_min,       8,
             double_qnan, -1.0, 1.0 );

  // Invalid values
  auto test_uint_invalid_value = test_read_flint_logic_error< uint64_t >;
  auto test_sint_invalid_value = test_read_flint_logic_error< int64_t >;
  CALL_TEST( test_uint_invalid_value, 0, 1, 0.0, 0.0 );
  CALL_TEST( test_uint_invalid_value, 0, 1, 0.0, -1.0 );
  CALL_TEST( test_uint_invalid_value, 0, 1, -double_inf, 0.0 );
  CALL_TEST( test_uint_invalid_value, 0, 1, 0.0, double_inf );
  CALL_TEST( test_uint_invalid_value, 0, 1, double_qnan, 0.0 );
  CALL_TEST( test_uint_invalid_value, 0, 1, 0.0, double_qnan );
  CALL_TEST( test_sint_invalid_value, 0, 1, -0.0, 0.0 );
  CALL_TEST( test_sint_invalid_value, 0, 1, -0.9, 1.0 );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_write_flint( size_t length, double value, double expected_value,
                  double minimum, double maximum )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_flint< T >( value, minimum, maximum, it, length );
  EXPECT_EQ( data.end(), it );
  it = data.begin();

  auto const result = klv_read_flint< T >( minimum, maximum, it, length );
  if( std::isnan( expected_value ) )
  {
    // GTest won't print result value on fail, so do it manually
    EXPECT_TRUE( std::isnan( result ) ) << "result: " << result;
    EXPECT_EQ( std::signbit( expected_value ), std::signbit( result ) );
  }
  else
  {
    auto const precision = klv_flint_precision( minimum, maximum, length );
    if( expected_value / precision < 1.0e15 )
    {
      EXPECT_NEAR( expected_value, result, precision );
    }
    else
    {
      EXPECT_DOUBLE_EQ( expected_value, result );
    }
  }
  EXPECT_EQ( data.end(), it );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_write_flint( size_t length, double value, double minimum, double maximum )
{
  test_write_flint< T >( length, value, value, minimum, maximum );
}

// ----------------------------------------------------------------------------
template < class T, class E >
void
test_write_flint_throw( size_t length, double value, double minimum,
                        double maximum )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  EXPECT_THROW( klv_write_flint< T >( value, minimum, maximum, it, length ),
                E );
  EXPECT_EQ( data.begin(), it );
}

// ----------------------------------------------------------------------------
template < class T >
void
test_write_flint_logic_error( size_t length, double value, double minimum,
                              double maximum )
{
  test_write_flint_throw< T, std::logic_error >( length, value, minimum,
                                                 maximum );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_flint )
{
  // Unsigned values
  CALL_TEST( test_write_flint< uint8_t  >, 1,  double_qnan,   0.1,  0.1, 1.0 );
  CALL_TEST( test_write_flint< uint8_t  >, 1,  0.1,                 0.1, 1.0 );
  CALL_TEST( test_write_flint< uint8_t  >, 1,  0.42,                0.1, 1.0 );
  CALL_TEST( test_write_flint< uint8_t  >, 1,  0.65,                0.1, 1.0 );
  CALL_TEST( test_write_flint< uint8_t  >, 1,  1.0,                 0.1, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 5, -7.01,         -7.0, -7.0, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 5,  0.0,                -7.0, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 5,  double_qnan,  -7.0, -7.0, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 6,  0.42,               -7.0, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 7,  0.65,               -7.0, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 8,  1.0,                -7.0, 1.0 );
  CALL_TEST( test_write_flint< uint64_t >, 8,  1.01,          1.0, -7.0, 1.0 );

  // Signed values
  CALL_TEST( test_write_flint< int8_t  >, 1, -1.01, double_qnan, -1.0, 1.0 );
  CALL_TEST( test_write_flint< int8_t  >, 1, -1.0,               -1.0, 1.0 );
  CALL_TEST( test_write_flint< int8_t  >, 1, -0.22,              -1.0, 1.0 );
  CALL_TEST( test_write_flint< int8_t  >, 1,  0.0,               -1.0, 1.0 );
  CALL_TEST( test_write_flint< int8_t  >, 1,  0.22,              -1.0, 1.0 );
  CALL_TEST( test_write_flint< int8_t  >, 1,  1.0,               -1.0, 1.0 );
  CALL_TEST( test_write_flint< int8_t  >, 1,  1.01, double_qnan, -1.0, 1.0 );
  CALL_TEST( test_write_flint< int64_t >, 8, -7.01, double_qnan, -7.0, 7.0 );
  CALL_TEST( test_write_flint< int64_t >, 7, -7.0,               -7.0, 7.0 );
  CALL_TEST( test_write_flint< int64_t >, 6, -0.22,              -7.0, 7.0 );
  CALL_TEST( test_write_flint< int64_t >, 5,  0.0,               -7.0, 7.0 );
  CALL_TEST( test_write_flint< int64_t >, 6,  0.22,              -7.0, 7.0 );
  CALL_TEST( test_write_flint< int64_t >, 7,  7.0,               -7.0, 7.0 );
  CALL_TEST( test_write_flint< int64_t >, 8,  7.01, double_qnan, -7.0, 7.0 );

  // Invalid values
  auto test_uint_invalid_value = test_write_flint_logic_error< uint64_t >;
  auto test_sint_invalid_value = test_write_flint_logic_error< int64_t >;
  CALL_TEST( test_uint_invalid_value, 1, 0.0,  0.0,         0.0 );
  CALL_TEST( test_uint_invalid_value, 1, 0.0,  0.0,        -1.0 );
  CALL_TEST( test_uint_invalid_value, 1, 0.0, -double_inf,  0.0 );
  CALL_TEST( test_uint_invalid_value, 1, 0.0,  0.0,         double_inf );
  CALL_TEST( test_uint_invalid_value, 1, 0.0,  double_qnan, 0.0 );
  CALL_TEST( test_uint_invalid_value, 1, 0.0,  0.0,         double_qnan );
  CALL_TEST( test_sint_invalid_value, 1, 0.0, -0.0,         0.0 );
  CALL_TEST( test_sint_invalid_value, 1, 0.0, -0.9,         1.0 );
}

// ----------------------------------------------------------------------------
void
test_read_float( double value, vec_t const& bytes )
{
  auto it = bytes.cbegin();
  auto const result = klv_read_float( it, bytes.size() );
  if( std::isnan( value ) )
  {
    // GTest won't print result value on fail, so do it manually
    EXPECT_TRUE( std::isnan( result ) ) << "result: " << result;
    EXPECT_EQ( std::signbit( value ), std::signbit( result ) );
  }
  else
  {
    EXPECT_DOUBLE_EQ( value, result );
  }
  EXPECT_EQ( bytes.cend(), it );
}

// ----------------------------------------------------------------------------
template < class E >
void
test_read_float_throw( vec_t const& bytes )
{
  auto it = bytes.cbegin();
  EXPECT_THROW( klv_read_float( it, bytes.size() ), E );
}

// ----------------------------------------------------------------------------
void
test_read_float_invalid_value( vec_t const& bytes )
{
  test_read_float_throw< kv::invalid_value >( bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_float )
{
  // Bit patterns confirmed on Wolfram-Alpha
  // Normal values - float
  CALL_TEST( test_read_float,  0.0f,   { 0x00, 0x00, 0x00, 0x00 } );
  CALL_TEST( test_read_float, -0.1f,   { 0xBD, 0xCC, 0xCC, 0xCD } );
  CALL_TEST( test_read_float, +0.1f,   { 0x3D, 0xCC, 0xCC, 0xCD } );
  CALL_TEST( test_read_float, -1e23f,  { 0xE5, 0xA9, 0x68, 0x16 } );
  CALL_TEST( test_read_float, +1e-23f, { 0x19, 0x41, 0x6D, 0x9A } );

  // Special values - float
  CALL_TEST( test_read_float,  float_min,  { 0xFF, 0x7F, 0xFF, 0xFF } );
  CALL_TEST( test_read_float,  float_max,  { 0x7F, 0x7F, 0xFF, 0xFF } );
  CALL_TEST( test_read_float, -float_inf,  { 0xFF, 0x80, 0x00, 0x00 } );
  CALL_TEST( test_read_float, +float_inf,  { 0x7F, 0x80, 0x00, 0x00 } );
  CALL_TEST( test_read_float, -float_qnan, { 0xFF, 0x80, 0x00, 0x01 } );
  CALL_TEST( test_read_float, +float_qnan, { 0x7F, 0x80, 0x00, 0x01 } );

  // Normal values - double
  CALL_TEST( test_read_float,  0.0,
             { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } );
  CALL_TEST( test_read_float, +1.01,
             { 0x3F, 0xF0, 0x28, 0xF5, 0xC2, 0x8F, 0x5C, 0x29 } );
  CALL_TEST( test_read_float, -1.01,
             { 0xBF, 0xF0, 0x28, 0xF5, 0xC2, 0x8F, 0x5C, 0x29 } );
  CALL_TEST( test_read_float, +1.1e123,
             { 0x59, 0x7A, 0x9F, 0xC3, 0x03, 0x5E, 0x18, 0x09 } );
  CALL_TEST( test_read_float, -1.1e-123,
             { 0xA6, 0x67, 0x44, 0xE8, 0x54, 0xEE, 0xA5, 0x5D } );

  // Special values - double
  CALL_TEST( test_read_float,  double_min,
             { 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } );
  CALL_TEST( test_read_float,  double_max,
             { 0x7F, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } );
  CALL_TEST( test_read_float, -double_inf,
             { 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } );
  CALL_TEST( test_read_float, +double_inf,
             { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } );
  CALL_TEST( test_read_float, -double_qnan,
             { 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } );
  CALL_TEST( test_read_float, +double_qnan,
             { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } );

  // Invalid length
  CALL_TEST( test_read_float_invalid_value, {} );
  CALL_TEST( test_read_float_invalid_value, { 0x00 } );
  CALL_TEST( test_read_float_invalid_value, { 0x00, 0x00 } );
  CALL_TEST( test_read_float_invalid_value, { 0x00, 0x00, 0x00, 0x00, 0x00 } );
}

// ----------------------------------------------------------------------------
void
test_write_float( double value, size_t length )
{
  vec_t bytes( length, 0xba );
  auto it = bytes.begin();
  klv_write_float( value, it, length );
  EXPECT_EQ( it, bytes.end() );
  it = bytes.begin();
  auto const result = klv_read_float( it, length );
  if( std::isnan( value ) )
  {
    // GTest won't print result value on fail, so do it manually
    EXPECT_TRUE( std::isnan( result ) ) << "result: " << result;
    EXPECT_EQ( std::signbit( value ), std::signbit( result ) );
  }
  else
  {
    EXPECT_EQ( result, value );
  }
}

// ----------------------------------------------------------------------------
template < class E >
void
test_write_float_throw( double value, size_t length )
{
  vec_t bytes( length, 0xba );
  auto it = bytes.begin();
  EXPECT_THROW( klv_write_float( value, it, bytes.size() ), E );
}

// ----------------------------------------------------------------------------
void
test_write_float_invalid_value( double value, size_t length )
{
  test_write_float_throw< kv::invalid_value >( value, length );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_float )
{
  // Normal values - float
  CALL_TEST( test_write_float, +0.0f,   4 );
  CALL_TEST( test_write_float, -0.0f,   4 );
  CALL_TEST( test_write_float, +1.234f, 4 );
  CALL_TEST( test_write_float, -1.234f, 4 );

  // Special values - float
  CALL_TEST( test_write_float,  float_min,  4 );
  CALL_TEST( test_write_float,  float_max,  4 );
  CALL_TEST( test_write_float, -float_inf,  4 );
  CALL_TEST( test_write_float, +float_inf,  4 );
  CALL_TEST( test_write_float, -float_qnan, 4 );
  CALL_TEST( test_write_float, +float_qnan, 4 );
  CALL_TEST( test_write_float, -float_snan, 4 );
  CALL_TEST( test_write_float, +float_snan, 4 );

  // Normal values - double
  CALL_TEST( test_write_float, +0.0,   8 );
  CALL_TEST( test_write_float, -0.0,   8 );
  CALL_TEST( test_write_float, +1.234, 8 );
  CALL_TEST( test_write_float, -1.234, 8 );

  // Special values - double
  CALL_TEST( test_write_float,  double_min,  8 );
  CALL_TEST( test_write_float,  double_max,  8 );
  CALL_TEST( test_write_float, -double_inf,  8 );
  CALL_TEST( test_write_float, +double_inf,  8 );
  CALL_TEST( test_write_float, -double_qnan, 8 );
  CALL_TEST( test_write_float, +double_qnan, 8 );
  CALL_TEST( test_write_float, -double_snan, 8 );
  CALL_TEST( test_write_float, +double_snan, 8 );

  // Invalid length
  CALL_TEST( test_write_float_invalid_value, 0.0, 0 );
  CALL_TEST( test_write_float_invalid_value, 0.0, 1 );
  CALL_TEST( test_write_float_invalid_value, 0.0, 2 );
  CALL_TEST( test_write_float_invalid_value, 0.0, 3 );
  CALL_TEST( test_write_float_invalid_value, 0.0, 5 );
  CALL_TEST( test_write_float_invalid_value, 0.0, 9 );
}

// ----------------------------------------------------------------------------
void
test_read_imap( uint64_t int_value, size_t length, double double_value,
                double minimum, double maximum )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_int< uint64_t >( int_value, it, length );
  it = data.begin();

  auto const result = klv_read_imap( minimum, maximum, it, length );
  if( std::isnan( double_value ) )
  {
    // GTest won't print result value on fail, so do it manually
    EXPECT_TRUE( std::isnan( result ) ) << "result: " << result;
    EXPECT_EQ( std::signbit( double_value ), std::signbit( result ) );
  }
  else
  {
    EXPECT_DOUBLE_EQ( double_value, result );
  }
  EXPECT_EQ( data.end(), it );
}

// ----------------------------------------------------------------------------
template < class E >
void
test_read_imap_throw( uint64_t int_value, size_t length,
                      double minimum, double maximum )
{
  auto const write_length = std::max< size_t >( 1, length );
  auto data = vec_t( write_length, 0xba );
  auto it = data.begin();
  klv_write_int< uint64_t >( int_value, it, write_length );
  it = data.begin();
  EXPECT_THROW( klv_read_imap( minimum, maximum, it, length ), E );
  EXPECT_EQ( data.begin(), it );
}

// ----------------------------------------------------------------------------
void
test_read_imap_type_overflow( uint64_t int_value, size_t length,
                              double minimum, double maximum )
{
  test_read_imap_throw< kv::metadata_type_overflow >(
    int_value, length, minimum, maximum );
}

// ----------------------------------------------------------------------------
void
test_read_imap_logic_error( uint64_t int_value, size_t length,
                            double minimum, double maximum )
{
  test_read_imap_throw< std::logic_error >(
    int_value, length, minimum, maximum );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_imap )
{
  // Normal values
  CALL_TEST( test_read_imap, 0,       1, -1.0, -1.0, 1.0 );
  CALL_TEST( test_read_imap, 1 << 6,  1,  0.0, -1.0, 1.0 );
  CALL_TEST( test_read_imap, 1 << 7,  1,  1.0, -1.0, 1.0 );
  CALL_TEST( test_read_imap, 0,       3, -1.0, -1.0, 1.0 );
  CALL_TEST( test_read_imap, 1 << 22, 3,  0.0, -1.0, 1.0 );
  CALL_TEST( test_read_imap, 1 << 23, 3,  1.0, -1.0, 1.0 );

  // Values from examples in ST1201.4, Table 6
  CALL_TEST( test_read_imap, 0x000000, 3,      -900.0, -900.0, 19000.0 );
  CALL_TEST( test_read_imap, 0x038400, 3,         0.0, -900.0, 19000.0 );
  CALL_TEST( test_read_imap, 0x038E00, 3,        10.0, -900.0, 19000.0 );
  CALL_TEST( test_read_imap, 0xE80000, 3, -double_inf, -900.0, 19000.0 );

  // Values from examples in ST1201.4, Table 7
  CALL_TEST( test_read_imap, 0x0000, 2, 0.1,               0.1, 0.9 );
  CALL_TEST( test_read_imap, 0x3333, 2, 0.499993896484375, 0.1, 0.9 );
  CALL_TEST( test_read_imap, 0x6666, 2, 0.899987792968750, 0.1, 0.9 );
  CALL_TEST( test_read_imap, 0xE800, 2, -double_inf,       0.1, 0.9 );

  // Special values
  CALL_TEST( test_read_imap, _imap_infinity(   false, 5 ), 5,
             +double_inf,  1.0, 2.0 );
  CALL_TEST( test_read_imap, _imap_infinity(   true,  5 ), 5,
             -double_inf,  1.0, 2.0 );
  CALL_TEST( test_read_imap, _imap_quiet_nan(  false, 5 ), 5,
             +double_qnan, 1.0, 2.0 );
  CALL_TEST( test_read_imap, _imap_quiet_nan(  true,  5 ), 5,
             -double_qnan, 1.0, 2.0 );
  CALL_TEST( test_read_imap, _imap_signal_nan( false, 5 ), 5,
             +double_snan, 1.0, 2.0 );
  CALL_TEST( test_read_imap, _imap_signal_nan( true,  5 ), 5,
             -double_snan, 1.0, 2.0 );

  // Values too large for native type
  CALL_TEST( test_read_imap_type_overflow, uint64_max, 9, -123.0, 321.0 );

  // Invalid arguments
  CALL_TEST( test_read_imap_logic_error, 0, 0,         0.0,         1.0 );
  CALL_TEST( test_read_imap_logic_error, 0, 1,         0.0,         0.0 );
  CALL_TEST( test_read_imap_logic_error, 0, 1,         0.0,        -1.0 );
  CALL_TEST( test_read_imap_logic_error, 0, 1, -double_inf,         0.0 );
  CALL_TEST( test_read_imap_logic_error, 0, 1,         0.0, +double_inf );
  CALL_TEST( test_read_imap_logic_error, 0, 1, double_qnan,         0.0 );
  CALL_TEST( test_read_imap_logic_error, 0, 1,         0.0, double_qnan );
}

// ----------------------------------------------------------------------------
void
test_write_imap( double value, double expected_value, size_t length,
                 double minimum, double maximum, bool force_exact = true )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  klv_write_imap( value, minimum, maximum, it, data.size() );
  EXPECT_EQ( data.end(), it );
  it = data.begin();

  auto const result = klv_read_imap( minimum, maximum, it, data.size() );
  if( std::isnan( expected_value ) )
  {
    // GTest won't print result value on fail, so do it manually
    EXPECT_TRUE( std::isnan( result ) ) << "result: " << result;
    EXPECT_EQ( std::signbit( expected_value ), std::signbit( result ) );
  }
  else if( force_exact || value == 0.0 || !std::isfinite( expected_value ) )
  {
    // Zero should be mapped (floating-point-) exactly
    EXPECT_DOUBLE_EQ( expected_value, result );
  }
  else
  {
    auto const precision = klv_imap_precision( minimum, maximum, length );
    EXPECT_NEAR( expected_value, result, precision / 2.0 );
  }
  EXPECT_EQ( data.end(), it );
}

// ----------------------------------------------------------------------------
void
test_write_imap( double value, size_t length, double minimum, double maximum )
{
  test_write_imap( value, value, length, minimum, maximum, false );
}

// ----------------------------------------------------------------------------
template < class E >
void
test_write_imap_throw( double value, double minimum, double maximum,
                       size_t length )
{
  auto data = vec_t( length, 0xba );
  auto it = data.begin();
  EXPECT_THROW( klv_write_imap( value, minimum, maximum, it, data.size() ),
                E );
  EXPECT_EQ( data.begin(), it );
}

// ----------------------------------------------------------------------------
void
test_write_imap_logic_error( double value, double minimum, double maximum,
                             size_t length )
{
  test_write_imap_throw< std::logic_error >( value, minimum, maximum, length );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_imap )
{
  // Valid values
  CALL_TEST( test_write_imap, -1.0,   8, -1.0, 1.0 );
  CALL_TEST( test_write_imap, -0.765, 7, -1.0, 1.0 );
  CALL_TEST( test_write_imap, -0.5,   5, -1.0, 1.0 );
  CALL_TEST( test_write_imap,  0.0,   3, -1.0, 1.0 );
  CALL_TEST( test_write_imap,  0.72,  1, -1.0, 1.0 );
  CALL_TEST( test_write_imap,  0.99,  2, -1.0, 1.0 );
  CALL_TEST( test_write_imap,  1.0,   8, -1.0, 1.0 );

  // Values from examples in ST1201.4, Table 6
  CALL_TEST( test_write_imap,      -900.0, 3, -900.0, 19000.0 );
  CALL_TEST( test_write_imap,         0.0, 3, -900.0, 19000.0 );
  CALL_TEST( test_write_imap,        10.0, 3, -900.0, 19000.0 );
  CALL_TEST( test_write_imap, -double_inf, 3, -900.0, 19000.0 );

  // Values from examples in ST1201.4, Table 7
  CALL_TEST( test_write_imap,         0.1,                    2, 0.1, 0.9 );
  CALL_TEST( test_write_imap,         0.5, 0.499993896484375, 2, 0.1, 0.9 );
  CALL_TEST( test_write_imap,         0.9, 0.899987792968750, 2, 0.1, 0.9 );
  CALL_TEST( test_write_imap, -double_inf,                    2, 0.1, 0.9 );

  // Special values
  CALL_TEST( test_write_imap,  double_inf,  1, 0.0, 1.0 );
  CALL_TEST( test_write_imap, -double_inf,  2, 0.0, 1.0 );
  CALL_TEST( test_write_imap,  double_qnan, 3, 0.0, 1.0 );
  CALL_TEST( test_write_imap, -double_qnan, 4, 0.0, 1.0 );
  CALL_TEST( test_write_imap,  double_snan, 5, 0.0, 1.0 );
  CALL_TEST( test_write_imap, -double_snan, 8, 0.0, 1.0 );
}

// ----------------------------------------------------------------------------
void
test_read_string( std::string const& s, vec_t const& bytes )
{
  auto it = bytes.cbegin();
  EXPECT_EQ( s, klv_read_string( it, bytes.size() ) );
  EXPECT_EQ( bytes.cend(), it );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_string )
{
  // Here we tolerate reading zero bytes as empty string, though in practice
  // all strings should have positive length. A case could be made for throwing
  // an exception instead
  CALL_TEST( test_read_string, "",   {} );
  CALL_TEST( test_read_string, "",   { '\0' } );
  CALL_TEST( test_read_string, "\1", { '\1' } );
  CALL_TEST( test_read_string, "Kitware",
             { 'K', 'i', 't', 'w', 'a', 'r', 'e' } );
  CALL_TEST( test_read_string, { "\0Kitware\0", 9 },
             { '\0', 'K', 'i', 't', 'w', 'a', 'r', 'e', '\0' } );
}

// ----------------------------------------------------------------------------
template < class E >
void
test_write_string_throw( std::string const& s, size_t max_length )
{
  auto data = vec_t( max_length, 0xba );
  auto it = data.begin();
  EXPECT_THROW( klv_write_string( s, it, max_length ), E );
  EXPECT_EQ( data.begin(), it );
}

// ----------------------------------------------------------------------------
void
test_write_string_buffer_overflow( std::string const& s, size_t max_length )
{
  test_write_string_throw< kv::metadata_buffer_overflow >( s, max_length );
}

// ----------------------------------------------------------------------------
void
test_write_string_type_overflow( std::string const& s, size_t max_length )
{
  test_write_string_throw< kv::metadata_type_overflow >( s, max_length );
}

// ----------------------------------------------------------------------------
void
test_write_string( std::string const& s )
{
  auto data = vec_t( klv_string_length( s ), 0xba );
  auto it = data.begin();
  klv_write_string( s, it, data.size() );
  EXPECT_EQ( it, data.end() );
  it = data.begin();
  EXPECT_EQ( s, klv_read_string( it, data.size() ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_string )
{
  // Valid values
  CALL_TEST( test_write_string, "" );
  CALL_TEST( test_write_string, "\1" );
  CALL_TEST( test_write_string, "Kitware" );
  CALL_TEST( test_write_string, { "\0Kitware\0", 9 } );

  // Not enough buffer space given
  CALL_TEST( test_write_string_buffer_overflow, "",                   0 );
  CALL_TEST( test_write_string_buffer_overflow, "\n",                 0 );
  CALL_TEST( test_write_string_buffer_overflow, "Kitware",            6 );
  CALL_TEST( test_write_string_buffer_overflow, { "\0Kitware\0", 9 }, 8 );

  // String which can't be written
  CALL_TEST( test_write_string_type_overflow, { "\0", 1 }, 1 );
}

// ----------------------------------------------------------------------------
void
test_string_length( std::string const& s, size_t length )
{
  EXPECT_EQ( length, klv_string_length( s ) );
}

// ----------------------------------------------------------------------------
template < class E >
void
test_string_length_throw( std::string const& s )
{
  EXPECT_THROW( klv_string_length( s ), E );
}

// ----------------------------------------------------------------------------
void
test_string_length_type_overflow( std::string const& s )
{
  test_string_length_throw< kv::metadata_type_overflow >( s );
}

// ----------------------------------------------------------------------------
TEST ( klv, string_length )
{
  // Valid values
  CALL_TEST( test_string_length, "",                   1 );
  CALL_TEST( test_string_length, "\1",                 1 );
  CALL_TEST( test_string_length, "Kitware",            7 );
  CALL_TEST( test_string_length, { "\0Kitware\0", 9 }, 9 );

  // String which can't be written
  CALL_TEST( test_string_length_type_overflow, { "\0", 1 } );
}
