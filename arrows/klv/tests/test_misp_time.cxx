// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test MISP timestamp read / write.

#include <tests/test_gtest.h>

#include <arrows/klv/misp_time.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::arrows::klv;

// ----------------------------------------------------------------------------
void
test_round_trip(
  misp_timestamp value,
  misp_timestamp_tag_type tag_type,
  bool is_nano,
  std::vector< uint8_t > const& expected_bytes )
{
  // Setup
  ASSERT_EQ( expected_bytes.size(), misp_timestamp_length() );
  std::vector< uint8_t > buffer( misp_timestamp_length(), 0xEE );

  // Write
  {
    auto it = &*buffer.begin();
    auto const end_it = &*buffer.end();
    write_misp_timestamp( value, it, tag_type, is_nano );
    EXPECT_EQ( end_it, it );
    EXPECT_EQ( expected_bytes, buffer );
  }

  // Read
  {
    auto it = &*buffer.cbegin();
    auto const end_it = &*buffer.cend();
    EXPECT_EQ( is_nano, is_misp_timestamp_nano( it ) );
    auto const read_value = read_misp_timestamp( it );
    EXPECT_EQ( end_it, it );
    EXPECT_EQ( value.nanoseconds(), read_value.nanoseconds() );
    EXPECT_EQ( value.status(), read_value.status() );
  }
}

// ----------------------------------------------------------------------------
TEST( misp_time, round_trip )
{
  using us = std::chrono::microseconds;
  using ns = std::chrono::nanoseconds;

  CALL_TEST(
    test_round_trip,
    misp_timestamp( us( 0x0001'2345'6789'ABCDull ) ),
    MISP_TIMESTAMP_TAG_STRING,
    false,
    { 'M', 'I', 'S', 'P', 'm', 'i', 'c', 'r',
      'o', 's', 'e', 'c', 't', 'i', 'm', 'e',
      0x9F,
      0x00, 0x01, 0xFF, 0x23, 0x45, 0xFF, 0x67, 0x89, 0xFF, 0xAB, 0xCD } );

  CALL_TEST(
    test_round_trip,
    misp_timestamp( ns( 0xFFEE'FFEE'FFEE'FFEEull ), 0x5F ),
    MISP_TIMESTAMP_TAG_UUID,
    true,
    { 0xCF, 0x84, 0x82, 0x78, 0xEE, 0x23, 0x30, 0x6C,
      0x92, 0x65, 0xE8, 0xFE, 0xF2, 0x2F, 0xB8, 0xB8,
      0x5F,
      0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE } );

  CALL_TEST(
    test_round_trip,
    misp_timestamp( us( 0x0001'2345'6789'ABCDull ) ),
    MISP_TIMESTAMP_TAG_UUID,
    false,
    { 0xA8, 0x68, 0x7D, 0xD4, 0xD7, 0x59, 0x37, 0x58,
      0xA5, 0xCE, 0xF0, 0x33, 0x8B, 0x65, 0x45, 0xF1,
      0x9F,
      0x00, 0x01, 0xFF, 0x23, 0x45, 0xFF, 0x67, 0x89, 0xFF, 0xAB, 0xCD } );
}
