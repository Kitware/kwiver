// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV checksum functions.

#include <arrows/klv/klv_checksum.h>

#include <tests/test_gtest.h>

#include <vector>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

using namespace kwiver::arrows::klv;

using vec_t = std::vector< uint8_t >;

// ----------------------------------------------------------------------------
void
test_running_sum_16( uint16_t checksum, vec_t const& data )
{
  EXPECT_EQ( checksum, klv_running_sum_16( &*data.cbegin(), &*data.cend() ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, running_sum_16 )
{
  // Arbitrary byte sequences of varying lengths (even & odd)
  CALL_TEST( test_running_sum_16, 0x0000, {} );
  CALL_TEST( test_running_sum_16, 0xAB00, { 0xAB } );
  CALL_TEST( test_running_sum_16, 0xABCD, { 0xAB, 0xCD } );
  CALL_TEST( test_running_sum_16, 0x9ACD, { 0xAB, 0xCD, 0xEF } );
  CALL_TEST( test_running_sum_16, 0x61CC,
             { 0x12, 0x00, 0x00, 0x43, 0x11, 0x43, 0xAC, 0x46, 0x92 } );
  CALL_TEST( test_running_sum_16, 0x61CD,
             { 0x12, 0x00, 0x00, 0x43, 0x11, 0x43, 0xAC, 0x46, 0x92, 0x01 } );
}

// ----------------------------------------------------------------------------
void
test_crc_16_ccitt( uint16_t checksum, vec_t const& data )
{
  EXPECT_EQ( checksum, klv_crc_16_ccitt( &*data.cbegin(), &*data.cend() ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, klv_crc_16_ccitt )
{
  // MISP Motion Imagery Handbook, p.126; and
  // http://srecord.sourceforge.net/crc16-ccitt.html
  CALL_TEST( test_crc_16_ccitt, 0x1D0F, {} );
  CALL_TEST( test_crc_16_ccitt, 0x9479, { 'A' } );
  CALL_TEST( test_crc_16_ccitt, 0x06C2, { 0x03, 0x05, 0x0b } );
  CALL_TEST( test_crc_16_ccitt, 0xE938, vec_t( 256, 'A' ) );
  CALL_TEST( test_crc_16_ccitt, 0xE5CC,
             { '1', '2', '3', '4', '5', '6', '7', '8', '9' } );
}

// ----------------------------------------------------------------------------
void
test_crc_32_mpeg( uint32_t checksum, vec_t const& data )
{
  EXPECT_EQ( checksum, klv_crc_32_mpeg( &*data.cbegin(), &*data.cend() ) );
}

// ----------------------------------------------------------------------------
TEST ( klv, klv_crc_32_mpeg )
{
  // Verified via https://crccalc.com/
  CALL_TEST( test_crc_32_mpeg, 0xFFFFFFFF, {} );
  CALL_TEST( test_crc_32_mpeg, 0x7E4FD274, { 'A' } );
  CALL_TEST( test_crc_32_mpeg, 0x0376E6E7,
             { '1', '2', '3', '4', '5', '6', '7', '8', '9' } );
}
