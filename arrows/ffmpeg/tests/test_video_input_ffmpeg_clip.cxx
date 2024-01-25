// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the FFmpeg video clip input.

#include <test_gtest.h>

#include <arrows/ffmpeg/tests/common.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/ffmpeg/ffmpeg_video_input_clip.h>

#include <vital/plugin_management/plugin_manager.h>

#include <filesystem>

namespace ffmpeg = kwiver::arrows::ffmpeg;
namespace kv = kwiver::vital;

std::filesystem::path g_data_dir;

namespace {

// ----------------------------------------------------------------------------
void
configure_input(
  ffmpeg::ffmpeg_video_input_clip& input,
  kv::frame_id_t frame_begin, kv::frame_id_t frame_end,
  bool start_at_keyframe )
{
  auto config = input.get_configuration();
  config->set_value( "frame_begin", frame_begin );
  config->set_value( "frame_end", frame_end );
  config->set_value( "start_at_keyframe", start_at_keyframe );
  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );
}

// ----------------------------------------------------------------------------
void test_clipped(
  ffmpeg::ffmpeg_video_input_clip& input,
  std::filesystem::path const& filepath,
  kv::frame_id_t frame_begin, kv::frame_id_t frame_end,
  kv::time_usec_t usec_begin )
{
  ffmpeg::ffmpeg_video_input unclipped_input;
  unclipped_input.open( filepath.string() );
  kv::timestamp ts;
  for( kv::frame_id_t i = 1; i < frame_begin; ++i )
  {
    unclipped_input.next_frame( ts );
  }

  input.open( filepath.string() );
  EXPECT_FALSE( input.good() );
  EXPECT_FALSE( input.end_of_video() );
  ts = input.frame_timestamp();
  EXPECT_EQ( 1, ts.get_frame() );

  CALL_TEST(
    expect_eq_videos,
    unclipped_input, input, 0.0, -frame_begin + 1, -usec_begin, true );

  EXPECT_FALSE( input.good() );
  EXPECT_TRUE( input.end_of_video() );

  if( !unclipped_input.end_of_video() )
  {
    ts = unclipped_input.frame_timestamp();
    EXPECT_EQ( frame_end, ts.get_frame() );
  }

  unclipped_input.close();
  input.close();

  EXPECT_FALSE( input.good() );
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class ffmpeg_video_input_clip : public ::testing::Test
{
public:
  void SetUp() override
  {
    ffmpeg_video_path = data_dir / "videos/ffmpeg_video.mp4";
    aphill_video_path = data_dir / "videos/aphill_short.ts";
  }

  std::filesystem::path ffmpeg_video_path;
  std::filesystem::path aphill_video_path;
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, create )
{
  EXPECT_NE( nullptr, kv::algo::video_input::create( "ffmpeg_clip" ) );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, entire_video_exact_aphill )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 1, 49, false );
  CALL_TEST( test_clipped, input, aphill_video_path, 1, 49, 0 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, entire_video_keyframe_aphill )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 1, 49, true );
  CALL_TEST( test_clipped, input, aphill_video_path, 1, 49, 0 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, entire_video_exact_ffmpeg )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 1, 51, false );
  CALL_TEST( test_clipped, input, ffmpeg_video_path, 1, 51, 0 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, entire_video_keyframe_ffmpeg )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 1, 51, true );
  CALL_TEST( test_clipped, input, ffmpeg_video_path, 1, 51, 0 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, end_past_end )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 1, 100, false );
  CALL_TEST( test_clipped, input, aphill_video_path, 1, 49, 0 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, begin_past_end )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 100, 200, false );
  EXPECT_THROW( input.open( aphill_video_path.string() ), std::runtime_error );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, single_frame )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 20, 21, false );
  CALL_TEST( test_clipped, input, aphill_video_path, 20, 21, 633'966 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, non_keyframe_exact_aphill )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 7, 23, false );
  CALL_TEST( test_clipped, input, aphill_video_path, 7, 23, 200'200 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, non_keyframe_keyframe_aphill )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 7, 23, true );
  CALL_TEST( test_clipped, input, aphill_video_path, 1, 23, 0 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, non_keyframe_exact_ffmpeg )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 7, 23, false );
  CALL_TEST( test_clipped, input, ffmpeg_video_path, 7, 23, 1'200'000 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, non_keyframe_keyframe_ffmpeg )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 7, 23, true );
  CALL_TEST( test_clipped, input, ffmpeg_video_path, 6, 23, 1'000'000 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, keyframe_exact_aphill )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 17, 33, false );
  CALL_TEST( test_clipped, input, aphill_video_path, 17, 33, 533'866 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, keyframe_keyframe_aphill )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 17, 33, true );
  CALL_TEST( test_clipped, input, aphill_video_path, 17, 33, 533'866 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, keyframe_exact_ffmpeg )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 11, 33, false );
  CALL_TEST( test_clipped, input, ffmpeg_video_path, 11, 33, 2'000'000 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_clip, keyframe_keyframe_ffmpeg )
{
  ffmpeg::ffmpeg_video_input_clip input;
  configure_input( input, 11, 33, true );
  CALL_TEST( test_clipped, input, ffmpeg_video_path, 11, 33, 2'000'000 );
}
