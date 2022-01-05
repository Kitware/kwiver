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

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ---------------------------------------------------------------------------
class ffmpeg_video_output : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ---------------------------------------------------------------------------
TEST_F ( ffmpeg_video_output, round_trip )
{
  auto const src_path = data_dir + "/aphill_short.ts";
  auto const tmp_path =
    kwiver::testing::temp_file_name( "test-ffmpeg-output-", ".ts" );

  kv::timestamp ts;
  ffmpeg::ffmpeg_video_input is;
  is.open( src_path );

  ffmpeg::ffmpeg_video_output os;
  os.open( tmp_path, is.implementation_settings().get() );

  std::map< int64_t, kv::image_container_sptr > src_images;
  std::map< int64_t, kv::image_container_sptr > tmp_images;

  // Write to a temporary file
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.frame_image();
    src_images.emplace( ts.get_frame(), image );
    os.add_image( image, ts );
  }

  os.close();
  is.close();

  // Read the temporary file back in
  is.open( tmp_path );
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.frame_image();
    tmp_images.emplace( ts.get_frame(), image );
  }
  is.close();

  // Verify the the average difference between pixels is not too high. Some
  // difference is expected due to compression artifacts, but we need to make
  // sure the frame images we get out are generally the same as what we put in
  EXPECT_EQ( src_images.size(), tmp_images.size() );
  for( auto const& image_entry : src_images )
  {
    ASSERT_TRUE( tmp_images.count( image_entry.first ) );

    auto error = 0.0;

    auto const& src_image = image_entry.second->get_image();
    auto const& tmp_image = tmp_images.at( image_entry.first )->get_image();
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

    // Determined experimentally. 6 / 256 is non-negligable compression, but
    // you can still see what the image is supposed to be
    EXPECT_LT( error, 6.0 );
  }
}
