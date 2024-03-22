// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1206 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1206.h>
#include <arrows/klv/klv_1303.h>
#include <arrows/klv/klv_imap.h>

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
  using format_t = klv_1206_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
auto const expected_result = klv_local_set{
  { KLV_1206_GRAZING_ANGLE, kli( 64.0 ) },
  { KLV_1206_GROUND_PLANE_SQUINT_ANGLE, kli( -26.0 ) },
  { KLV_1206_LOOK_DIRECTION, KLV_1206_LOOK_DIRECTION_RIGHT },
  { KLV_1206_IMAGE_PLANE, KLV_1206_IMAGE_PLANE_SLANT },
  { KLV_1206_RANGE_RESOLUTION, kli( 524288.0 ) },
  { KLV_1206_CROSS_RANGE_RESOLUTION, kli( 32768.0 ) },
  { KLV_1206_RANGE_IMAGE_PLANE_PIXEL_SIZE, kli( 24576.0 ) },
  { KLV_1206_CROSS_RANGE_IMAGE_PLANE_PIXEL_SIZE, kli( 16384.0 ) },
  { KLV_1206_IMAGE_ROWS, uint64_t{ 720 } },
  { KLV_1206_IMAGE_COLUMNS, uint64_t{ 1080 } },
  { KLV_1206_RANGE_DIRECTION_ANGLE, kli( 12.0 ) },
  { KLV_1206_TRUE_NORTH, kli( 28.0 ) },
  { KLV_1206_RANGE_LAYOVER_ANGLE, kli( 40.0 ) },
  { KLV_1206_GROUND_APERTURE_ANGULAR_EXTENT, kli( 15.0 ) },
  { KLV_1206_APERTURE_DURATION, uint64_t{ 4096 } },
  { KLV_1206_GROUND_TRACK_ANGLE, kli( 64.0 ) },
  { KLV_1206_MINIMUM_DETECTABLE_VELOCITY, kli( 0.5 ) },
  { KLV_1206_TRUE_PULSE_REPETITION_FREQUENCY, kli( 528384.0 ) },
  { KLV_1206_PULSE_REPETITION_FREQUENCY_SCALE_FACTOR, kli( 0.0390625 ) },
  { KLV_1206_TRANSMIT_RF_CENTER_FREQUENCY, kli( 4311744512.0 ) },
  { KLV_1206_TRANSMIT_RF_BANDWIDTH, kli( 469762048.0 ) },
  { KLV_1206_RADAR_CROSS_SECTION_SCALE_FACTOR_POLYNOMIAL,
    klv_1303_mdap< klv_imap >{
      { 4, 2 },
      { klv_imap{ 8192.0 },
        klv_imap{ 16384.0 },
        klv_imap{ 24576.0 },
        klv_imap{ 32768.0 },
        klv_imap{ 40960.0 },
        klv_imap{ 49152.0 },
        klv_imap{ 57344.0 },
        klv_imap{ 65536.0 } } } },
  { KLV_1206_REFERENCE_FRAME_PRECISION_TIMESTAMP,
    uint64_t{ 1311768464867721216 } },
  { KLV_1206_REFERENCE_FRAME_GRAZING_ANGLE, kli( 7.0625 ) },
  { KLV_1206_REFERENCE_FRAME_GROUND_PLANE_SQUINT_ANGLE, kli( -87.875 ) },
  { KLV_1206_REFERENCE_FRAME_RANGE_DIRECTION_ANGLE, kli( 12.25 ) },
  { KLV_1206_REFERENCE_FRAME_RANGE_LAYOVER_ANGLE, kli( 20.25 ) },
  { KLV_1206_DOCUMENT_VERSION, uint64_t{ 1 } } };

auto const input_bytes = klv_bytes_t{
  0x01, 0x02, // KLV_1206_GRAZING_ANGLE
  0x40, 0x00,
  0x02, 0x02, // KLV_1206_GROUND_PLANE_SQUINT_ANGLE
  0x20, 0x00,
  0x03, 0x01, // KLV_1206_LOOK_DIRECTION
  0x01,
  0x04, 0x01, // KLV_1206_IMAGE_PLANE
  0x00,
  0x05, 0x04, // KLV_1206_RANGE_RESOLUTION
  0x40, 0x00, 0x00, 0x00,
  0x06, 0x04, // KLV_1206_CROSS_RANGE_RESOLUTION
  0x04, 0x00, 0x00, 0x00,
  0x07, 0x04, // KLV_1206_RANGE_IMAGE_PLANE_PIXEL_SIZE
  0x03, 0x00, 0x00, 0x00,
  0x08, 0x04, // KLV_1206_CROSS_RANGE_IMAGE_PLANE_PIXEL_SIZE
  0x02, 0x00, 0x00, 0x00,
  0x09, 0x02, // KLV_1206_IMAGE_ROWS
  0x02, 0xD0,
  0x0A, 0x02, // KLV_1206_IMAGE_COLUMNS
  0x04, 0x38,
  0x0B, 0x02, // KLV_1206_RANGE_DIRECTION_ANGLE
  0x03, 0x00,
  0x0C, 0x02, // KLV_1206_TRUE_NORTH
  0x07, 0x00,
  0x0D, 0x02, // KLV_1206_RANGE_LAYOVER_ANGLE
  0x0A, 0x00,
  0x0E, 0x02, // KLV_1206_GROUND_APERTURE_ANGULAR_EXTENT
  0x0F, 0x00,
  0x0F, 0x04, // KLV_1206_APERTURE_DURATION
  0x00, 0x00, 0x10, 0x00,
  0x10, 0x02, // KLV_1206_GROUND_TRACK_ANGLE
  0x10, 0x00,
  0x11, 0x02, // KLV_1206_MINIMUM_DETECTABLE_VELOCITY
  0x00, 0x80,
  0x12, 0x04, // KLV_1206_TRUE_PULSE_REPETITION_FREQUENCY
  0x40, 0x80, 0x00, 0x00,
  0x13, 0x02, // KLV_1206_PULSE_REPETITION_FREQUENCY_SCALE_FACTOR
  0x05, 0x00,
  0x14, 0x04, // KLV_1206_TRANSMIT_RF_CENTER_FREQUENCY
  0x00, 0x80, 0x80, 0x00,
  0x15, 0x04, // KLV_1206_TRANSMIT_RF_BANDWIDTH
  0x00, 0x70, 0x00, 0x00,
  0x16, 0x1D, // KLV_1206_RADAR_CROSS_SECTION_SCALE_FACTOR_POLYNOMIAL

  0x02,                   // Dimension count
  0x04, 0x02,             // Dimension sizes
  0x02,                   // Element size
  0x02,                   // Apa
  0x00, 0x00, 0x00, 0x00, // Apa params
  0x49, 0x74, 0x24, 0x00,
  0x01, 0x00, 0x02, 0x00, // Array
  0x03, 0x00, 0x04, 0x00,
  0x05, 0x00, 0x06, 0x00,
  0x07, 0x00, 0x08, 0x00,

  0x17, 0x08, // KLV_1206_REFERENCE_FRAME_PRECISION_TIMESTAMP
  0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x02, // KLV_1206_REFERENCE_FRAME_GRAZING_ANGLE
  0x07, 0x10,
  0x19, 0x02, // KLV_1206_REFERENCE_FRAME_GROUND_PLANE_SQUINT_ANGLE
  0x01, 0x10,
  0x1A, 0x02, // KLV_1206_REFERENCE_FRAME_RANGE_DIRECTION_ANGLE
  0x03, 0x10,
  0x1B, 0x02, // KLV_1206_REFERENCE_FRAME_RANGE_LAYOVER_ANGLE
  0x05, 0x10,
  0x1C, 0x01, // KLV_1206_DOCUMENT_VERSION
  0x01, };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1206 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}
