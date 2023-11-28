// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1607 read / write.

#include "data_format.h"

#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1607.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_read_write_0601( klv_value const& expected_result,
                      klv_bytes_t const& input_bytes )
{
  using format_t = klv_0601_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, apply_derive_1607 )
{
  auto const parent = klv_local_set{
    { 1, 3.0 },
    { 2, 4.0 },
    { 3, {} },
    { 4, 5.0 },
    { 4, 6.0 },
    { 5, 7.0 },
    { 5, 8.0 },
    { 6, 9.0 },
    { 6, 0.0 }, };
  auto const child = klv_local_set{
    { 2, 5.0 },
    { 3, 10.0 },
    { 4, 123.0 },
    { 4, 5.0 },
    { 5, {} }, };
  auto const expected_result = klv_local_set{
    { 1, 3.0 },
    { 2, 5.0 },
    { 3, 10.0 },
    { 4, 5.0 },
    { 4, 123.0 },
    { 5, {} },
    { 6, 9.0 },
    { 6, 0.0 }, };

  auto result = parent;
  klv_1607_apply_child( result, child );
  ASSERT_EQ( expected_result, result );

  auto const rederived_child = klv_1607_derive_child( parent, result );
  ASSERT_EQ( child, rederived_child );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1607_0601 )
{
  auto const expected_result = klv_local_set{
    { KLV_0601_PRECISION_TIMESTAMP, uint64_t{ 0x1234 } },
    { KLV_0601_VERSION_NUMBER, uint64_t{ 17 } },
    { KLV_0601_MISSION_ID, std::string{ "ALPHA" } },
    { KLV_0601_AMEND_LOCAL_SET,
      klv_local_set{
        { KLV_0601_MISSION_ID, std::string{ "BRAVO" } } } },
    { KLV_0601_SEGMENT_LOCAL_SET,
      klv_local_set{
        { KLV_0601_ALTERNATE_PLATFORM_NAME, std::string{ "LARRY" } } } },
    { KLV_0601_SEGMENT_LOCAL_SET,
      klv_local_set{
        { KLV_0601_ALTERNATE_PLATFORM_NAME, std::string{ "JOHNNY" } },
        { KLV_0601_LASER_PRF_CODE, uint64_t{ 1111 } } } } };
  auto const input_bytes = klv_bytes_t{
    0x02, 0x08, // KLV_0601_PRECISION_TIMESTAMP
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34,
    0x41, 0x01, // KLV_0601_VERSION_NUMBER
    0x11,
    0x03, 0x05, // KLV_0601_MISSION_ID
    'A', 'L', 'P', 'H', 'A',
    0x65, 0x07, // KLV_0601_AMEND_LOCAL_SET
    0x03, 0x05, // KLV_0601_MISSION_ID
    'B', 'R', 'A', 'V', 'O',
    0x64, 0x07, // KLV_0601_SEGMENT_LOCAL_SET
    0x46, 0x05, // KLV_0601_ALTERNATE_PLATFORM_NAME
    'L', 'A', 'R', 'R', 'Y',
    0x64, 0x0C, // KLV_0601_SEGMENT_LOCAL_SET
    0x46, 0x06, // KLV_0601_ALTERNATE_PLATFORM_NAME
    'J', 'O', 'H', 'N', 'N', 'Y',
    0x3E, 0x02, // KLV_0601_LASER_PRF_CODE
    0x04, 0x57, };

  CALL_TEST( test_read_write_0601, expected_result, input_bytes );
}
