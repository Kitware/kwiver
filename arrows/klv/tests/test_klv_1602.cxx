// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1602 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1602.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_read_write( klv_value const& expected_result,
                 klv_bytes_t const& input_bytes )
{
  using format_t = klv_1602_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
auto const expected_result = klv_local_set{
  { KLV_1602_TIMESTAMP,                   uint64_t{ 0x0102030400000000 } },
  { KLV_1602_VERSION,                     uint64_t{ 1 } },
  { KLV_1602_SOURCE_IMAGE_ROWS,           uint64_t{ 720 } },
  { KLV_1602_SOURCE_IMAGE_COLUMNS,        uint64_t{ 1080 } },
  { KLV_1602_SOURCE_IMAGE_AOI_ROWS,       uint64_t{ 360 } },
  { KLV_1602_SOURCE_IMAGE_AOI_COLUMNS,    uint64_t{ 480 } },
  { KLV_1602_SOURCE_IMAGE_AOI_POSITION_X, int64_t{ 64 } },
  { KLV_1602_SOURCE_IMAGE_AOI_POSITION_Y, int64_t{ 48 } },
  { KLV_1602_SUB_IMAGE_ROWS,              uint64_t{ 120 } },
  { KLV_1602_SUB_IMAGE_COLUMNS,           uint64_t{ 300 } },
  { KLV_1602_SUB_IMAGE_POSITION_X,        int64_t{ 128 } },
  { KLV_1602_SUB_IMAGE_POSITION_Y,        int64_t{ 64 } },
  { KLV_1602_ACTIVE_SUB_IMAGE_ROWS,       uint64_t{ 480 } },
  { KLV_1602_ACTIVE_SUB_IMAGE_COLUMNS,    uint64_t{ 720 } },
  { KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_X,   int64_t{ -64 } },
  { KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_Y,   int64_t{ -128 } },
  { KLV_1602_TRANSPARENCY,                uint64_t{ 0 } },
  { KLV_1602_Z_ORDER,                     uint64_t{ 128 } }, };

// ----------------------------------------------------------------------------
auto const input_bytes = klv_bytes_t{
  0x01, 0x08, // KLV_1602_TIMESTAMP
  0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x01, // KLV_1602_VERSION
  0x01,
  0x03, 0x02, // KLV_1602_SOURCE_IMAGE_ROWS
  0x02, 0xD0,
  0x04, 0x02, // KLV_1602_SOURCE_IMAGE_COLUMNS
  0x04, 0x38,
  0x05, 0x02, // KLV_1602_SOURCE_IMAGE_AOI_ROWS
  0x01, 0x68,
  0x06, 0x02, // KLV_1602_SOURCE_IMAGE_AOI_COLUMNS
  0x01, 0xE0,
  0x07, 0x01, // KLV_1602_SOURCE_IMAGE_AOI_POSITION_X
  0x40,
  0x08, 0x01, // KLV_1602_SOURCE_IMAGE_AOI_POSITION_Y
  0x30,
  0x09, 0x01, // KLV_1602_SUB_IMAGE_ROWS
  0x78,
  0x0A, 0x02, // KLV_1602_SUB_IMAGE_COLUMNS
  0x01, 0x2C,
  0x0B, 0x02, // KLV_1602_SUB_IMAGE_POSITION_X
  0x00, 0x80,
  0x0C, 0x01, // KLV_1602_SUB_IMAGE_POSITION_Y
  0x40,
  0x0D, 0x02, // KLV_1602_ACTIVE_SUB_IMAGE_ROWS
  0x01, 0xE0,
  0x0E, 0x02, // KLV_1602_ACTIVE_SUB_IMAGE_COLUMNS
  0x02, 0xD0,
  0x0F, 0x01, // KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_X
  0xC0,
  0x10, 0x01, // KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_Y
  0x80,
  0x11, 0x01, // KLV_1602_TRANSPARENCY
  0x00,
  0x12, 0x01, // KLV_1602_Z_ORDER
  0x80 };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1602 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}
