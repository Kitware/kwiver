// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test ffmpeg video reader's KLV capabilities.

#include <tests/test_gtest.h>

#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/serialize/json/klv/metadata_map_io.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <kwiversys/SystemTools.hxx>

#include <array>

using namespace kwiver::arrows;
namespace kv = kwiver::vital;

std::string g_data_dir;

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
class ffmpeg_video_input_klv : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // TODO(C++17): Replace with std::filesystem
    using st = kwiversys::SystemTools;
    mpeg2_path = st::JoinPath( { "", data_dir, "videos/mpeg2_klv.ts" } );
    h264_path = st::JoinPath( { "", data_dir, "videos/h264_klv.ts" } );
    h265_path = st::JoinPath( { "", data_dir, "videos/h265_klv.ts" } );
    no_streams_path =
      st::JoinPath( { "", data_dir, "videos/h264_no_klv.ts" } );
    tricky_streams_path =
      st::JoinPath( { "", data_dir, "videos/h265_tricky_klv.ts" } );
    stream_klv_path =
      st::JoinPath( { "", data_dir, "video_stream_klv.json.zz" } );
    tricky_stream_klv_path =
      st::JoinPath( { "", data_dir, "video_stream_tricky_klv.json.zz" } );

    {
      auto config = input.get_configuration();
      config->set_value< bool >( "use_misp_timestamps", true );
      input.set_configuration( config );
    }

    {
      auto config = serializer.get_configuration();
      config->set_value< bool >( "compress", true );
      serializer.set_configuration( config );
    }
  }

  // Loads expected KLV for standard videos from JSON
  void load_stream_klv()
  {
    if( !expected_stream_klv.empty() )
    {
      return;
    }

    auto const map = serializer.load( stream_klv_path )->metadata();
    for( auto& entry : map )
    {
      expected_stream_klv.emplace(
        entry.first,
        std::move(
          dynamic_cast< klv::klv_metadata& >( *entry.second.at( 0 ) ) ) );
    }
  }

  // Loads expected KLV for "tricky stream" video from JSON
  void load_tricky_stream_klv()
  {
    if( !expected_tricky_stream_klv.empty() )
    {
      return;
    }

    auto const map =
      serializer.load( tricky_stream_klv_path )->metadata();
    for( auto& entry : map )
    {
      for( auto& md : entry.second )
      {
        expected_tricky_stream_klv.emplace(
          entry.first,
          std::move( dynamic_cast< klv::klv_metadata& >( *md ) ) );
      }
    }
  }

  // Corners of each frame are set to specific colors
  void
  verify_rgb_sentinel(kv::image const &image) {
    auto const x = image.width() - 1;
    auto const y = image.height() - 1;

    EXPECT_NEAR( 255, image.at< uint8_t >( 0, 0, 0 ), pixel_epsilon );
    EXPECT_NEAR( 0,   image.at< uint8_t >( 0, 0, 1 ), pixel_epsilon );
    EXPECT_NEAR( 0,   image.at< uint8_t >( 0, 0, 2 ), pixel_epsilon );

    EXPECT_NEAR( 0,   image.at< uint8_t >( 0, y, 0 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( 0, y, 1 ), pixel_epsilon );
    EXPECT_NEAR( 0,   image.at< uint8_t >( 0, y, 2 ), pixel_epsilon );

    EXPECT_NEAR( 0,   image.at< uint8_t >( x, y, 0 ), pixel_epsilon );
    EXPECT_NEAR( 0,   image.at< uint8_t >( x, y, 1 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( x, y, 2 ), pixel_epsilon );

    EXPECT_NEAR( 255, image.at< uint8_t >( x, 0, 0 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( x, 0, 1 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( x, 0, 2 ), pixel_epsilon );
  }

  // Corners of each frame are set to specific values
  void
  verify_gray_sentinel(kv::image const &image) {
    auto const x = image.width() - 1;
    auto const y = image.height() - 1;

    EXPECT_NEAR( 0, image.at< uint8_t >( 0, 0, 0 ), pixel_epsilon );
    EXPECT_NEAR( 0, image.at< uint8_t >( 0, 0, 1 ), pixel_epsilon );
    EXPECT_NEAR( 0, image.at< uint8_t >( 0, 0, 2 ), pixel_epsilon );

    EXPECT_NEAR( 255, image.at< uint8_t >( 0, y, 0 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( 0, y, 1 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( 0, y, 2 ), pixel_epsilon );

    EXPECT_NEAR( 0, image.at< uint8_t >( x, y, 0 ), pixel_epsilon );
    EXPECT_NEAR( 0, image.at< uint8_t >( x, y, 1 ), pixel_epsilon );
    EXPECT_NEAR( 0, image.at< uint8_t >( x, y, 2 ), pixel_epsilon );

    EXPECT_NEAR( 255, image.at< uint8_t >( x, 0, 0 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( x, 0, 1 ), pixel_epsilon );
    EXPECT_NEAR( 255, image.at< uint8_t >( x, 0, 2 ), pixel_epsilon );
  }

  // Frame number encoded as 32-bit binary number in top row of pixels.
  // Light grey = 1, dark grey = 0
  uint32_t
  read_frame_number_sentinel(kv::image const& image)
  {
    uint32_t result = 0;
    for( size_t i = 0; i < 32; ++i )
    {
      auto const total =
        static_cast< uint16_t >( image.at< uint8_t >( i + 2, 0 ) ) +
        static_cast< uint16_t >( image.at< uint8_t >( i + 2, 1 ) ) +
        static_cast< uint16_t >( image.at< uint8_t >( i + 2, 2 ) );
      result |= ( ( total > 127 * 3 ) << i );
    }
    return result;
  }

  void
  verify_standard_video()
  {
    load_stream_klv();

    kv::timestamp ts;
    for( auto const& entry : expected_stream_klv )
    {
      SCOPED_TRACE( std::string{ "Frame: " } + std::to_string( entry.first ) );

      // Check that loading the next frame works
      ASSERT_TRUE( input.next_frame( ts ) );
      EXPECT_EQ( entry.first, ts.get_frame() );

      // Check that video has metadata
      auto const input_md = input.frame_metadata();
      ASSERT_EQ( 1, input_md.size() );

      // Check that video has KLV
      auto const input_klv_md =
        dynamic_cast< klv::klv_metadata const* >( input_md.at( 0 ).get() );
      ASSERT_NE( nullptr, input_klv_md );

      // Check KLV values
      auto const& expected_klv = entry.second.klv();
      auto const& actual_klv = input_klv_md->klv();
      EXPECT_EQ( expected_klv, actual_klv );

      // Check MISP timestamp
      EXPECT_EQ(
        "misp",
        input_klv_md->find( kv::VITAL_META_UNIX_TIMESTAMP_SOURCE )
          .as_string() );
      auto const expected_timestamp =
        0x000459F4A6AA4AA8ull +
        static_cast< uint64_t >( ( entry.first - 1 ) * 1000000.0 / 30.0 );
      EXPECT_EQ(
        expected_timestamp,
        input_klv_md->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );

      // Check frame image
      auto const image = input.frame_image();
      ASSERT_NE( nullptr, image );
      ASSERT_EQ( 160, image->width() );
      ASSERT_EQ( 120, image->height() );
      ASSERT_EQ( 3, image->depth() );
      verify_rgb_sentinel( image->get_image() );

      // Check frame number code
      EXPECT_EQ(
        entry.first, read_frame_number_sentinel( image->get_image() ) + 1 );
    }

    // Check end of video
    EXPECT_FALSE( input.next_frame( ts ) );
    EXPECT_TRUE( input.end_of_video() );
  }

  constexpr static size_t pixel_epsilon = 32;
  constexpr static size_t expected_frame_count = 30;

  ffmpeg::ffmpeg_video_input input;
  std::string mpeg2_path;
  std::string h264_path;
  std::string h265_path;
  std::string no_streams_path;
  std::string tricky_streams_path;
  std::string stream_klv_path;
  std::string tricky_stream_klv_path;
  std::map< kv::frame_id_t, klv::klv_metadata > expected_stream_klv;
  std::multimap< kv::frame_id_t, klv::klv_metadata > expected_tricky_stream_klv;
  serialize::json::metadata_map_io_klv serializer;
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_klv, mpeg2_klv_verify )
{
  input.open( mpeg2_path );
  verify_standard_video();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_klv, h264_klv_verify )
{
  input.open( h264_path );
  verify_standard_video();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_klv, h265_klv_verify )
{
  input.open( h265_path );
  verify_standard_video();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_klv, h264_no_klv_verify )
{
  input.open( no_streams_path );

  kv::timestamp ts;
  for( size_t i = 0; i < expected_frame_count; ++i )
  {
    auto const frame_number = i + 1;
    SCOPED_TRACE( std::string{ "Frame: " } + std::to_string( frame_number ) );

    // Check that loading the next frame works
    ASSERT_TRUE( input.next_frame( ts ) );
    EXPECT_EQ( frame_number, ts.get_frame() );

    // Check that video has metadata
    auto const input_md = input.frame_metadata();
    ASSERT_EQ( 1, input_md.size() );

    // Check that video has no KLV
    ASSERT_EQ(
      nullptr,
      dynamic_cast< klv::klv_metadata const* >( input_md.at( 0 ).get() ) );

    // Check frame image
    auto const image = input.frame_image();
    ASSERT_NE( nullptr, image );
    ASSERT_EQ( 160, image->width() );
    ASSERT_EQ( 120, image->height() );
    ASSERT_EQ( 3, image->depth() );
    verify_rgb_sentinel( image->get_image() );

    // Check frame number code
    EXPECT_EQ(
      frame_number, read_frame_number_sentinel( image->get_image() ) + 1 );
  }

  // Check end of video
  EXPECT_FALSE( input.next_frame( ts ) );
  EXPECT_TRUE( input.end_of_video() );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input_klv, h265_tricky_klv_verify )
{
  load_tricky_stream_klv();

  input.open( tricky_streams_path );
  kv::timestamp ts;
  for( size_t i = 0; i < expected_frame_count; ++i )
  {
    auto const frame_number = i + 1;
    SCOPED_TRACE( std::string{ "Frame: " } + std::to_string( frame_number ) );

    // Check that loading the next frame works
    ASSERT_TRUE( input.next_frame( ts ) );
    EXPECT_EQ( frame_number, ts.get_frame() );

    // Check that video has metadata
    auto const input_mds = input.frame_metadata();
    ASSERT_EQ( 3, input_mds.size() );

    // Check that video has KLV
    auto const expected_md_range =
      expected_tricky_stream_klv.equal_range( frame_number );
    for( auto const& input_md : input_mds )
    {
      // Get which stream this packet came from
      auto const stream_index =
        input_md->find( kv::VITAL_META_VIDEO_DATA_STREAM_INDEX ).get< int >();
      SCOPED_TRACE(
        std::string{ "Stream: " } + std::to_string( stream_index ) );

      // Get KLV-specific metadata object
      auto const input_klv_md =
        dynamic_cast< klv::klv_metadata const* >( input_md.get() );
      ASSERT_NE( nullptr, input_klv_md );

      // Attempt to find matching ground-truth object
      std::vector< klv::klv_packet > const* expected_klv = nullptr;
      for( auto it = expected_md_range.first;
           it != expected_md_range.second; ++it )
      {
        auto const index_entry =
          it->second.find( kv::VITAL_META_VIDEO_DATA_STREAM_INDEX );
        ASSERT_TRUE( index_entry.is_valid() );
        auto const this_stream_index = index_entry.get< int >();
        if( this_stream_index == stream_index )
        {
          expected_klv = &it->second.klv();
          break;
        }
      }

      // Ensure KLV packets are equal
      auto const& actual_klv = input_klv_md->klv();
      if( expected_klv == nullptr )
      {
        EXPECT_TRUE( actual_klv.empty() );
      }
      else
      {
        EXPECT_EQ( *expected_klv, actual_klv );
      }

      // Check MISP timestamp
      EXPECT_EQ(
        "misp",
        input_klv_md->find( kv::VITAL_META_UNIX_TIMESTAMP_SOURCE )
          .as_string() );
      auto const expected_timestamp =
        0x000459F4A6AA4AA8ull +
        static_cast< uint64_t >( i * 1000000.0 / 30.0 );
      EXPECT_EQ(
        expected_timestamp,
        input_klv_md->find( kv::VITAL_META_UNIX_TIMESTAMP ).as_uint64() );
    }

    // Check frame image
    auto const image = input.frame_image();
    ASSERT_NE( nullptr, image );
    ASSERT_EQ( 160, image->width() );
    ASSERT_EQ( 120, image->height() );
    ASSERT_EQ( 3, image->depth() );
    verify_gray_sentinel( image->get_image() );

    // Check frame number code
    EXPECT_EQ(
      frame_number, read_frame_number_sentinel( image->get_image() ) + 1 );
  }

  // Check end of video
  EXPECT_FALSE( input.next_frame( ts ) );
  EXPECT_TRUE( input.end_of_video() );
}
