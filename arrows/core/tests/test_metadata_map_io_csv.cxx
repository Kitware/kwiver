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
class metadata_map_csv : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Set write_enum_names so this test still works if the field names change
    kv::config_block_sptr config = kv::config_block::empty_config();
    io.get_default_config(*config);
    config->set_value( "write_enum_names", true );
    io.set_configuration( config );

    map = {
      { 4, { std::make_shared< kv::metadata >() } },
      { 7, { std::make_shared< kv::metadata >(),
            std::make_shared< kv::metadata >() } } };

    map[ 4 ][ 0 ]->add< kv::VITAL_META_VIDEO_FRAME_NUMBER >( 4 );
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

    map[ 7 ][ 0 ]->add< kv::VITAL_META_VIDEO_FRAME_NUMBER >( 7 );
    map[ 7 ][ 0 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 3 );
    map[ 7 ][ 0 ]->add< kv::VITAL_META_VIDEO_MICROSECONDS >( 123456789012 );
    map[ 7 ][ 0 ]->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 1 );

    map[ 7 ][ 1 ]->add< kv::VITAL_META_VIDEO_FRAME_NUMBER >( 7 );
    map[ 7 ][ 1 ]->add< kv::VITAL_META_UNIX_TIMESTAMP >( 5 );
    map[ 7 ][ 1 ]->add< kv::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 2 );

    example_csv =
      "Frame ID,UNIX_TIMESTAMP,PLATFORM_DESIGNATION,VIDEO_DATA_STREAM_INDEX,"
      "VIDEO_MICROSECONDS,"
      "SENSOR_LOCATION.0,SENSOR_LOCATION.1,SENSOR_LOCATION.2,"
      "SENSOR_HORIZONTAL_FOV,"
      "CORNER_POINTS.0,CORNER_POINTS.1,CORNER_POINTS.2,CORNER_POINTS.3,"
      "CORNER_POINTS.4,CORNER_POINTS.5,CORNER_POINTS.6,CORNER_POINTS.7\n"
      "4,1,\"\"\"Platform,\"\"\",1,,2,3,0,60.7,0,3,2,3,2,6,0,6\n"
      "7,3,,1,34:17:36.789012,,,,,,,,,,,,\n"
      "7,5,,2,,,,,,,,,,,,,\n";
  }

  metadata_map_io_csv io;
  kv::metadata_map::map_metadata_t map;
  std::stringstream ss;
  std::string example_csv;
};

// ----------------------------------------------------------------------------
TEST_F( metadata_map_csv, save )
{
  // Write to CSV
  io.save_( ss, std::make_shared< kv::simple_metadata_map >( map ), "" );

  EXPECT_EQ( example_csv, ss.str() );
}

// ----------------------------------------------------------------------------
TEST_F( metadata_map_csv, load )
{
  // Read from CSV
  ss.str( example_csv );
  auto const result_map = io.load_( ss, "" )->metadata();

  auto true_it = map.cbegin();
  auto result_it = result_map.cbegin();
  while( true_it != map.cend() && result_it != result_map.cend() )
  {
    EXPECT_EQ( true_it->first, result_it->first );
    EXPECT_TRUE( std::equal(
      true_it->second.begin(), true_it->second.end(),
      result_it->second.begin(), result_it->second.end(),
      []( auto const& lhs, auto const& rhs ){ return *lhs == *rhs; } ) )
      << "Frame " << true_it->first << " not equal";
    ++true_it;
    ++result_it;
  }

  EXPECT_EQ( true_it, map.cend() );
  EXPECT_EQ( result_it, result_map.cend() );
}
