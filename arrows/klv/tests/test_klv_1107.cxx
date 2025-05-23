// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1107 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1107.h>
#include <arrows/klv/klv_imap.h>
#include <arrows/klv/klv_packet.h>

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
  using format_t = klv_1107_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
using kld = klv_lengthy< double >;
auto const expected_result = klv_local_set{
  { KLV_1107_SENSOR_ECEF_POSITION_X,               kli( -831506944.0 ) },
  { KLV_1107_SENSOR_ECEF_POSITION_Y,               kli( -831441408.0 ) },
  { KLV_1107_SENSOR_ECEF_POSITION_Z,               kli( -831375872.0 ) },
  { KLV_1107_SENSOR_ECEF_VELOCITY_X,               kli( -19858.0 ) },
  { KLV_1107_SENSOR_ECEF_VELOCITY_Y,               kli( -19856.0 ) },
  { KLV_1107_SENSOR_ECEF_VELOCITY_Z,               kli( -19854.0 ) },
  { KLV_1107_SENSOR_ABSOLUTE_AZIMUTH,              kli( 1.03125 ) },
  { KLV_1107_SENSOR_ABSOLUTE_PITCH,                kli( 0.03125 ) },
  { KLV_1107_SENSOR_ABSOLUTE_ROLL,                 kli( 0.03125 ) },
  { KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE,         kli( 0.046875 ) },
  { KLV_1107_SENSOR_ABSOLUTE_PITCH_RATE,           kli( 0.046875 ) },
  { KLV_1107_SENSOR_ABSOLUTE_ROLL_RATE,            kli( 0.046875 ) },
  { KLV_1107_BORESIGHT_OFFSET_DELTA_X,             kli( 244 ) },
  { KLV_1107_BORESIGHT_OFFSET_DELTA_Y,             kli( 252 ) },
  { KLV_1107_BORESIGHT_OFFSET_DELTA_Z,             kli( 260 ) },
  { KLV_1107_BORESIGHT_DELTA_ANGLE_1,              kli( 0.1875 ) },
  { KLV_1107_BORESIGHT_DELTA_ANGLE_2,              kli( 0.19140625 ) },
  { KLV_1107_BORESIGHT_DELTA_ANGLE_3,              kli( 0.1953125 ) },
  { KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_Y, kli( -1.0 ) },
  { KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_X, kli( -0.5 ) },
  { KLV_1107_EFFECTIVE_FOCAL_LENGTH,               kli( 4096.0 ) },
  { KLV_1107_RADIAL_DISTORTION_CONSTANT,           1.0 },
  { KLV_1107_RADIAL_DISTORTION_PARAMETER_1,        2.0 },
  { KLV_1107_RADIAL_DISTORTION_PARAMETER_2,        3.0 },
  { KLV_1107_RADIAL_DISTORTION_PARAMETER_3,        4.0 },
  { KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_1,    5.0 },
  { KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_2,    6.0 },
  { KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_3,    7.0 },
  { KLV_1107_DIFFERENTIAL_SCALE_AFFINE_PARAMETER,  8.0 },
  { KLV_1107_SKEWNESS_AFFINE_PARAMETER,            9.0 },
  { KLV_1107_SLANT_RANGE,                          10.0 },
  { KLV_1107_SDCC_FLP,                             {} },
  { KLV_1107_GENERALIZED_TRANSFORMATION_LOCAL_SET, {} },
  { KLV_1107_IMAGE_ROWS,                           uint64_t{ 720 } },
  { KLV_1107_IMAGE_COLUMNS,                        uint64_t{ 1080 } },
  { KLV_1107_PIXEL_SIZE_X,                         kli( 0.0626 ) },
  { KLV_1107_PIXEL_SIZE_Y,                         kli( 0.09385 ) },
  { KLV_1107_SLANT_RANGE_PEDIGREE, KLV_1107_SLANT_RANGE_PEDIGREE_MEASURED },
  { KLV_1107_LINE_COORDINATE,                      11.0 },
  { KLV_1107_SAMPLE_COORDINATE,                    12.0 },
  { KLV_1107_LRF_DIVERGENCE,                       13.0 },
  { KLV_1107_RADIAL_DISTORTION_VALID_RANGE,        14.0 },
  { KLV_1107_PRECISION_TIMESTAMP, uint64_t{ 0x0001020304050607 } },
  { KLV_1107_DOCUMENT_VERSION,                     uint64_t{ 4 } },
  { KLV_1107_LEAP_SECONDS,                         int64_t{ 37 } },
  { KLV_1107_EFFECTIVE_FOCAL_LENGTH_EXTENDED,      kli( 1024.0 ) }
};

// ----------------------------------------------------------------------------
auto const input_bytes = klv_bytes_t{
  // KLV_1107_SENSOR_ECEF_POSITION_X
  0x01, 0x03, 0x0A, 0x0B, 0x00,
  // KLV_1107_SENSOR_ECEF_POSITION_Y
  0x02, 0x03, 0x0A, 0x0C, 0x00,
  // KLV_1107_SENSOR_ECEF_POSITION_Z
  0x03, 0x03, 0x0A, 0x0D, 0x00,

  // KLV_1107_SENSOR_ECEF_VELOCITY_X
  0x04, 0x03, 0x0A, 0x0B, 0x00,
  // KLV_1107_SENSOR_ECEF_VELOCITY_Y
  0x05, 0x03, 0x0A, 0x0C, 0x00,
  // KLV_1107_SENSOR_ECEF_VELOCITY_Z
  0x06, 0x03, 0x0A, 0x0D, 0x00,

  // KLV_1107_SENSOR_ABSOLUTE_AZIMUTH
  0x07, 0x02, 0x42, 0x00,
  // KLV_1107_SENSOR_ABSOLUTE_PITCH
  0x08, 0x02, 0x42, 0x00,
  // KLV_1107_SENSOR_ABSOLUTE_ROLL
  0x09, 0x02, 0x42, 0x00,

  // KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE
  0x0A, 0x02, 0x43, 0x00,
  // KLV_1107_SENSOR_ABSOLUTE_PITCH_RATE
  0x0B, 0x02, 0x43, 0x00,
  // KLV_1107_SENSOR_ABSOLUTE_ROLL_RATE
  0x0C, 0x02, 0x43, 0x00,

  // KLV_1107_BORESIGHT_OFFSET_DELTA_X
  0x0D, 0x02, 0x44, 0x00,
  // KLV_1107_BORESIGHT_OFFSET_DELTA_Y
  0x0E, 0x02, 0x45, 0x00,
  // KLV_1107_BORESIGHT_OFFSET_DELTA_Z
  0x0F, 0x02, 0x46, 0x00,

  // KLV_1107_BORESIGHT_DELTA_ANGLE_1
  0x10, 0x01, 0x70,
  // KLV_1107_BORESIGHT_DELTA_ANGLE_2
  0x11, 0x01, 0x71,
  // KLV_1107_BORESIGHT_DELTA_ANGLE_3
  0x12, 0x01, 0x72,

  // KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_Y
  0x13, 0x02, 0x30, 0x00,
  // KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_X
  0x14, 0x02, 0x31, 0x00,

  // KLV_1107_EFFECTIVE_FOCAL_LENGTH
  0x15, 0x01, 0x20,

  // KLV_1107_RADIAL_DISTORTION_CONSTANT
  0x16, 0x04, 0x3F, 0x80, 0x00, 0x00,
  // KLV_1107_RADIAL_DISTORTION_PARAMETER_1
  0x17, 0x04, 0x40, 0x00, 0x00, 0x00,
  // KLV_1107_RADIAL_DISTORTION_PARAMETER_2
  0x18, 0x04, 0x40, 0x40, 0x00, 0x00,
  // KLV_1107_RADIAL_DISTORTION_PARAMETER_3
  0x19, 0x04, 0x40, 0x80, 0x00, 0x00,

  // KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_1
  0x1A, 0x04, 0x40, 0xA0, 0x00, 0x00,
  // KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_2
  0x1B, 0x04, 0x40, 0xC0, 0x00, 0x00,
  // KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_3
  0x1C, 0x04, 0x40, 0xE0, 0x00, 0x00,

  // KLV_1107_DIFFERENTIAL_SCALE_AFFINE_PARAMETER
  0x1D, 0x04, 0x41, 0x00, 0x00, 0x00,
  // KLV_1107_SKEWNESS_AFFINE_PARAMETER
  0x1E, 0x04, 0x41, 0x10, 0x00, 0x00,

  // KLV_1107_SLANT_RANGE
  0x1F, 0x04, 0x41, 0x20, 0x00, 0x00,

  // KLV_1107_SDCC_FLP
  0x20, 0x00,

  // KLV_1107_GENERALIZED_TRANSFORMATION_LOCAL_SET
  0x21, 0x00,

  // KLV_1107_IMAGE_ROWS
  0x22, 0x02, 0x02, 0xD0,
  // KLV_1107_IMAGE_COLUMNS
  0x23, 0x02, 0x04, 0x38,

  // KLV_1107_PIXEL_SIZE_X
  0x24, 0x01, 0x40,
  // KLV_1107_PIXEL_SIZE_Y
  0x25, 0x01, 0x60,

  // KLV_1107_SLANT_RANGE_PEDIGREE
  0x26, 0x01, 0x01,

  // KLV_1107_LINE_COORDINATE
  0x27, 0x04, 0x41, 0x30, 0x00, 0x00,
  // KLV_1107_SAMPLE_COORDINATE
  0x28, 0x04, 0x41, 0x40, 0x00, 0x00,

  // KLV_1107_LRF_DIVERGENCE
  0x29, 0x04, 0x41, 0x50, 0x00, 0x00,

  // KLV_1107_RADIAL_DISTORTION_VALID_RANGE
  0x2A, 0x04, 0x41, 0x60, 0x00, 0x00,

  // KLV_1107_PRECISION_TIMESTAMP
  0x2B, 0x08, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,

  // KLV_1107_DOCUMENT_VERSION
  0x2C, 0x01, 0x04,

  // KLV_1107_LEAP_SECONDS
  0x2E, 0x01, 0x25,

  // KLV_1107_EFFECTIVE_FOCAL_LENGTH_EXTENDED
  0x2F, 0x02, 0x01, 0x00,
};

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1107 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1107_packet )
{
  auto const packet_footer = klv_bytes_t{ KLV_1107_CHECKSUM, 2, 0xA7, 0x5A };
  CALL_TEST( test_read_write_packet,
             expected_result, input_bytes, packet_footer, klv_1107_key() );
}
