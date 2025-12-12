// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/ffmpeg/tests/common.h>

#include <arrows/ffmpeg/algo/ffmpeg_video_input.h>
#include <arrows/ffmpeg/algo/ffmpeg_video_output.h>

#include <thread>

kv::path_t g_data_dir;

static std::string video_name = "videos/mpeg2_klv.ts";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_video_streaming, tcp )
{
  auto const src_path = g_data_dir + "/" + video_name;

  // Open file on disk, for sending over network
  ffmpeg::ffmpeg_video_input file_input1;
  file_input1.set_real_time( true );
  file_input1.open( src_path );

  // Read first frame to ensure accurate video settings
  kv::timestamp file_ts;
  ASSERT_TRUE( file_input1.next_frame( file_ts ) );

  auto const settings = file_input1.implementation_settings();

  // Open file on disk again, for comparing with received output
  ffmpeg::ffmpeg_video_input file_input2;
  file_input2.open( src_path );

  // Create sender and receiver
  ffmpeg::ffmpeg_video_output network_output;
  ffmpeg::ffmpeg_video_input network_input;

  // Port is hardcoded here for now
  std::string const url = "tcp://localhost:8778/kwiver-test/file.ts";

  // This thread sends the video over the network
  auto const send_video =
    [ & ](){
      // Wait a bit in case receiver hasn't started listening yet
      std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

      network_output.open( url, settings.get() );

      do
      {
        if( auto const data = file_input1.raw_frame_metadata() )
        {
          network_output.add_metadata( *data );
        }

        if( auto const data = file_input1.uninterpreted_frame_data() )
        {
          network_output.add_uninterpreted_data( *data );
        }

        if( auto const image_data = file_input1.raw_frame_image() )
        {
          network_output.add_image( *image_data );
        }
      } while( file_input1.next_frame( file_ts ) );

      network_output.close();
      file_input1.close();
    };

  // This thread receives the video from the first thread
  auto const recv_video =
    [ & ](){
      network_input.open( url );
      expect_eq_videos( file_input2, network_input );
      network_input.close();
    };

  // Start the two threads
  std::thread send_thread{ send_video };
  std::thread recv_thread{ recv_video };

  // Wait for them to finish
  send_thread.join();
  recv_thread.join();
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_video_streaming, next_frame_timeout )
{
  auto const src_path = g_data_dir + "/" + video_name;

  // Open file on disk, for sending over network
  ffmpeg::ffmpeg_video_input file_input1;
  file_input1.set_real_time( true );
  file_input1.open( src_path );

  // Read first frame to ensure accurate video settings
  kv::timestamp file_ts;
  ASSERT_TRUE( file_input1.next_frame( file_ts ) );

  auto const settings = file_input1.implementation_settings();

  // Open file on disk again, for comparing with received output
  ffmpeg::ffmpeg_video_input file_input2;
  file_input2.open( src_path );

  // Create sender and receiver
  ffmpeg::ffmpeg_video_output network_output;
  ffmpeg::ffmpeg_video_input network_input;

  // Port is hardcoded here for now
  std::string const url = "tcp://localhost:8779/kwiver-test/file.ts";

  // This thread sends the video over the network
  auto const send_video =
    [ & ](){
      // Wait a bit in case receiver hasn't started listening yet
      std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

      network_output.open( url, settings.get() );

      do
      {
        if( auto const data = file_input1.raw_frame_metadata() )
        {
          network_output.add_metadata( *data );
        }

        if( auto const data = file_input1.uninterpreted_frame_data() )
        {
          network_output.add_uninterpreted_data( *data );
        }

        if( auto const image_data = file_input1.raw_frame_image() )
        {
          network_output.add_image( *image_data );
        }
      } while( file_input1.next_frame( file_ts ) );

      network_output.close();
      file_input1.close();
    };

  // This thread receives the video from the first thread
  auto const recv_video =
    [ & ](){
      network_input.open( url );
      kv::timestamp network_ts;
      kv::timestamp file2_ts;
      size_t timeout_count = 0;
      while( file_input2.next_frame( file2_ts ) )
      {
        try
        {
          ASSERT_TRUE( network_input.next_frame( network_ts, 1 ) );
        }
        catch( kv::video_input_timeout_exception const& )
        {
          ++timeout_count;
          EXPECT_FALSE( network_input.good() );
          EXPECT_FALSE( network_input.end_of_video() );
          EXPECT_TRUE( network_input.next_frame( network_ts, 5'000'000 ) );
        }
        ASSERT_NE( nullptr, network_input.frame_image() );
        EXPECT_EQ( file2_ts, network_ts );
        expect_eq_images(
          file_input2.frame_image()->get_image(),
          network_input.frame_image()->get_image(), 0.0 );
      }

      EXPECT_EQ( 29, timeout_count );
      EXPECT_FALSE( network_input.next_frame( network_ts ) );

      network_input.close();
    };

  // Start the two threads
  std::thread send_thread{ send_video };
  std::thread recv_thread{ recv_video };

  // Wait for them to finish
  send_thread.join();
  recv_thread.join();
}

// ----------------------------------------------------------------------------
// We *should* have tests for network timeout while seeking, but setting up a
// seekable network resource seems to be difficult with just FFmpeg. Figuring
// this out is not currently with the effort - no use cases require it (yet).
TEST ( ffmpeg_video_streaming, DISABLED_seek_frame_timeout )
{
  // TODO
}
