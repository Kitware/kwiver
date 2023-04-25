// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test merge_metadata_streams filter.

#include <arrows/core/merge_metadata_streams.h>

#include <gtest/gtest.h>

using namespace kwiver;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST( merge_metadata_streams, merge_empty )
{
  arrows::core::merge_metadata_streams filter;
  auto const result = filter.filter( {}, nullptr );
  ASSERT_EQ( 1, result.size() );
  ASSERT_NE( nullptr, result[ 0 ] );
  ASSERT_TRUE( result[ 0 ]->empty() );
}

// ----------------------------------------------------------------------------
TEST( merge_metadata_streams, merge_null )
{
  arrows::core::merge_metadata_streams filter;
  auto const result = filter.filter( { nullptr, nullptr }, nullptr );
  ASSERT_EQ( 1, result.size() );
  ASSERT_NE( nullptr, result[ 0 ] );
  ASSERT_TRUE( result[ 0 ]->empty() );
}

// ----------------------------------------------------------------------------
TEST( merge_metadata_streams, merge_one )
{
  arrows::core::merge_metadata_streams filter;
  auto const md = std::make_shared< vital::metadata >();
  md->add< vital::VITAL_META_UNIX_TIMESTAMP >( 7 );
  auto const result = filter.filter( { md }, nullptr );
  ASSERT_EQ( 1, result.size() );
  ASSERT_NE( nullptr, result[ 0 ] );
  ASSERT_EQ( *md, *result[ 0 ] );
}

// ----------------------------------------------------------------------------
TEST( merge_metadata_streams, merge_multiple )
{
  arrows::core::merge_metadata_streams filter;
  vital::metadata_vector md = {
    std::make_shared< vital::metadata >(),
    std::make_shared< vital::metadata >(),
    std::make_shared< vital::metadata >()
  };
  md[ 0 ]->add< vital::VITAL_META_UNIX_TIMESTAMP >( 0 );
  md[ 0 ]->add< vital::VITAL_META_MISSION_ID >( "0" );
  md[ 0 ]->add< vital::VITAL_META_MISSION_NUMBER >( "#" );
  md[ 1 ]->add< vital::VITAL_META_UNIX_TIMESTAMP >( 1 );
  md[ 1 ]->add< vital::VITAL_META_MISSION_ID >( "1" );
  md[ 1 ]->add< vital::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 1 );
  md[ 1 ]->add< vital::VITAL_META_VIDEO_DATA_STREAM_SYNCHRONOUS >( false );
  md[ 2 ]->add< vital::VITAL_META_UNIX_TIMESTAMP >( 2 );
  md[ 2 ]->add< vital::VITAL_META_VIDEO_DATA_STREAM_INDEX >( 2 );
  md[ 2 ]->add< vital::VITAL_META_VIDEO_DATA_STREAM_SYNCHRONOUS >( true );

  auto const result = filter.filter( md, nullptr );
  ASSERT_EQ( 1, result.size() );
  ASSERT_NE( nullptr, result[ 0 ] );
  ASSERT_EQ( 3, result[ 0 ]->size() );
  ASSERT_EQ(
    2, result[ 0 ]->find( vital::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
  ASSERT_EQ(
    "1", result[ 0 ]->find( vital::VITAL_META_MISSION_ID ).as_string() );
  ASSERT_EQ(
    "#", result[ 0 ]->find( vital::VITAL_META_MISSION_NUMBER ).as_string() );
}
