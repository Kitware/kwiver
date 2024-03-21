// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the ffmpeg_video_input_rewire class.

#include <tests/test_gtest.h>

#include <arrows/ffmpeg/tests/common.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/ffmpeg/ffmpeg_video_input_rewire.h>

#include <arrows/klv/klv_metadata.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <filesystem>

namespace ffmpeg = kwiver::arrows::ffmpeg;
namespace klv = kwiver::arrows::klv;
namespace kv = kwiver::vital;

std::filesystem::path g_data_dir;

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
TEST( ffmpeg_video_input_rewire, create )
{
  EXPECT_NE( nullptr, kv::algo::video_input::create( "ffmpeg_rewire" ) );
}

// ----------------------------------------------------------------------------
TEST( ffmpeg_video_input_rewire, video_only )
{
  auto const path = ( g_data_dir / "videos/aphill_short.ts" ).string();

  // Configure single input
  ffmpeg::ffmpeg_video_input_rewire input;
  auto config = input.get_configuration();
  config->set_value( "source-0:type", "video" );
  config->set_value( "source-0:filename", path );
  config->set_value( "source-0:input:type", "ffmpeg" );
  config->set_value( "streams", "" );
  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );
  input.open( "" );

  // Open original video directly
  ffmpeg::ffmpeg_video_input check_input;
  check_input.open( path );

  kv::timestamp check_ts;
  kv::timestamp ts;

  // Loop through rewired and original video together
  for( check_input.next_frame( check_ts ), input.next_frame( ts );
       !check_input.end_of_video() && !input.end_of_video();
       check_input.next_frame( check_ts ), input.next_frame( ts ) )
  {
    SCOPED_TRACE(
      std::string{ "Frame: " } +
      std::to_string( check_ts.get_frame() ) + " | " +
      std::to_string( ts.get_frame() ) );

    // Timestamps should be the same
    EXPECT_EQ( check_ts.get_frame(), ts.get_frame() );
    EXPECT_EQ( check_ts.get_time_usec(), ts.get_time_usec() );

    // Metadata should be empty
    auto const metadata = input.frame_metadata();
    EXPECT_EQ( 0, metadata.size() );

    // Images should be identical
    auto const check_image = check_input.frame_image()->get_image();
    auto const image = input.frame_image()->get_image();
    expect_eq_images( check_image, image, 0.0 );
  }

  // Videos should have the same number of frames
  EXPECT_TRUE( check_input.end_of_video() );
  EXPECT_TRUE( input.end_of_video() );

  check_input.close();
  input.close();
}

// ----------------------------------------------------------------------------
TEST( ffmpeg_video_input_rewire, metadata )
{
  std::vector< std::string > paths = {
    ( g_data_dir / "videos/aphill_short.ts" ).string(),
    ( g_data_dir / "videos/h265_tricky_klv.ts" ).string(),
    ( g_data_dir / "videos/h264_no_klv.ts" ).string(),
  };
  auto const n = paths.size();

  // Configure input to draw from three different videos
  ffmpeg::ffmpeg_video_input_rewire input;
  auto config = input.get_configuration();
  for( size_t i = 0; i < n; ++i )
  {
    auto const prefix = "source-" + std::to_string( i ) + ":";
    config->set_value( prefix + "type", "video" );
    config->set_value( prefix + "filename", paths[ i ] );
    config->set_value( prefix + "input:type", "ffmpeg" );
  }
  config->set_value( "streams", "2/unmarked,0/1,1/1,1/3" );
  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );
  input.open( "" );

  // Open all original videos directly
  std::vector< kv::timestamp > check_tss;
  std::vector< std::shared_ptr< ffmpeg::ffmpeg_video_input > > check_inputs;
  for( size_t i = 0; i < n; ++i )
  {
    check_inputs.emplace_back( new ffmpeg::ffmpeg_video_input );
    check_inputs.back()->open( paths[ i ] );
    check_tss.emplace_back();
  }

  kv::timestamp ts;

  // Loop through rewired and original videos together
  for( input.next_frame( ts ); !input.end_of_video(); input.next_frame( ts ) )
  {
    for( size_t i = 0; i < n; ++i )
    {
      check_inputs[ i ]->next_frame( check_tss[ i ] );
    }

    SCOPED_TRACE(
      std::string{ "Frame: " } +
      std::to_string( ts.get_frame() ) );

    // Number of frames must be the same as the first video stream
    ASSERT_FALSE( check_inputs[ 0 ]->end_of_video() );

    // Timestamps must be the same as the first video stream
    EXPECT_EQ( check_tss[ 0 ].get_frame(), ts.get_frame() );
    EXPECT_EQ( check_tss[ 0 ].get_time_usec(), ts.get_time_usec() );

    auto const metadata = input.frame_metadata();
    if( ts.get_frame() <= 30 ) // Videos 1 and 2 are only 30 frames long
    {
      // Check that metadata came frame each input as expected
      EXPECT_EQ( 4, metadata.size() );
      for( size_t i = 0; i < 4; ++i )
      {
        auto const& entry =
          metadata.at( i )->find( kv::VITAL_META_VIDEO_DATA_STREAM_INDEX );
        EXPECT_TRUE( entry );
        EXPECT_EQ( i + 1, entry.get< int >() );
      }
    }
    else
    {
      // Check that metadata came from the one remaining input as expected
      EXPECT_EQ( 1, metadata.size() );
      auto const& entry =
        metadata.at( 0 )->find( kv::VITAL_META_VIDEO_DATA_STREAM_INDEX );
      EXPECT_TRUE( entry );
      EXPECT_EQ( 2, entry.get< int >() );
    }

    for( auto const& md : metadata )
    {
      auto const index =
        md->find( kv::VITAL_META_VIDEO_DATA_STREAM_INDEX ).get< int >();

      if( index == 1 )
      {
        // Stream 1 is from the no-KLV video
        EXPECT_EQ(
          nullptr, dynamic_cast< klv::klv_metadata const* >( md.get() ) );
      }
      else
      {
        // Check that the remaining streams have the appropriate KLV data
        std::map< int, std::pair< size_t, size_t > > mapping{
          { 2, { 0, 1 } },
          { 3, { 1, 1 } },
          { 4, { 1, 3 } },
        };
        auto [input_index, stream_index] = mapping.at( index );
        auto const klv_md =
          dynamic_cast< klv::klv_metadata const* >( md.get() );
        auto const check_klv_md =
          dynamic_cast< klv::klv_metadata const* >(
            check_inputs.at( input_index )->frame_metadata()
            .at( stream_index - 1 ).get() );
        ASSERT_NE( nullptr, klv_md );
        ASSERT_NE( nullptr, check_klv_md );
        EXPECT_EQ( check_klv_md->klv(), klv_md->klv() );
      }
    }

    // Images must be identical
    auto const check_image = check_inputs[ 0 ]->frame_image()->get_image();
    auto const image = input.frame_image()->get_image();
    expect_eq_images( check_image, image, 0.0 );
  }

  // Rewired video must have the same number of frames as the first video input
  check_inputs[ 0 ]->next_frame( check_tss[ 0 ] );
  EXPECT_TRUE( check_inputs[ 0 ]->end_of_video() );

  for( size_t i = 0; i < n; ++i )
  {
    check_inputs[ i ]->close();
  }
  input.close();
}

// ----------------------------------------------------------------------------
TEST( ffmpeg_video_input_rewire, audio )
{
  auto const video_path = ( g_data_dir / "videos/aphill_short.ts" ).string();
  auto const audio_path = ( g_data_dir / "videos/h264_audio.ts" ).string();

  // Configure audio- and non-audio inputs
  ffmpeg::ffmpeg_video_input_rewire input;
  auto config = input.get_configuration();
  config->set_value( "source-0:type", "video" );
  config->set_value( "source-0:filename", video_path );
  config->set_value( "source-0:input:type", "ffmpeg" );
  config->set_value( "source-1:type", "video" );
  config->set_value( "source-1:filename", audio_path );
  config->set_value( "source-1:input:type", "ffmpeg" );
  config->set_value( "streams", "1/1" );
  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );
  input.open( "" );

  // Open source videos directly
  ffmpeg::ffmpeg_video_input video_input;
  ffmpeg::ffmpeg_video_input audio_input;
  video_input.open( video_path );
  audio_input.open( audio_path );

  kv::timestamp video_ts;
  kv::timestamp audio_ts;
  kv::timestamp ts;

  // Run through rewired and original videos together
  for( video_input.next_frame( video_ts ),
       audio_input.next_frame( audio_ts ),
       input.next_frame( ts );
       !video_input.end_of_video() && !input.end_of_video();
       video_input.next_frame( video_ts ),
       audio_input.next_frame( audio_ts ),
       input.next_frame( ts ) )
  {
    SCOPED_TRACE(
      std::string{ "Frame: " } +
      std::to_string( video_ts.get_frame() ) + " | " +
      std::to_string( ts.get_frame() ) );

    // Timestamps must match first video
    EXPECT_EQ( video_ts.get_frame(), ts.get_frame() );
    EXPECT_EQ( video_ts.get_time_usec(), ts.get_time_usec() );

    // Compare audio packets
    auto const check_audio_ptr = audio_input.uninterpreted_frame_data();
    auto const audio_ptr = input.uninterpreted_frame_data();
    if( check_audio_ptr )
    {
      // If original video has audio, the rewired video must have the same audio
      auto const& check_audio =
        dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const* >(
          check_audio_ptr.get() )->audio_packets;

      ASSERT_NE( nullptr, audio_ptr );
      auto const& audio =
        dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const* >(
          audio_ptr.get() )->audio_packets;

      auto const cmp =
        []( ffmpeg::packet_uptr const& lhs, ffmpeg::packet_uptr const& rhs )
        {
          return
            lhs->size == rhs->size &&
            !std::memcmp( lhs->data, rhs->data, lhs->size );
        };
      EXPECT_TRUE(
        std::equal(
          check_audio.begin(), check_audio.end(),
          audio.begin(), audio.end(), cmp ) );
    }
    else if( audio_ptr )
    {
      // If the original video has no audio but the rewired one does, it must
      // be empty
      auto const& audio =
        dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const* >(
          audio_ptr.get() )->audio_packets;
      EXPECT_EQ( 0, audio.size() );
    }

    // Images must be identical
    auto const check_image = video_input.frame_image()->get_image();
    auto const image = input.frame_image()->get_image();
    expect_eq_images( check_image, image, 0.0 );
  }

  // Rewired video must be same length as first video input
  EXPECT_TRUE( video_input.end_of_video() );
  EXPECT_TRUE( input.end_of_video() );

  video_input.close();
  audio_input.close();
  input.close();
}
