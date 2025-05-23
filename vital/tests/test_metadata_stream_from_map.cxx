// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Tests for the metadata_[io]stream_from_map classes.

#include <vital/tests/test_metadata_stream.h>
#include <vital/types/metadata_stream_from_map.h>

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
