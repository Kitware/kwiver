// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 0806 read / write.

#include "data_format.h"

#include <arrows/klv/klv_0903.h>
#include <arrows/klv/klv_0903_algorithm_set.h>
#include <arrows/klv/klv_0903_location_pack.h>
#include <arrows/klv/klv_0903_ontology_set.h>
#include <arrows/klv/klv_0903_vchip_set.h>
#include <arrows/klv/klv_0903_vfeature_set.h>
#include <arrows/klv/klv_0903_vmask_set.h>
#include <arrows/klv/klv_0903_vobject_set.h>
#include <arrows/klv/klv_0903_vtarget_pack.h>
#include <arrows/klv/klv_0903_vtracker_set.h>
#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_series.hpp>

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
test_read_write( klv_value const& expected_result,
                 klv_bytes_t const& input_bytes )
{
  using format_t = klv_0903_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
auto const expected_result = klv_local_set{
  { KLV_0903_PRECISION_TIMESTAMP, uint64_t{ 987654321000000 } },
  { KLV_0903_VMTI_SYSTEM_NAME, std::string{ "DSTO_ADSS_VMTI" } },
  { KLV_0903_VERSION, uint64_t{ 5 } },
  { KLV_0903_NUM_TARGETS_DETECTED, uint64_t{ 28 } },
  { KLV_0903_NUM_TARGETS_REPORTED, uint64_t{ 14 } },
  { KLV_0903_FRAME_NUMBER, uint64_t{ 78000 } },
  { KLV_0903_FRAME_WIDTH, uint64_t{ 1920 } },
  { KLV_0903_FRAME_HEIGHT, uint64_t{ 1080 } },
  { KLV_0903_SOURCE_SENSOR, std::string{ "EO Nose" } },
  { KLV_0903_HORIZONTAL_FOV, kld{ 12.5 } },
  { KLV_0903_VERTICAL_FOV, kld{ 10.0 } },
  { KLV_0903_MIIS_ID, klv_value{} },
  { KLV_0903_VTARGET_SERIES,
    klv_0903_vtarget_series{ {
      klv_0903_vtarget_pack{
        1, klv_local_set{
          { KLV_0903_VTARGET_CENTROID, uint64_t{ 409600 } } } },
      klv_0903_vtarget_pack{
        1234, klv_local_set{
          { KLV_0903_VTARGET_BOUNDARY_TOP_LEFT, uint64_t{ 409600 } },
          { KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT, uint64_t{ 409600 } },
          { KLV_0903_VTARGET_PRIORITY, uint64_t{ 27 } },
          { KLV_0903_VTARGET_CONFIDENCE_LEVEL, uint64_t{ 80 } },
          { KLV_0903_VTARGET_HISTORY, uint64_t{ 2765 } },
          { KLV_0903_VTARGET_PERCENT_PIXELS, uint64_t{ 50 } },
          { KLV_0903_VTARGET_COLOR, uint64_t{ 0xDAA520 } },
          { KLV_0903_VTARGET_INTENSITY, uint64_t{ 13140 } },
          { KLV_0903_VTARGET_LOCATION_OFFSET_LATITUDE, kld{ 10.0 } },
          { KLV_0903_VTARGET_LOCATION_OFFSET_LONGITUDE, kld{ 12.0 } },
          { KLV_0903_VTARGET_LOCATION_ELLIPSOID_HEIGHT, kld{ 10000.0 } },
          { KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LATITUDE_OFFSET, kld{ 10.0 } },
          { KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LONGITUDE_OFFSET, kld{ 10.0 } },
          { KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LATITUDE_OFFSET,
            kld{ 10.0 } },
          { KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LONGITUDE_OFFSET,
            kld{ 10.0 } },
          { KLV_0903_VTARGET_LOCATION, {} },
          { KLV_0903_VTARGET_BOUNDARY_SERIES, {} },
          { KLV_0903_VTARGET_CENTROID_ROW, uint64_t{ 872 } },
          { KLV_0903_VTARGET_CENTROID_COLUMN, uint64_t{ 1137 } },
          { KLV_0903_VTARGET_FPA_INDEX, klv_0903_fpa_index{ 2, 3 } },
          { KLV_0903_VTARGET_ALGORITHM_ID, uint64_t{ 3 } },
          { KLV_0903_VTARGET_VMASK, {} },
          { KLV_0903_VTARGET_VOBJECT, {} },
          { KLV_0903_VTARGET_VFEATURE, {} },
          { KLV_0903_VTARGET_VTRACKER, {} },
          { KLV_0903_VTARGET_VCHIP, {} },
          { KLV_0903_VTARGET_VCHIP_SERIES, {} },
          { KLV_0903_VTARGET_VOBJECT_SERIES, {} }, } } } } },
  { KLV_0903_ALGORITHM_SERIES, klv_0903_algorithm_series{ {
      klv_local_set{
        { KLV_0903_ALGORITHM_ID, uint64_t{ 9 } },
        { KLV_0903_ALGORITHM_NAME, std::string{ "k6_yolo_9000_tracker" } },
        { KLV_0903_ALGORITHM_VERSION, std::string{ "2.6a" } },
        { KLV_0903_ALGORITHM_CLASS, std::string{ "kalmann" } },
        { KLV_0903_ALGORITHM_NUM_FRAMES, uint64_t{ 10 } }, } } } },
  { KLV_0903_ONTOLOGY_SERIES, klv_0903_ontology_series{ {
      klv_local_set{
        { KLV_0903_ONTOLOGY_ID, uint64_t{ 17 } },
        { KLV_0903_ONTOLOGY_PARENT_ID, uint64_t{ 12 } },
        { KLV_0903_ONTOLOGY_URI, std::string{ "URI" } },
        { KLV_0903_ONTOLOGY_CLASS, std::string{ "class" } }, } } } }, };

// ----------------------------------------------------------------------------
auto const input_bytes = klv_bytes_t{
  0x02, 0x08, // KLV_0903_PRECISION_TIMESTAMP
  0x00, 0x03, 0x82, 0x44, 0x30, 0xF6, 0xCE, 0x40,
  0x03, 0x0E, // KLV_0903_VMTI_SYSTEM_NAME
  0x44, 0x53, 0x54, 0x4F, 0x5F, 0x41, 0x44, 0x53, 0x53, 0x5F, 0x56, 0x4D, 0x54,
  0x49,
  0x04, 0x01, // KLV_0903_VERSION
  0x05,
  0x05, 0x01, // KLV_0903_NUM_TARGETS_DETECTED
  0x1C,
  0x06, 0x01, // KLV_0903_NUM_TARGETS_REPORTED
  0x0E,
  0x07, 0x03, // KLV_0903_FRAME_NUMBER
  0x01, 0x30, 0xB0,
  0x08, 0x02, // KLV_0903_FRAME_WIDTH
  0x07, 0x80,
  0x09, 0x02, // KLV_0903_FRAME_HEIGHT
  0x04, 0x38,
  0x0A, 0x07, // KLV_0903_SOURCE_SENSOR
  0x45, 0x4F, 0x20, 0x4E, 0x6F, 0x73, 0x65,
  0x0B, 0x02, // KLV_0903_HORIZONTAL_FOV
  0x06, 0x40,
  0x0C, 0x02, // KLV_0903_VERTICAL_FOV
  0x05, 0x00,
  0x0D, 0x00, // KLV_0903_MIIS_ID
  0x65, 0x6D, // KLV_0903_VTARGET_SERIES

  0x06, 0x01, // Start VTarget Pack
  0x01, 0x03, // KLV_0903_VTARGET_CENTROID
  0x06, 0x40, 0x00,

  0x65, 0x89, 0x52, // Start VTarget Pack
  0x02, 0x03, // KLV_0903_VTARGET_BOUNDARY_TOP_LEFT
  0x06, 0x40, 0x00,
  0x03, 0x03, // KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT
  0x06, 0x40, 0x00,
  0x04, 0x01, // KLV_0903_VTARGET_PRIORITY
  0x1B,
  0x05, 0x01, // KLV_0903_VTARGET_CONFIDENCE_LEVEL
  0x50,
  0x06, 0x02, // KLV_0903_VTARGET_HISTORY
  0x0A, 0xCD,
  0x07, 0x01, // KLV_0903_VTARGET_PERCENT_PIXELS
  0x32,
  0x08, 0x03, // KLV_0903_VTARGET_COLOR
  0xDA, 0xA5, 0x20,
  0x09, 0x02, // KLV_0903_VTARGET_INTENSITY
  0x33, 0x54,
  0x0A, 0x03, // KLV_0903_VTARGET_LOCATION_OFFSET_LATITUDE
  0x3A, 0x66, 0x67,
  0x0B, 0x03, // KLV_0903_VTARGET_LOCATION_OFFSET_LONGITUDE
  0x3E, 0x66, 0x67,
  0x0C, 0x02, // KLV_0903_VTARGET_LOCATION_ELLIPSOID_HEIGHT
  0x2A, 0x94,
  0x0D, 0x03, // KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LATITUDE_OFFSET
  0x3A, 0x66, 0x67,
  0x0E, 0x03, // KLV_0903_VTARGET_BOUNDARY_TOP_LEFT_LONGITUDE_OFFSET
  0x3A, 0x66, 0x67,
  0x0F, 0x03, // KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LATITUDE_OFFSET
  0x3A, 0x66, 0x67,
  0x10, 0x03, // KLV_0903_VTARGET_BOUNDARY_BOTTOM_RIGHT_LONGITUDE_OFFSET
  0x3A, 0x66, 0x67,
  0x11, 0x00, // KLV_0903_VTARGET_LOCATION
  0x12, 0x00, // KLV_0903_VTARGET_BOUNDARY_SERIES
  0x13, 0x02, // KLV_0903_VTARGET_CENTROID_ROW
  0x03, 0x68,
  0x14, 0x02, // KLV_0903_VTARGET_CENTROID_COLUMN
  0x04, 0x71,
  0x15, 0x02, // KLV_0903_VTARGET_FPA_INDEX
  0x02, 0x03,
  0x16, 0x01, // KLV_0903_VTARGET_ALGORITHM_ID
  0x03,
  0x65, 0x00, // KLV_0903_VTARGET_VMASK
  0x66, 0x00, // KLV_0903_VTARGET_VOBJECT
  0x67, 0x00, // KLV_0903_VTARGET_VFEATURE
  0x68, 0x00, // KLV_0903_VTARGET_VTRACKER
  0x69, 0x00, // KLV_0903_VTARGET_VCHIP
  0x6A, 0x00, // KLV_0903_VTARGET_VCHIP_SERIES
  0x6B, 0x00, // KLV_0903_VTARGET_VOBJECT_SERIES

  0x66, 0x2C, // KLV_0903_ALGORITHM_SERIES

  0x2B, // Start Algorithm set
  0x01, 0x01, // KLV_0903_ALGORITHM_ID
  0x09,
  0x02, 0x14, // KLV_0903_ALGORITHM_NAME
  'k', '6', '_', 'y', 'o', 'l', 'o', '_', '9', '0', '0', '0', '_', 't', 'r',
  'a', 'c', 'k', 'e', 'r',
  0x03, 0x04, // KLV_0903_ALGORITHM_VERSION
  '2', '.', '6', 'a',
  0x04, 0x07, // KLV_0903_ALGORITHM_CLASS
  'k', 'a', 'l', 'm', 'a', 'n', 'n',
  0x05, 0x01, // KLV_0903_ALGORITHM_NUM_FRAMES
  0x0A,

  0x67, 0x13, // KLV_0903_ONTOLOGY_SERIES

  0x12, // Start Ontology set
  0x01, 0x01, // KLV_0903_ONTOLOGY_ID
  0x11,
  0x02, 0x01, // KLV_0903_ONTOLOGY_PARENT_ID
  0x0C,
  0x03, 0x03, // KLV_0903_ONTOLOGY_URI
  'U', 'R', 'I',
  0x04, 0x05,
  'c', 'l', 'a', 's', 's', };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_vtarget_location )
{
  auto const expected_result = klv_0903_location_pack{
    -87.984282970428467,
    -115.49705505371094,
    1671.0,
    klv_0903_sigma_pack{ 8.0625, 24.125, 40.1875 },
    klv_0903_rho_pack{ -0.748046875, -0.24609375, 0.255859375 }, };

  auto const input_bytes = klv_bytes_t{
    0x01, 0x02, 0x03, 0x04,
    0x10, 0x20, 0x30, 0x40,
    0x0A, 0x0B,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, };

  using format_t = klv_0903_location_pack_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_LOCATION ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_boundary_series )
{
  auto const expected_result =
    klv_0903_location_series{ {
    klv_0903_location_pack{
      -87.984282970428467,
      -115.49705505371094,
      1671.0,
      klv_0903_sigma_pack{ 8.0625, 24.125, 40.1875 },
      klv_0903_rho_pack{ -0.748046875, -0.24609375, 0.255859375 }, } } };

  auto const input_bytes = klv_bytes_t{
    22,
    0x01, 0x02, 0x03, 0x04,
    0x10, 0x20, 0x30, 0x40,
    0x0A, 0x0B,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, };

  using format_t = klv_0903_location_series_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_BOUNDARY_SERIES ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_vmask )
{
  auto const expected_result = klv_local_set{
    { KLV_0903_VMASK_POLYGON,
      klv_uint_series{ {
        uint64_t{ 14762 },
        uint64_t{ 14783 },
        uint64_t{ 15115 } } } },
    { KLV_0903_VMASK_BITMASK_SERIES,
      klv_0903_pixel_run_series{ {
        klv_0903_pixel_run{ 74, 2 },
        klv_0903_pixel_run{ 89, 4 },
        klv_0903_pixel_run{ 106, 2 } } } } };

  auto const input_bytes = klv_bytes_t{
    0x01, 0x09,
    0x02, 0x39, 0xAA,
    0x02, 0x39, 0xBF,
    0x02, 0x3B, 0x0B,
    0x02, 0x0C,
    0x03, 0x01, 0x4A, 0x02,
    0x03, 0x01, 0x59, 0x04,
    0x03, 0x01, 0x6A, 0x02, };

  using format_t = klv_0903_vmask_local_set_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_VMASK ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_vobject )
{
  auto const expected_result = klv_local_set{
    { KLV_0903_VOBJECT_ONTOLOGY, std::string{ "URI" } },
    { KLV_0903_VOBJECT_ONTOLOGY_CLASS, std::string{ "class" } },
    { KLV_0903_VOBJECT_ONTOLOGY_ID, uint64_t{ 7 } },
    { KLV_0903_VOBJECT_CONFIDENCE, kld{ 32.0 } }, };

  auto const input_bytes = klv_bytes_t{
    0x01, 0x03,
    'U', 'R', 'I',
    0x02, 0x05,
    'c', 'l', 'a', 's', 's',
    0x03, 0x01,
    0x07,
    0x04, 0x01,
    32, };

  using format_t = klv_0903_vobject_local_set_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_VOBJECT ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_vfeature )
{
  auto const expected_result = klv_local_set{
    { KLV_0903_VFEATURE_SCHEMA, std::string{ "A" } },
    { KLV_0903_VFEATURE_SCHEMA_FEATURE, std::string{ "B" } }, };

  auto const input_bytes = klv_bytes_t{
    0x01, 0x01,
    'A',
    0x02, 0x01,
    'B', };

  using format_t = klv_0903_vfeature_local_set_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_VFEATURE ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_vtracker )
{
  auto const expected_result = klv_local_set{
    { KLV_0903_VTRACKER_TRACK_ID, klv_uuid{ {
        0xF8, 0x1D, 0x4F, 0xAE, 0x7D, 0xEC, 0x11, 0xD0,
        0xA7, 0x65, 0x00, 0xA0, 0xC9, 0x1E, 0x6B, 0xF6 } } },
    { KLV_0903_VTRACKER_DETECTION_STATUS, KLV_0903_DETECTION_STATUS_DROPPED },
    { KLV_0903_VTRACKER_START_TIME, uint64_t{ 987654321000000 } },
    { KLV_0903_VTRACKER_END_TIME, uint64_t{ 987654321000000 } },
    { KLV_0903_VTRACKER_BOUNDARY_SERIES, {} },
    { KLV_0903_VTRACKER_ALGORITHM, std::string{ "test" } },
    { KLV_0903_VTRACKER_CONFIDENCE_LEVEL, uint64_t{ 50 } },
    { KLV_0903_VTRACKER_NUM_TRACK_POINTS, uint64_t{ 27 } },
    { KLV_0903_VTRACKER_TRACK_HISTORY_SERIES, {} },
    { KLV_0903_VTRACKER_VELOCITY,
      klv_0903_velocity_pack{ -608.75, -336.75, 208.3125 } },
    { KLV_0903_VTRACKER_ACCELERATION,
      klv_0903_acceleration_pack{ 159.25, 175.25, 208.3125 } },
    { KLV_0903_VTRACKER_ALGORITHM_ID, uint64_t{ 3 } }, };

  auto const input_bytes = klv_bytes_t{
    0x01, 0x10, // KLV_0903_VTRACKER_TRACK_ID
    0xF8, 0x1D, 0x4F, 0xAE, 0x7D, 0xEC, 0x11, 0xD0,
    0xA7, 0x65, 0x00, 0xA0, 0xC9, 0x1E, 0x6B, 0xF6,
    0x02, 0x01, // KLV_0903_VTRACKER_DETECTION_STATUS
    0x02,
    0x03, 0x08, // KLV_0903_VTRACKER_START_TIME
    0x00, 0x03, 0x82, 0x44, 0x30, 0xF6, 0xCE, 0x40,
    0x04, 0x08, // KLV_0903_VTRACKER_END_TIME
    0x00, 0x03, 0x82, 0x44, 0x30, 0xF6, 0xCE, 0x40,
    0x05, 0x00, // KLV_0903_VTRACKER_BOUNDARY_SERIES
    0x06, 0x04, // KLV_0903_VTRACKER_ALGORITHM
    0x74, 0x65, 0x73, 0x74,
    0x07, 0x01, // KLV_0903_VTRACKER_CONFIDENCE_LEVEL
    0x32,
    0x08, 0x01, // KLV_0903_VTRACKER_NUM_TRACK_POINTS
    0x1B,
    0x09, 0x00, // KLV_0903_VTRACKER_TRACK_HISTORY_SERIES
    0x0A, 0x06, // KLV_0903_VTRACKER_VELOCITY
    0x12, 0x34,
    0x23, 0x34,
    0x45, 0x45,
    0x0B, 0x06, // KLV_0903_VTRACKER_ACCELERATION
    0x42, 0x34,
    0x43, 0x34,
    0x45, 0x45,
    0x0C, 0x01, // KLV_0903_ALGORITHM_ID
    0x03, };

  using format_t = klv_0903_vtracker_local_set_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_VTRACKER ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0903_vchip )
{
  auto const expected_result = klv_local_set{
    { KLV_0903_VCHIP_IMAGE_TYPE, std::string{ "jpeg" } },
    { KLV_0903_VCHIP_IMAGE_URI, std::string{ "URI" } },
    { KLV_0903_VCHIP_EMBEDDED_IMAGE, klv_blob{ { 0x01, 0x02, 0x03, 0x04, } } },
  };

  auto const input_bytes = klv_bytes_t{
    0x01, 0x04,
    0x6A, 0x70, 0x65, 0x67,
    0x02, 0x03,
    'U', 'R', 'I',
    0x03, 0x04,
    0x01, 0x02, 0x03, 0x04, };

  using format_t = klv_0903_vchip_local_set_format;
  ASSERT_EQ( typeid( format_t ),
             typeid( klv_0903_vtarget_pack_traits_lookup()
                     .by_tag( KLV_0903_VTARGET_VCHIP ).format() ) );
  test_read_write_format< format_t >( expected_result, input_bytes );
}
