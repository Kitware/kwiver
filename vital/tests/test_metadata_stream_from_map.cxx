// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Tests for the metadata_[io]stream_from_map classes.

#include <vital/types/metadata_stream_from_map.h>

#include <tests/test_gtest.h>

using namespace kwiver::vital;

namespace {

// ----------------------------------------------------------------------------
void
test_istream_frame(
  metadata_istream& is, frame_id_t frame, metadata_vector const& md )
{
  ASSERT_FALSE( is.at_end() );
  EXPECT_EQ( frame, is.frame_number() );
  EXPECT_EQ( md, is.metadata() );
}

// ----------------------------------------------------------------------------
void
test_istream_at_end( metadata_istream& is )
{
  // Repeat in case next_frame() changes state
  for( size_t i = 0; i < 2; ++i )
  {
    ASSERT_TRUE( is.at_end() );
    EXPECT_THROW( is.frame_number(), std::invalid_argument );
    EXPECT_THROW( is.metadata(), std::invalid_argument );
    EXPECT_FALSE( is.next_frame() );
  }
}

// ----------------------------------------------------------------------------
void
test_ostream_frame(
  metadata_ostream& os, frame_id_t frame, metadata_vector const& md )
{
  ASSERT_FALSE( os.at_end() );
  ASSERT_NO_THROW( ASSERT_TRUE( os.write_frame( frame, md ) ) );
}

// ----------------------------------------------------------------------------
void
test_ostream_at_end( metadata_ostream& os )
{
  ASSERT_TRUE( os.at_end() );
  EXPECT_THROW( os.write_frame( 1024, {} ), std::invalid_argument );
  ASSERT_TRUE( os.at_end() );
  EXPECT_NO_THROW( os.write_end() );
  ASSERT_TRUE( os.at_end() );
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( metadata_stream_from_map, istream_empty )
{
  metadata_istream_from_map::map_t map;
  metadata_istream_from_map is{ map };

  CALL_TEST( test_istream_at_end, is );
}

// ----------------------------------------------------------------------------
TEST ( metadata_stream_from_map, istream )
{
  auto const md = std::make_shared< metadata >();
  md->add< VITAL_META_UNIX_TIMESTAMP >( 5 );
  metadata_istream_from_map::map_t map{
    { 1, { md } },
    { 3, {} }, };
  metadata_istream_from_map is{ map };

  CALL_TEST( test_istream_frame, is, 1, { md } );
  ASSERT_TRUE( is.next_frame() );
  CALL_TEST( test_istream_frame, is, 3, {} );
  ASSERT_FALSE( is.next_frame() );

  CALL_TEST( test_istream_at_end, is );
}

// ----------------------------------------------------------------------------
TEST ( metadata_stream_from_map, ostream )
{
  auto const md = std::make_shared< metadata >();
  md->add< VITAL_META_UNIX_TIMESTAMP >( 5 );
  metadata_ostream_from_map::map_t map;
  metadata_ostream_from_map os{ map };

  CALL_TEST( test_ostream_frame, os, 1, { md } );
  CALL_TEST( test_ostream_frame, os, 3, { nullptr } );
  CALL_TEST( test_ostream_frame, os, 1, { nullptr } );
  CALL_TEST( test_ostream_frame, os, 1, {} );
  CALL_TEST( test_ostream_frame, os, 5, { md, md } );
  CALL_TEST( test_ostream_frame, os, 6, {} );

  os.write_end();
  CALL_TEST( test_ostream_at_end, os );

  metadata_ostream_from_map::map_t expected_map{
    { 1, { md, nullptr } },
    { 3, { nullptr } },
    { 5, { md, md } },
    { 6, {} }, };
  EXPECT_EQ( expected_map, map );
}
