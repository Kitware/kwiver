// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/ffmpeg/ffmpeg_video_output.h>

#include <vital/plugin_loader/plugin_manager.h>
#include <vital/range/iota.h>

namespace ffmpeg = kwiver::arrows::ffmpeg;
namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

kv::path_t g_data_dir;

static std::string short_video_name = "videos/aphill_short.ts";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class ffmpeg_video_output : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
// Verify the average difference between pixels is not too high. Some
// difference is expected due to compression artifacts, but we need to make
// sure the frame images we get out are generally the same as what we put in.
void
expect_eq_images( kv::image const& src_image,
                  kv::image const& tmp_image,
                  double epsilon )
{
  auto error = 0.0;

  ASSERT_TRUE( src_image.width() == tmp_image.width() );
  ASSERT_TRUE( src_image.height() == tmp_image.height() );
  ASSERT_TRUE( src_image.depth() == tmp_image.depth() );

  for( auto const i : kvr::iota( src_image.width() ) )
  {
    for( auto const j : kvr::iota( src_image.height() ) )
    {
      for( auto const k : kvr::iota( src_image.depth() ) )
      {
        error += std::abs(
          static_cast< double >( src_image.at< uint8_t >( i, j, k ) ) -
          static_cast< double >( tmp_image.at< uint8_t >( i, j, k ) ) );
      }
    }
  }
  error /= src_image.width() * src_image.height() * src_image.depth();

  EXPECT_LT( error, epsilon );
}

// ----------------------------------------------------------------------------
// Test that reading, writing, then reading a video produces generally the
// same result as the first time we read it.
TEST_F ( ffmpeg_video_output, round_trip )
{
  auto const src_path = data_dir + "/" + short_video_name;
  auto const tmp_path =
    kwiver::testing::temp_file_name( "test-ffmpeg-output-", ".ts" );

  kv::timestamp ts;
  ffmpeg::ffmpeg_video_input is;
  is.open( src_path );

  ffmpeg::ffmpeg_video_output os;
  os.open( tmp_path, is.implementation_settings().get() );

  // This will delete the temporary file even if an exception is thrown
  struct _tmp_file_deleter
  {
    ~_tmp_file_deleter()
    {
      std::remove( tmp_path.c_str() );
    }

    std::string tmp_path;
  } tmp_file_deleter{ tmp_path };

  // Write to a temporary file
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.frame_image();
    os.add_image( image, ts );
  }
  os.close();
  is.close();

  // Read the temporary file back in
  ffmpeg::ffmpeg_video_input src_is;
  ffmpeg::ffmpeg_video_input tmp_is;
  kv::timestamp src_ts;
  kv::timestamp tmp_ts;
  src_is.open( src_path );
  tmp_is.open( tmp_path );

  // Check each pair of frames for equality
  for( src_is.next_frame( src_ts ), tmp_is.next_frame( tmp_ts );
       !src_is.end_of_video() && !tmp_is.end_of_video();
       src_is.next_frame( src_ts ), tmp_is.next_frame( tmp_ts ) )
  {
    // Determined experimentally. 6.5 / 256 is non-negligable compression, but
    // you can still see what the image is supposed to be
    auto const image_epsilon = 6.5;
    auto const src_image = src_is.frame_image()->get_image();
    auto const tmp_image = tmp_is.frame_image()->get_image();
    expect_eq_images( src_image, tmp_image, image_epsilon );
  }
  EXPECT_TRUE( src_is.end_of_video() );
  EXPECT_TRUE( tmp_is.end_of_video() );
  src_is.close();
  tmp_is.close();
}
