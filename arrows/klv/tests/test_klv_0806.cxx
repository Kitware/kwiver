// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test KLV 0806 read / write.

#include "data_format.h"

#include <arrows/klv/klv_0806.h>
#include <arrows/klv/klv_0806_aoi_set.h>
#include <arrows/klv/klv_0806_poi_set.h>
#include <arrows/klv/klv_0806_user_defined_set.h>
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
  using format_t = klv_0806_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
using kld = klv_lengthy< double >;
auto const expected_result = klv_local_set{
  { KLV_0806_TIMESTAMP,
    uint64_t{ 256 } },
  { KLV_0806_PLATFORM_TRUE_AIRSPEED,
    uint64_t{ 500 } },
  { KLV_0806_PLATFORM_INDICATED_AIRSPEED,
    uint64_t{ 400 } },
  { KLV_0806_TELEMETRY_ACCURACY_INDICATOR,
    klv_blob{ 0xAB } },
  { KLV_0806_FRAG_CIRCLE_RADIUS,
    uint64_t{ 30 } },
  { KLV_0806_FRAME_CODE,
    uint64_t{ 20 } },
  { KLV_0806_VERSION_NUMBER,
    uint64_t{ 4 } },
  { KLV_0806_VIDEO_DATA_RATE,
    uint64_t{ 2048 } },
  { KLV_0806_DIGITAL_VIDEO_FILE_FORMAT,
    std::string{ "MPEG2" } },
  { KLV_0806_USER_DEFINED_LOCAL_SET,
    klv_local_set{
      { KLV_0806_USER_DEFINED_SET_DATA_TYPE_ID,
        klv_0806_user_defined_data_type_id{
          KLV_0806_USER_DEFINED_SET_DATA_TYPE_STRING, 1 } },
      { KLV_0806_USER_DEFINED_SET_DATA,
        klv_0806_user_defined_data{ { 'T', 'E', 'S', 'T' } } }, } },
  { KLV_0806_POI_LOCAL_SET,
    klv_local_set{
      { KLV_0806_POI_SET_NUMBER,
        uint64_t{ 2 } },
      { KLV_0806_POI_SET_LATITUDE,
        kld{ 0.0 } },
      { KLV_0806_POI_SET_LONGITUDE,
        kld{ -119.52758079372699740 } },
      { KLV_0806_POI_SET_ALTITUDE,
        kld{ 515.03013656824578 } },
      { KLV_0806_POI_SET_TYPE,
        KLV_0806_POI_AOI_TYPE_TARGET },
      { KLV_0806_POI_SET_TEXT,
        std::string{ "TEST" } },
      { KLV_0806_POI_SET_SOURCE_ICON,
        std::string{ "icon7" } },
      { KLV_0806_POI_SET_SOURCE_ID,
        std::string{ "#5" } },
      { KLV_0806_POI_SET_LABEL,
        std::string{ "test" } },
      { KLV_0806_POI_SET_OPERATION_ID,
        std::string{ "Test" } } } },
  { KLV_0806_AOI_LOCAL_SET,
    klv_local_set{
      { KLV_0806_AOI_SET_NUMBER,
        uint64_t{ 3 } },
      { KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_1,
        kld{ 12.65955448740141165 } },
      { KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_1,
        kld{ 25.32460213886788125 } },
      { KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_3,
        kld{ 12.66504765146646960 } },
      { KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_3,
        kld{ 25.33558846699799716 } },
      { KLV_0806_AOI_SET_TYPE,
        KLV_0806_POI_AOI_TYPE_FRIENDLY },
      { KLV_0806_AOI_SET_TEXT,
        std::string{ "1" } },
      { KLV_0806_AOI_SET_SOURCE_ID,
        std::string{ "2" } },
      { KLV_0806_AOI_SET_LABEL,
        std::string{ "3" } },
      { KLV_0806_AOI_SET_OPERATION_ID,
        std::string{ "4" } }, } },
  { KLV_0806_MGRS_ZONE,
    uint64_t{ 5 } },
  { KLV_0806_MGRS_LATITUDE_BAND_GRID_SQUARE,
    std::string{ "ABC" } },
  { KLV_0806_MGRS_EASTING,
    uint64_t{ 1024 } },
  { KLV_0806_MGRS_NORTHING,
    uint64_t{ 2048 } },
  { KLV_0806_FRAME_CENTER_MGRS_ZONE,
    uint64_t{ 6 } },
  { KLV_0806_FRAME_CENTER_MGRS_LATITUDE_BAND_GRID_SQUARE,
    std::string{ "XYZ" } },
  { KLV_0806_FRAME_CENTER_MGRS_EASTING,
    uint64_t{ 100 } },
  { KLV_0806_FRAME_CENTER_MGRS_NORTHING,
    uint64_t{ 200 } }, };

// ----------------------------------------------------------------------------
auto const input_bytes = klv_bytes_t{
  KLV_0806_TIMESTAMP, 8,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  KLV_0806_PLATFORM_TRUE_AIRSPEED, 2,
  0x01, 0xF4,
  KLV_0806_PLATFORM_INDICATED_AIRSPEED, 2,
  0x01, 0x90,
  KLV_0806_TELEMETRY_ACCURACY_INDICATOR, 1,
  0xAB,
  KLV_0806_FRAG_CIRCLE_RADIUS, 2,
  0x00, 30,
  KLV_0806_FRAME_CODE, 4,
  0x00, 0x00, 0x00, 20,
  KLV_0806_VERSION_NUMBER, 1,
  4,
  KLV_0806_VIDEO_DATA_RATE, 4,
  0x00, 0x00, 0x08, 0x00,
  KLV_0806_DIGITAL_VIDEO_FILE_FORMAT, 5,
  'M', 'P', 'E', 'G', '2',

  KLV_0806_USER_DEFINED_LOCAL_SET, 9,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_ID, 1,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_STRING << 6 | 1,
    KLV_0806_USER_DEFINED_SET_DATA, 4,
    'T', 'E', 'S', 'T',

    KLV_0806_POI_LOCAL_SET, 52,
    KLV_0806_POI_SET_NUMBER, 2,
    0x00, 0x02,
    KLV_0806_POI_SET_LATITUDE, 4,
    0x00, 0x00, 0x00, 0x00,
    KLV_0806_POI_SET_LONGITUDE, 4,
    0xAB, 0x00, 0xAB, 0x00,
    KLV_0806_POI_SET_ALTITUDE, 2,
    0x12, 0x34,
    KLV_0806_POI_SET_TYPE, 1,
    KLV_0806_POI_AOI_TYPE_TARGET,
    KLV_0806_POI_SET_TEXT, 4,
    'T', 'E', 'S', 'T',
    KLV_0806_POI_SET_SOURCE_ICON, 5,
    'i', 'c', 'o', 'n', '7',
    KLV_0806_POI_SET_SOURCE_ID, 2,
    '#', '5',
    KLV_0806_POI_SET_LABEL, 4,
    't', 'e', 's', 't',
    KLV_0806_POI_SET_OPERATION_ID, 4,
    'T', 'e', 's', 't',

    KLV_0806_AOI_LOCAL_SET, 43,
    KLV_0806_AOI_SET_NUMBER, 2,
    0x00, 0x03,
    KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_1, 4,
    0x12, 0x01, 0x34, 0x00,
    KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_1, 4,
    0x12, 0x02, 0x34, 0x00,
    KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_3, 4,
    0x12, 0x03, 0x34, 0x00,
    KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_3, 4,
    0x12, 0x04, 0x34, 0x00,
    KLV_0806_AOI_SET_TYPE, 1,
    KLV_0806_POI_AOI_TYPE_FRIENDLY,
    KLV_0806_AOI_SET_TEXT, 1,
    '1',
    KLV_0806_AOI_SET_SOURCE_ID, 1,
    '2',
    KLV_0806_AOI_SET_LABEL, 1,
    '3',
    KLV_0806_AOI_SET_OPERATION_ID, 1,
    '4',

    KLV_0806_MGRS_ZONE, 1,
    5,
    KLV_0806_MGRS_LATITUDE_BAND_GRID_SQUARE, 3,
    'A', 'B', 'C',
    KLV_0806_MGRS_EASTING, 3,
    0x00, 0x04, 0x00,
    KLV_0806_MGRS_NORTHING, 3,
    0x00, 0x08, 0x00,
    KLV_0806_FRAME_CENTER_MGRS_ZONE, 1,
    6,
    KLV_0806_FRAME_CENTER_MGRS_LATITUDE_BAND_GRID_SQUARE, 3,
    'X', 'Y', 'Z',
    KLV_0806_FRAME_CENTER_MGRS_EASTING, 3,
    0x00, 0x00, 0x64,
    KLV_0806_FRAME_CENTER_MGRS_NORTHING, 3,
    0x00, 0x00, 0xC8, };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0806 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}
