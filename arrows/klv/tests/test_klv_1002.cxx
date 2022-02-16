// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1102 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1002.h>
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
  using format_t = klv_1002_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
using kld = klv_lengthy< double >;
auto const expected_result = klv_local_set{
  { KLV_1002_PRECISION_TIMESTAMP, uint64_t{ 0x1234 } },
  { KLV_1002_DOCUMENT_VERSION, uint64_t{ 2 } },
  { KLV_1002_RANGE_IMAGE_ENUMERATIONS,
    klv_1002_enumerations{
      KLV_1002_COMPRESSION_METHOD_NONE,
      KLV_1002_DATA_TYPE_DEPTH_RANGE_IMAGE,
      KLV_1002_SOURCE_RANGE_SENSOR } },
  { KLV_1002_SPRM, kld{ 256.0 } },
  { KLV_1002_SPRM_UNCERTAINTY, kld{ 4.0 } },
  { KLV_1002_SPRM_ROW, kld{ 320 } },
  { KLV_1002_SPRM_COLUMN, kld{ 240 } },
  { KLV_1002_NUMBER_SECTIONS_X, uint64_t{ 4 } },
  { KLV_1002_NUMBER_SECTIONS_Y, uint64_t{ 1 } },
  { KLV_1002_GENERALIZED_TRANSFORMATION_LOCAL_SET,
    klv_local_set{ { KLV_1202_VERSION, uint64_t{ 1 } } } },
  { KLV_1002_SECTION_DATA_PACK,
    klv_1002_section_data_pack{
      2, 0,
      { { 2, 2 },
        { 100.0, 105.0, 95.0, 100.0 } },
      kv::nullopt,
      kld{ 1.0 },
      kld{ 2.0 },
      kv::nullopt } } };

// ----------------------------------------------------------------------------
auto const input_bytes = klv_bytes_t{
  0x01, 0x08, // KLV_1002_PRECISION_TIMESTAMP
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34,
  0x0B, 0x01, // KLV_1002_DOCUMENT_VERSION
  0x02,
  0x0C, 0x01, // KLV_1002_RANGE_IMAGE_ENUMERATIONS
  0x48,
  0x0D, 0x04, // KLV_1002_SPRM
  0x43, 0x80, 0x00, 0x00,
  0x0E, 0x04, // KLV_1002_SPRM_UNCERTAINTY
  0x40, 0x80, 0x00, 0x00,
  0x0F, 0x04, // KLV_1002_SPRM_ROW
  0x43, 0xA0, 0x00, 0x00,
  0x10, 0x04, // KLV_1002_SPRM_COLUMN
  0x43, 0x70, 0x00, 0x00,
  0x11, 0x01, // KLV_1002_NUMBER_SECTIONS_X
  0x04,
  0x12, 0x01, // KLV_1002_NUMBER_SECTIONS_Y
  0x01,
  0x13, 0x03, // KLV_1002_GENERALIZED_TRANSFORMATION_LOCAL_SET
  0x0A, 0x01, // KLV_1202_VERSION
  0x01,
  0x14, 0x25, // KLV_1002_SECTION_DATA_PACK
  0x01, 0x02, // Section Number X
  0x01, 0x00, // Section Number Y
  0x15, // Range Measurements
  0x02,
  0x02, 0x02,
  0x04,
  0x01,
  0x42, 0xC8, 0x00, 0x00, 0x42, 0xD2, 0x00, 0x00,
  0x42, 0xBE, 0x00, 0x00, 0x42, 0xC8, 0x00, 0x00,
  0x00, // No Uncertainty
  0x04, 0x3F, 0x80, 0x00, 0x00, // A
  0x04, 0x40, 0x00, 0x00, 0x00, // B
  // No C
};

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1002 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}
