// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1202 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1010.h>
#include <arrows/klv/klv_1202.h>

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
  using format_t = klv_1202_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
auto const expected_result = klv_local_set{
  { KLV_1202_X_NUMERATOR_X_FACTOR, 1.0 },
  { KLV_1202_X_NUMERATOR_Y_FACTOR, 2.0 },
  { KLV_1202_X_NUMERATOR_CONSTANT, 3.0 },
  { KLV_1202_Y_NUMERATOR_X_FACTOR, 4.0 },
  { KLV_1202_Y_NUMERATOR_Y_FACTOR, 5.0 },
  { KLV_1202_Y_NUMERATOR_CONSTANT, 6.0 },
  { KLV_1202_DENOMINATOR_X_FACTOR, 7.0 },
  { KLV_1202_DENOMINATOR_Y_FACTOR, 8.0 },
  { KLV_1202_SDCC_FLP,
    klv_1010_sdcc_flp{
      { KLV_1202_X_NUMERATOR_X_FACTOR,
        KLV_1202_X_NUMERATOR_Y_FACTOR,
        KLV_1202_X_NUMERATOR_CONSTANT,
        KLV_1202_Y_NUMERATOR_X_FACTOR,
        KLV_1202_Y_NUMERATOR_Y_FACTOR,
        KLV_1202_Y_NUMERATOR_CONSTANT,
        KLV_1202_DENOMINATOR_X_FACTOR,
        KLV_1202_DENOMINATOR_Y_FACTOR },
      { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 },
      {} } },
  { KLV_1202_VERSION,              uint64_t{ 2 } },
  { KLV_1202_TRANSFORMATION_TYPE,  KLV_1202_TRANSFORMATION_TYPE_OPTICAL } };

auto const input_bytes = klv_bytes_t{
  0x01, 0x04, // KLV_1202_X_NUMERATOR_X_FACTOR
  0x3F, 0x80, 0x00, 0x00,
  0x02, 0x04, // KLV_1202_X_NUMERATOR_Y_FACTOR
  0x40, 0x00, 0x00, 0x00,
  0x03, 0x04, // KLV_1202_X_NUMERATOR_CONSTANT
  0x40, 0x40, 0x00, 0x00,
  0x04, 0x04, // KLV_1202_Y_NUMERATOR_X_FACTOR
  0x40, 0x80, 0x00, 0x00,
  0x05, 0x04, // KLV_1202_Y_NUMERATOR_Y_FACTOR
  0x40, 0xA0, 0x00, 0x00,
  0x06, 0x04, // KLV_1202_Y_NUMERATOR_CONSTANT
  0x40, 0xC0, 0x00, 0x00,
  0x07, 0x04, // KLV_1202_DENOMINATOR_X_FACTOR
  0x40, 0xE0, 0x00, 0x00,
  0x08, 0x04, // KLV_1202_DENOMINATOR_Y_FACTOR
  0x41, 0x00, 0x00, 0x00,
  0x09, 0x22, // KLV_1202_SDCC_FLP
  0x08, // Matrix size
  0x40, // Parse control
  0x3F, 0x80, 0x00, 0x00, // Sigma
  0x40, 0x00, 0x00, 0x00,
  0x40, 0x40, 0x00, 0x00,
  0x40, 0x80, 0x00, 0x00,
  0x40, 0xA0, 0x00, 0x00,
  0x40, 0xC0, 0x00, 0x00,
  0x40, 0xE0, 0x00, 0x00,
  0x41, 0x00, 0x00, 0x00,
  0x0A, 0x01, // KLV_1202_VERSION
  0x02,
  0x0B, 0x01, // KLV_1202_TRANSFORMATION_TYPE
  0x04, };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1202 )
{
  CALL_TEST( test_read_write, expected_result, input_bytes );
}
