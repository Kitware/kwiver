// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test conversion of metadata maps to and from CSV.

#include <arrows/core/metadata_map_io_csv.h>

#include <vital/types/geodesy.h>

#include <tests/test_gtest.h>

namespace kv = kwiver::vital;
using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST( metadata_map_io_csv, save )
{
  // Test data with one of each type
  kv::metadata_map::map_metadata_t map = {
    { 4, { std::make_shared< kv::metadata >() } },
    { 7, { std::make_shared< kv::metadata >(),
           std::make_shared< kv::metadata >() } } };

  map[ 4 ][ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 1 );
  map[ 4 ][ 0 ]->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 1 );
  map[ 4 ][ 0 ]->add< kv::VITAL_META_SENSOR_HORIZONTAL_FOV >( 60.7 );
  map[ 4 ][ 0 ]->add< kv::VITAL_META_PLATFORM_DESIGNATION >( "\"Platform,\"" );
  map[ 4 ][ 0 ]->add< kv::VITAL_META_SENSOR_LOCATION >(
    { kv::vector_2d{ 2.0, 3.0 }, kv::SRID::lat_lon_WGS84 } );
  map[ 4 ][ 0 ]->add< kv::VITAL_META_CORNER_POINTS >(
    { { kv::vector_2d{ 0.0, 3.0 },
        kv::vector_2d{ 2.0, 3.0 },
        kv::vector_2d{ 2.0, 6.0 },
        kv::vector_2d{ 0.0, 6.0 } }, kv::SRID::lat_lon_WGS84 } );

  map[ 7 ][ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 3 );
  map[ 7 ][ 0 ]->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 1 );

  map[ 7 ][ 1 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 5 );
  map[ 7 ][ 1 ]->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 2 );

  // Create CSV writer
  metadata_map_io_csv io;

  // Set write_enum_names so this test still works if the field names change
  auto const config = io.get_configuration();
  config->set_value( "write_enum_names", true );
  io.set_configuration( config );

  // Write to CSV
  std::stringstream ss;
  io.save_( ss, std::make_shared< kv::simple_metadata_map >( map ), "" );

  std::string const expected_result =
    "Frame ID,UNIX_TIMESTAMP,PLATFORM_DESIGNATION,VIDEO_DATA_STREAM_INDEX,"
    "SENSOR_LOCATION.0,SENSOR_LOCATION.1,SENSOR_LOCATION.2,"
    "SENSOR_HORIZONTAL_FOV,"
    "CORNER_POINTS.0,CORNER_POINTS.1,CORNER_POINTS.2,CORNER_POINTS.3,"
    "CORNER_POINTS.4,CORNER_POINTS.5,CORNER_POINTS.6,CORNER_POINTS.7\n"
    "4,1,\"\"\"Platform,\"\"\",1,2,3,0,60.7,0,3,2,3,2,6,0,6\n"
    "7,3,,1,,,,,,,,,,,,\n"
    "7,5,,2,,,,,,,,,,,,\n";
  EXPECT_EQ( expected_result, ss.str() );
}
