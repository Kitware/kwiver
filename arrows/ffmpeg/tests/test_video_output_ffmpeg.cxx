// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/ffmpeg/ffmpeg_video_output.h>
#include <arrows/ffmpeg/ffmpeg_video_uninterpreted_data.h>
#include <arrows/klv/klv_metadata.h>

#include <vital/plugin_loader/plugin_manager.h>
#include <vital/range/iota.h>

#include <random>

namespace ffmpeg = kwiver::arrows::ffmpeg;
namespace klv = kwiver::arrows::klv;
namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

kv::path_t g_data_dir;

static std::string short_video_name = "videos/aphill_short.ts";
static std::string audio_video_name = "videos/h264_audio.ts";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

namespace {

constexpr uint64_t random_seed = 54321;
constexpr size_t random_image_width = 256;
constexpr size_t random_image_height = 128;

} // namespace

// ----------------------------------------------------------------------------
class ffmpeg_video_output : public ::testing::Test
{
protected:
  void
  SetUp() override
  {
    auto const width = random_image_width;
    auto const height = random_image_height;
    random_image_data.resize( width * height * 3 );

    std::mt19937 generator( random_seed );
    std::uniform_int_distribution< unsigned int > dist( 96, 144 );
    for( auto& element : random_image_data )
    {
      element = dist( generator );
    }

    auto const ptr = random_image_data.data();
    size_t depth = 1;
    random_image_gray =
      kv::image( ptr, width, height, depth, depth, depth * width, 1 );

    depth = 3;
    random_image_rgb_packed =
      kv::image( ptr, width, height, depth, depth, depth * width, 1 );
    random_image_bgr_packed =
      kv::image( ptr + depth - 1, width, height, depth,
                 depth, depth * width, -1 );
    random_image_rgb_planar =
      kv::image( ptr, width, height, depth, 1, width, width * height );
    random_image_bgr_planar =
      kv::image( ptr + width * height * ( depth - 1 ), width, height, depth,
                 1, width, width * height * -1 );

    random_image_container_gray.reset(
        new kv::simple_image_container{ random_image_gray } );
    random_image_container_rgb_packed.reset(
        new kv::simple_image_container{ random_image_rgb_packed } );
    random_image_container_bgr_packed.reset(
        new kv::simple_image_container{ random_image_bgr_packed } );
    random_image_container_rgb_planar.reset(
        new kv::simple_image_container{ random_image_rgb_planar } );
    random_image_container_bgr_planar.reset(
        new kv::simple_image_container{ random_image_bgr_planar } );
  }

  std::vector< uint8_t > random_image_data;

  kv::image random_image_gray;
  kv::image random_image_rgb_packed;
  kv::image random_image_bgr_packed;
  kv::image random_image_rgb_planar;
  kv::image random_image_bgr_planar;

  kv::image_container_sptr random_image_container_gray;
  kv::image_container_sptr random_image_container_rgb_packed;
  kv::image_container_sptr random_image_container_bgr_packed;
  kv::image_container_sptr random_image_container_rgb_planar;
  kv::image_container_sptr random_image_container_bgr_planar;

  std::vector< kv::image_container_sptr > random_image_containers;

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

  EXPECT_LE( error, epsilon );
}

// ----------------------------------------------------------------------------
void
expect_eq_audio( kv::video_uninterpreted_data_sptr const& src_data,
                 kv::video_uninterpreted_data_sptr const& tmp_data )
{
  ASSERT_EQ( src_data == nullptr, tmp_data == nullptr );
  if( !src_data )
  {
    return;
  }

  auto const& src_packets =
    dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const& >( *src_data )
    .audio_packets;
  auto const& tmp_packets =
    dynamic_cast< ffmpeg::ffmpeg_video_uninterpreted_data const& >( *tmp_data )
    .audio_packets;
  ASSERT_EQ( src_packets.size(), tmp_packets.size() );

  auto src_it = src_packets.begin();
  auto tmp_it = tmp_packets.begin();
  while( src_it != src_packets.begin() && tmp_it != tmp_packets.begin() )
  {
    ASSERT_EQ( ( *src_it )->size, ( *tmp_it )->size );
    EXPECT_TRUE(
      std::equal( ( *src_it )->data, ( *src_it )->data + ( *src_it )->size,
                  ( *tmp_it )->data ) );
    ++src_it;
    ++tmp_it;
  }
}

// ----------------------------------------------------------------------------
void
expect_eq_videos( std::string const& src_path, std::string const& tmp_path,
                  double image_epsilon )
{
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
    EXPECT_EQ( src_ts.get_frame(), tmp_ts.get_frame() );
    EXPECT_EQ( src_ts.get_time_usec(), tmp_ts.get_time_usec() );

    auto const src_data = src_is.uninterpreted_frame_data();
    auto const tmp_data = tmp_is.uninterpreted_frame_data();
    expect_eq_audio( src_data, tmp_data );

    auto const src_image = src_is.frame_image()->get_image();
    auto const tmp_image = tmp_is.frame_image()->get_image();
    expect_eq_images( src_image, tmp_image, image_epsilon );
  }
  EXPECT_TRUE( src_is.end_of_video() );
  EXPECT_TRUE( tmp_is.end_of_video() );
  src_is.close();
  tmp_is.close();
}

namespace {

// This will delete the temporary file even if an exception is thrown
struct _tmp_file_deleter
{
  ~_tmp_file_deleter()
  {
    std::remove( tmp_path.c_str() );
  }

  std::string tmp_path;
};

} // namespace

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
  _tmp_file_deleter tmp_file_deleter{ tmp_path };

  // Write to a temporary file
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.frame_image();
    os.add_image( image, ts );
  }
  os.close();
  is.close();

  // Determined experimentally. 6.5 / 256 is non-negligable compression, but
  // you can still see what the image is supposed to be
  auto image_epsilon = 6.5;

  // Hardware decoding produces a lower-quality image
  if( is.get_configuration()->get_value< bool >( "cuda_enabled", false ) )
  {
    image_epsilon = 10.5;
  }

  // Read the temporary file back in
  expect_eq_videos( src_path, tmp_path, image_epsilon );
}

// ----------------------------------------------------------------------------
// Similar to round_trip, but copying the video stream instead of re-encoding.
TEST_F ( ffmpeg_video_output, round_trip_direct )
{
  auto const src_path = data_dir + "/" + short_video_name;
  auto const tmp_path =
    kwiver::testing::temp_file_name( "test-ffmpeg-output-", ".ts" );

  kv::timestamp ts;
  ffmpeg::ffmpeg_video_input is;
  is.open( src_path );

  ffmpeg::ffmpeg_video_output os;
  os.open( tmp_path, is.implementation_settings().get() );
  _tmp_file_deleter tmp_file_deleter{ tmp_path };

  // Skip this test if we can't write the output video in the same format as
  // the input video
  {
    auto const src_generic_settings = is.implementation_settings();
    auto const src_settings =
      dynamic_cast< ffmpeg::ffmpeg_video_settings const* >(
        src_generic_settings.get() );
    auto const tmp_generic_settings = os.implementation_settings();
    auto const tmp_settings =
      dynamic_cast< ffmpeg::ffmpeg_video_settings const* >(
        tmp_generic_settings.get() );
    if( !src_settings || !tmp_settings ||
        src_settings->parameters->codec_id !=
        tmp_settings->parameters->codec_id )
    {
      return;
    }
  }

  // Write to a temporary file
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.raw_frame_image();
    ASSERT_TRUE( image );
    os.add_image( *image );
  }
  os.close();
  is.close();

  // Images should be identical
  auto const image_epsilon = 0.0;

  // Read the temporary file back in
  expect_eq_videos( src_path, tmp_path, image_epsilon );
}

// ----------------------------------------------------------------------------
// Similar to round_trip, but for a test video with an audio stream.
TEST_F ( ffmpeg_video_output, round_trip_audio )
{
  auto const src_path = data_dir + "/" + audio_video_name;
  auto const tmp_path =
    kwiver::testing::temp_file_name( "test-ffmpeg-output-", ".ts" );

  kv::timestamp ts;
  ffmpeg::ffmpeg_video_input is;
  is.open( src_path );

  ffmpeg::ffmpeg_video_output os;
  os.open( tmp_path, is.implementation_settings().get() );
  _tmp_file_deleter tmp_file_deleter{ tmp_path };

  // Write to a temporary file
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.frame_image();
    auto const uninterpreted_data = is.uninterpreted_frame_data();
    if( uninterpreted_data )
    {
      os.add_uninterpreted_data( *uninterpreted_data );
    }
    os.add_image( image, ts );
  }
  os.close();
  is.close();

  // Determined experimentally. 6.5 / 256 is non-negligable compression, but
  // you can still see what the image is supposed to be
  auto image_epsilon = 6.5;

  // Hardware decoding produces a lower-quality image
  if( is.get_configuration()->get_value< bool >( "cuda_enabled", false ) )
  {
    image_epsilon = 10.5;
  }

  // Read the temporary file back in
  expect_eq_videos( src_path, tmp_path, image_epsilon );
}

// ----------------------------------------------------------------------------
// Similar to round_trip_direct, but for a test video with an audio stream.
TEST_F ( ffmpeg_video_output, round_trip_audio_direct )
{
  auto const src_path = data_dir + "/" + audio_video_name;
  auto const tmp_path =
    kwiver::testing::temp_file_name( "test-ffmpeg-output-", ".ts" );

  kv::timestamp ts;
  ffmpeg::ffmpeg_video_input is;
  is.open( src_path );

  ffmpeg::ffmpeg_video_output os;
  os.open( tmp_path, is.implementation_settings().get() );
  _tmp_file_deleter tmp_file_deleter{ tmp_path };

  // Skip this test if we can't write the output video in the same format as
  // the input video
  {
    auto const src_generic_settings = is.implementation_settings();
    auto const src_settings =
      dynamic_cast< ffmpeg::ffmpeg_video_settings const* >(
        src_generic_settings.get() );
    auto const tmp_generic_settings = os.implementation_settings();
    auto const tmp_settings =
      dynamic_cast< ffmpeg::ffmpeg_video_settings const* >(
        tmp_generic_settings.get() );
    if( !src_settings || !tmp_settings ||
        src_settings->parameters->codec_id !=
        tmp_settings->parameters->codec_id )
    {
      return;
    }
  }

  // Write to a temporary file
  for( is.next_frame( ts ); !is.end_of_video(); is.next_frame( ts ) )
  {
    auto const image = is.raw_frame_image();
    ASSERT_TRUE( image );
    auto const uninterpreted_data = is.uninterpreted_frame_data();
    if( uninterpreted_data )
    {
      os.add_uninterpreted_data( *uninterpreted_data );
    }
    os.add_image( *image );
  }
  os.close();
  is.close();

  // Images should be identical
  auto const image_epsilon = 0.0;

  // Read the temporary file back in
  expect_eq_videos( src_path, tmp_path, image_epsilon );
}

// ----------------------------------------------------------------------------
// Ensure we can open a video output without knowing the implementation type.
TEST_F ( ffmpeg_video_output, generic_open )
{
  // Constants
  auto const tmp_path =
    kwiver::testing::temp_file_name( "test-ffmpeg-output-", ".mp4" );
  constexpr size_t frame_rate_num = 15;

  // Create
  ffmpeg::ffmpeg_video_output ff_os;
  kv::algo::video_output& os = ff_os;

  // Configure
  auto config = os.get_configuration();
  config->set_value( "width", random_image_width );
  config->set_value( "height", random_image_height );
  config->set_value( "frame_rate_num", frame_rate_num );
  os.set_configuration( config );

  // Open / close
  os.open( tmp_path, nullptr );
  _tmp_file_deleter tmp_file_deleter{ tmp_path };
  kv::timestamp ts;

  // Add images of varying formats
  os.add_image( random_image_container_gray, ts );
  os.add_image( random_image_container_rgb_packed, ts );
  os.add_image( random_image_container_bgr_packed, ts );
  os.add_image( random_image_container_rgb_planar, ts );
  os.add_image( random_image_container_bgr_planar, ts );

  os.close();
}
