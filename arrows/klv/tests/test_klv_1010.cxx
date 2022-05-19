// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1010 read / write.

#include "data_format.h"

#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1010.h>
#include <arrows/klv/klv_1107.h>

using kld = klv_lengthy< double >;

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
void
test_read_write_1107( klv_value const& expected_result,
                      klv_bytes_t const& input_bytes )
{
  using format_t = klv_1107_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1010_0601 )
{
  auto const expected_result = klv_local_set{
    { KLV_0601_VERSION_NUMBER,         uint64_t{ 13 } },
    { KLV_0601_SENSOR_LATITUDE,        kld{ 60.176822966978335 } },
    { KLV_0601_SENSOR_LONGITUDE,       kld{ 128.42675904204452 } },
    { KLV_0601_PLATFORM_HEADING_ANGLE, kld{ 159.97436484321355 } },
    { KLV_0601_SDCC_FLP,
      klv_1010_sdcc_flp{
        { KLV_0601_SENSOR_LATITUDE,
          KLV_0601_SENSOR_LONGITUDE,
          KLV_0601_PLATFORM_HEADING_ANGLE },
        { 1.0, 2.0, 0.0 },
        { -0.5, 0.0, 0.0 },
        4, 3,
        false, true,
        true,
        true, } } };

  auto const input_bytes = klv_bytes_t{
    0x41, 0x01, // KLV_0601_VERSION_NUMBER
    0x0D,
    0x0D, 0x04, // KLV_0601_SENSOR_LATITUDE
    0x55, 0x95, 0xB6, 0x6D,
    0x0E, 0x04, // KLV_0601_SENSOR_LONGITUDE
    0x5B, 0x53, 0x60, 0xC4,
    0x05, 0x02, // KLV_0601_PLATFORM_HEADING_ANGLE
    0x71, 0xC2,
    0x66, 0x13, // KLV_0601_SDCC_FLP
    0x03, // Matrix size
    0xB3, 0x04, // Parse control
    0x80, // Sparse bit vector
    0x3F, 0x80, 0x00, 0x00, // Sigma
    0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, // Rho
  };

  CALL_TEST( test_read_write_0601, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1010_1107 )
{
  auto const expected_result = klv_local_set{
    { KLV_1107_SLANT_RANGE, {} },
    { KLV_1107_EFFECTIVE_FOCAL_LENGTH, {} },
    { KLV_1107_SENSOR_ECEF_VELOCITY_Z, {} },
    { KLV_1107_BORESIGHT_DELTA_ANGLE_1, {} },
    { KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE, {} },
    { KLV_1107_SENSOR_ABSOLUTE_AZIMUTH, {} },
    { KLV_1107_SDCC_FLP,
      klv_1010_sdcc_flp{
        { KLV_1107_SLANT_RANGE,
          KLV_1107_EFFECTIVE_FOCAL_LENGTH,
          KLV_1107_SENSOR_ECEF_VELOCITY_Z,
          KLV_1107_BORESIGHT_DELTA_ANGLE_1,
          KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE,
          KLV_1107_SENSOR_ABSOLUTE_AZIMUTH },
        { 300.0, 300.0, 30.0, 1.0, 1.0, 0.125 },
        {},
        2, 0,
        true, false,
        true,
        false
      } }
  };

  auto const input_bytes = klv_bytes_t{
    KLV_1107_SLANT_RANGE,                  0x00,
    KLV_1107_EFFECTIVE_FOCAL_LENGTH,       0x00,
    KLV_1107_SENSOR_ECEF_VELOCITY_Z,       0x00,
    KLV_1107_BORESIGHT_DELTA_ANGLE_1,      0x00,
    KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE, 0x00,
    KLV_1107_SENSOR_ABSOLUTE_AZIMUTH,      0x00,

    KLV_1107_SDCC_FLP, 0x0F,
    0x06, // Matrix size
    0x80, 0x12, // Parse control
    // No sparse bit vector
    0x25, 0x80, // Sigma
    0x25, 0x80,
    0x1E, 0x00,
    0x40, 0x00,
    0x80, 0x00,
    0x40, 0x00,
    // No rho
  };

  CALL_TEST( test_read_write_1107, expected_result, input_bytes );
}
