// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1601 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1601.h>

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
  using format_t = klv_1601_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
auto const expected_result = klv_local_set {
  { KLV_1601_VERSION, uint64_t{ 1 } },
  { KLV_1601_ALGORITHM_NAME, std::string{ "ALGO" } },
  { KLV_1601_ALGORITHM_VERSION, std::string{ "1.0" } },
  { KLV_1601_PIXEL_POINTS,
    klv_1303_mdap< uint64_t >{
      { 4, 4 },
      { 133, 128, 97, 69, 31, 91, 122, 129,
        89, 82, 52, 27, 125, 176, 204, 210 } } },
  { KLV_1601_GEOGRAPHIC_POINTS, klv_1303_mdap< double >{
      { 2, 4 },
      { 32.98416f, 32.98417f, 32.98418f, 32.98419f,
        48.08388f, 48.08389f, 48.08390f, 48.08391f } } },
  { KLV_1601_SECOND_IMAGE_NAME, std::string{ "test.img" } },
  { KLV_1601_ALGORITHM_CONFIG_ID,
    klv_uuid{
      0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
      0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99 } },
  { KLV_1601_ELEVATION, klv_1303_mdap< double >{
      { 4 },
      { 1500.0f, 1501.0f, 1500.0f, 1499.0f } } },
  { KLV_1601_PIXEL_SDCC, klv_1303_mdap< double >{
      { 6, 4 },
      { 1.3125, 2.3125, 3.3125, 4.3125,         // Row sigma 1
        1.375, 2.375, 3.375, 4.375,             // Column sigma 1
        -0.921875, -0.90625, -0.890625, -0.875, // Row-column rho 1
        10.4375, 11.4375, 12.4375, 13.4375,     // Row sigma 2
        10.5, 11.5, 12.5, 13.5,                 // Column sigma 2
        -0.671875, -0.65625, -0.640625, -0.625, // Row-column sigma 2
      } } },
  { KLV_1601_GEOGRAPHIC_SDCC, klv_1303_mdap< double >{
      { 6, 4 },
      { 0.0, 32.0, 128.0, 160.0,     // Latitude sigma
        128.0, 160.0, 0.0, 32.0,     // Longitude sigma
        -0.5, -0.4375, 0.25, 0.3125, // Latitude-longitude rho
        384.0, 416.0, 512.0, 544.0,  // Elevation sigma
        0, 0.0625, -0.25, -0.1875,   // Latitude-elevation rho
        0.25, 0.3125, -0.5, -0.4375  // Longitude-elevation rho
      } } }, };

// ----------------------------------------------------------------------------
auto const input_bytes = klv_bytes_t{
  0x01, 0x01, // KLV_1601_VERSION
  0x01,
  0x02, 0x04, // KLV_1601_ALGORITHM_NAME
  'A', 'L', 'G', 'O',
  0x03, 0x03, // KLV_1601_ALGORITHM_VERSION
  '1', '.', '0',
  0x04, 0x15, // KLV_1601_PIXEL_POINTS
  0x02, 0x04, 0x04, 0x01, 0x01, // Header
  0x85, 0x80, 0x61, 0x45,
  0x1F, 0x5B, 0x7A, 0x81,
  0x59, 0x52, 0x34, 0x1B,
  0x7D, 0xB0, 0xCC, 0xD2,
  0x05, 0x25, // KLV_1601_GEOGRAPHIC_POINTS
  0x02, 0x02, 0x04, 0x04, 0x01, // Header
  0x42, 0x03, 0xEF, 0xC8, 0x42, 0x03, 0xEF, 0xCA,
  0x42, 0x03, 0xEF, 0xCD, 0x42, 0x03, 0xEF, 0xD0,
  0x42, 0x40, 0x55, 0xE5, 0x42, 0x40, 0x55, 0xE7,
  0x42, 0x40, 0x55, 0xEA, 0x42, 0x40, 0x55, 0xED,
  0x06, 0x08, // KLV_1601_SECOND_IMAGE_NAME
  't', 'e', 's', 't', '.', 'i', 'm', 'g',
  0x07, 0x10, // KLV_1601_ALGORITHM_CONFIG_ID
  0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
  0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
  0x08, 0x18, // KLV_1601_ELEVATION
  0x01, 0x04, 0x03, 0x02, // Header
  0xC4, 0x61, 0x00, 0x00, 0x46, 0x94, 0x70, 0x00, // IMAP params
  0x09, 0x60, 0x00, 0x09, 0x61, 0x00,
  0x09, 0x60, 0x00, 0x09, 0x5F, 0x00,
  0x09, 0x35, // KLV_1601_PIXEL_SDCC
  0x02, 0x06, 0x04, 0x02, 0x01, // Header
  0x01, 0x50, 0x02, 0x50, 0x03, 0x50, 0x04, 0x50, // Row sigma 1
  0x01, 0x60, 0x02, 0x60, 0x03, 0x60, 0x04, 0x60, // Column sigma 1
  0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00, // Row-column rho 1
  0x0A, 0x70, 0x0B, 0x70, 0x0C, 0x70, 0x0D, 0x70, // Row sigma 2
  0x0A, 0x80, 0x0B, 0x80, 0x0C, 0x80, 0x0D, 0x80, // Column sigma 2
  0x15, 0x00, 0x16, 0x00, 0x17, 0x00, 0x18, 0x00, // Row-column sigma 2
  0x0A, 0x1D, // KLV_1601_GEOGRAPHIC_SDCC
  0x02, 0x06, 0x04, 0x01, 0x01, // Header
  0x00, 0x04, 0x10, 0x14, // Latitude sigma
  0x10, 0x14, 0x00, 0x04, // Longitude sigma
  0x20, 0x24, 0x50, 0x54, // Latitude-longitude rho
  0x30, 0x34, 0x40, 0x44, // Elevation sigma
  0x40, 0x44, 0x30, 0x34, // Latitude-elevation rho
  0x50, 0x54, 0x20, 0x24, // Longitude-elevation rho
};

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1601 )
{
  CALL_TEST( test_read_write, expected_result, input_bytes );
}
