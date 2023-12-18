// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Tests for the metadata_[io]stream_from_map classes.

#include <arrows/core/metadata_stream_from_video.h>

#include <vital/tests/test_metadata_stream.h>

using namespace kwiver::arrows::core;

namespace {

// ----------------------------------------------------------------------------
class mock_video_input : public algo::video_input
{
public:
  mock_video_input( metadata_map::map_metadata_t const& map )
    : m_metadata{ map }, m_it{ m_metadata.begin() }, m_good{ false }
  {}

  void
  open( std::string ) override {}

  void
  close() override { m_good = false; }

  bool
  end_of_video() const override { return m_it == m_metadata.end(); }

  bool
  good() const override { return m_good; }

  bool
  seekable() const override { return false; }

  size_t
  num_frames() const override { return 0; }

  bool
  next_frame( timestamp& ts, uint32_t = 0 ) override
  {
    if( end_of_video() )
    {
      return false;
    }

    if( !m_good )
    {
      m_good = true;
    }
    else
    {
      ++m_it;
    }
    ts.set_invalid();
    if( end_of_video() )
    {
      m_good = false;
      return false;
    }
    ts.set_frame( m_it->first );
    return true;
  }

  bool
  seek_frame( timestamp&, frame_id_t, uint32_t = 0 ) override
  {
    return false;
  }

  timestamp
  frame_timestamp() const
  {
    timestamp ts;
    if( good() )
    {
      ts.set_frame( m_it->first );
    }
    return ts;
  }

  image_container_sptr
  frame_image() override { return nullptr; }

  metadata_vector
  frame_metadata() override
  {
    return good() ? m_it->second : metadata_vector{};
  }

  metadata_map_sptr
  metadata_map() override { return nullptr; }

  void
  set_configuration( config_block_sptr ) override {}

  config_block_sptr
  get_configuration() const override {
    auto cb = config_block::empty_config();
    return cb;
  }

  bool
  check_configuration( config_block_sptr ) const override { return true; }

private:
  metadata_map::map_metadata_t m_metadata;
  metadata_map::map_metadata_t::const_iterator m_it;
  bool m_good;
};

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( metadata_stream_from_video, istream_empty )
{
  metadata_map::map_metadata_t map;
  mock_video_input video{ map };
  video.open( "" );
  metadata_istream_from_video is{ video };

  CALL_TEST( test_istream_at_end, is );
}

// ----------------------------------------------------------------------------
TEST ( metadata_stream_from_video, istream )
{
  auto const md = std::make_shared< metadata >();
  md->add< VITAL_META_UNIX_TIMESTAMP >( 5 );
  metadata_map::map_metadata_t map{
    { 1, { md } },
    { 4, { nullptr, md, md } } };
  mock_video_input video{ map };
  video.open( "" );
  metadata_istream_from_video is{ video };

  CALL_TEST( test_istream_frame, is, 1, { md } );
  ASSERT_TRUE( is.next_frame() );
  CALL_TEST( test_istream_frame, is, 4, { nullptr, md, md } );
  ASSERT_FALSE( is.next_frame() );

  CALL_TEST( test_istream_at_end, is );
}
