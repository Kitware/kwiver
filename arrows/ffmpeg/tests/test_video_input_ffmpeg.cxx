// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test opening/closing a video file

#include <test_gtest.h>

#include <arrows/core/video_input_filter.h>
#include <arrows/ffmpeg/ffmpeg_video_input.h>
#include <arrows/tests/test_video_input.h>

#include <vital/exceptions/io.h>
#include <vital/exceptions/video.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <kwiversys/SystemTools.hxx>

#include <iostream>
#include <memory>
#include <string>


namespace ffmpeg = kwiver::arrows::ffmpeg;
namespace core = kwiver::arrows::core;
namespace kv = kwiver::vital;
namespace algo = kv::algo;

kv::path_t g_data_dir;

static size_t const expected_frame_count = 50;

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
class ffmpeg_video_input : public ::testing::Test
{
public:
  void SetUp() override
  {
    // TODO(C++17): Replace with std::filesystem
    using st = kwiversys::SystemTools;
    ffmpeg_video_path =
      st::JoinPath( { "", data_dir, "videos/ffmpeg_video.mp4" } );
    aphill_video_path =
      st::JoinPath( { "", data_dir, "videos/aphill_short.ts" } );
  }

  kv::path_t ffmpeg_video_path;
  kv::path_t aphill_video_path;
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, create )
{
  EXPECT_NE( nullptr, algo::video_input::create( "ffmpeg" ) );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, is_good_before_open )
{
  ffmpeg::ffmpeg_video_input input;
  EXPECT_FALSE( input.good() );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, is_good_correct_file_path )
{
  ffmpeg::ffmpeg_video_input input;

  // Open the video
  input.open( ffmpeg_video_path );
  EXPECT_FALSE( input.good() )
    << "Video state after open but before first frame";

  // Get the next frame
  kv::timestamp ts;
  EXPECT_TRUE( input.next_frame( ts ) )
    << "Video state after open but before first frame";
  EXPECT_EQ( ts.get_frame(), 1 ) << "Initial frame value mismatch";
  EXPECT_TRUE( input.good() ) << "Video state after first frame";

  // Close the video
  input.close();
  EXPECT_FALSE( input.good() ) << "Video state after close";
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, is_good_invalid_file_path )
{
  ffmpeg::ffmpeg_video_input input;
  auto const filename = data_dir + "/DoesNOTExist.mp4";

  // Open the video
  EXPECT_THROW( input.open( filename ), kv::file_not_found_exception );
  EXPECT_FALSE( input.good() )
    << "Video state after open but before first frame";

  // Get the next frame
  kv::timestamp ts;
  EXPECT_THROW( input.next_frame( ts ), kv::file_not_read_exception );
  EXPECT_EQ( ts.get_frame(), 0 ) << "Initial frame value mismatch";
  EXPECT_FALSE( input.good() ) << "Video state after first frame";

  // Close the video
  input.close();
  EXPECT_FALSE( input.good() ) << "Video state after close";
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, frame_image )
{
  ffmpeg::ffmpeg_video_input input;

  // Open the video
  input.open( ffmpeg_video_path );
  EXPECT_FALSE( input.good() )
    << "Video state after open but before first frame";
  EXPECT_EQ( input.frame_image(), nullptr )
    << "Video should not have an image yet";

  // Get the next frame
  kv::timestamp ts;
  input.next_frame( ts );
  EXPECT_EQ( ts.get_frame(), 1 );

  auto const frame = input.frame_image();
  EXPECT_EQ( frame->depth(), 3 );
  EXPECT_EQ( frame->get_image().width(), 80 );
  EXPECT_EQ( frame->get_image().height(), 54 );
  EXPECT_EQ( frame->get_image().d_step(), 1 );
  EXPECT_EQ( frame->get_image().h_step(), 80 * 3 );
  EXPECT_EQ( frame->get_image().w_step(), 3 );
  EXPECT_TRUE( frame->get_image().is_contiguous() );

  EXPECT_EQ( decode_barcode( *frame ), 1 );
}

// ----------------------------------------------------------------------------
// Verify that disabling imagery processing acts as expected and doesn't break
// anything else.
TEST_F( ffmpeg_video_input, imagery_disabled )
{
  ffmpeg::ffmpeg_video_input input;

  auto config = input.get_configuration();
  config->set_value< bool >( "imagery_enabled", false );
  input.set_configuration( config );
  input.open( aphill_video_path );

  EXPECT_FALSE( input.good() );
  EXPECT_EQ( input.frame_image(), nullptr );

  size_t frame_count = 0;
  kv::timestamp ts;
  while( input.next_frame( ts ) )
  {
    ++frame_count;
    EXPECT_TRUE( input.good() );
    EXPECT_EQ( input.frame_image(), nullptr );
    EXPECT_EQ( ts.get_frame(), frame_count );

    auto const md = input.frame_metadata();
    ASSERT_FALSE( md.empty() );
    ASSERT_TRUE( md.at( 0 )->has( kv::VITAL_META_UNIX_TIMESTAMP ) );
  }

  input.close();
  EXPECT_FALSE( input.good() );
}

// ----------------------------------------------------------------------------
// Verify that disabling KLV processing acts as expected and doesn't break
// anything else.
TEST_F( ffmpeg_video_input, klv_disabled )
{
  ffmpeg::ffmpeg_video_input input;

  auto config = input.get_configuration();
  config->set_value< bool >( "klv_enabled", false );
  input.set_configuration( config );
  input.open( ffmpeg_video_path );

  EXPECT_FALSE( input.good() );
  EXPECT_FALSE(
    input.get_implementation_capabilities().capability(
      algo::video_input::HAS_METADATA ) );

  size_t frame_count = 0;
  kv::timestamp ts;
  while( input.next_frame( ts ) )
  {
    ++frame_count;
    EXPECT_TRUE( input.good() );
    EXPECT_NE( input.frame_image(), nullptr );
    EXPECT_EQ( ts.get_frame(), frame_count );

    auto const md = input.frame_metadata();
    ASSERT_FALSE( md.empty() );
    ASSERT_FALSE( md.at( 0 )->has( kv::VITAL_META_UNIX_TIMESTAMP ) );
  }

  input.close();
  EXPECT_FALSE( input.good() );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, seek_frame )
{
  ffmpeg::ffmpeg_video_input input;

  // Open the video
  input.open( ffmpeg_video_path );

  test_seek_frame( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, seek_then_next_frame )
{
  ffmpeg::ffmpeg_video_input input;

  // Open the video
  input.open( ffmpeg_video_path );

  test_seek_then_next( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, next_then_seek_frame )
{
  ffmpeg::ffmpeg_video_input input;

  // Open the video
  input.open( ffmpeg_video_path );

  test_next_then_seek( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, next_then_seek_then_next )
{
  ffmpeg::ffmpeg_video_input input;

  // Open the video
  input.open( ffmpeg_video_path );

  test_next_then_seek_then_next( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, end_of_video )
{
  ffmpeg::ffmpeg_video_input input;

  EXPECT_TRUE( input.end_of_video() ) << "End of video before open";

  // Open the video
  input.open( ffmpeg_video_path );
  EXPECT_FALSE( input.end_of_video() ) << "End of video after open";

  kv::timestamp ts;
  while( input.next_frame( ts ) )
  {
    EXPECT_FALSE( input.end_of_video() ) << "End of video while reading";
  }

  EXPECT_EQ( ts.get_frame(), expected_frame_count ) << "Last frame";
  EXPECT_TRUE( input.end_of_video() ) << "End of video after last frame";
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, read_video_aphill )
{
  ffmpeg::ffmpeg_video_input input;

  input.open( aphill_video_path );

  kv::timestamp ts;
  size_t frame_count = 0;
  while( input.next_frame( ts ) )
  {
    ++frame_count;
    EXPECT_EQ( frame_count, ts.get_frame() )
      << "Frame numbers should be sequential";
  }

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, read_video )
{
  ffmpeg::ffmpeg_video_input input;
  input.open( ffmpeg_video_path );

  EXPECT_EQ( expected_frame_count, input.num_frames() )
    << "Number of frames before extracting frames should be "
    << expected_frame_count;

  kv::timestamp ts;
  size_t frame_count = 0;
  while( input.next_frame( ts ) )
  {
    ++frame_count;

    auto img = input.frame_image();
    auto md = input.frame_metadata();

    EXPECT_EQ( frame_count, ts.get_frame() )
      << "Frame numbers should be sequential";
    EXPECT_EQ( ts.get_frame(), decode_barcode( *img ) )
      << "Frame number should match barcode in frame image";

    EXPECT_EQ( red, test_color_pixel( 1, *img ) );
    EXPECT_EQ( green, test_color_pixel( 2, *img ) );
    EXPECT_EQ( blue, test_color_pixel( 3, *img ) );
  }
  EXPECT_EQ( expected_frame_count, frame_count );
  EXPECT_EQ( expected_frame_count, input.num_frames() );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, read_video_nth_frame_output )
{
  // Make config block
  auto config = kv::config_block::empty_config();
  config->set_value( "video_input:type", "ffmpeg" );
  config->set_value( "output_nth_frame", nth_frame_output );

  core::video_input_filter input;

  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  test_read_video_nth_frame( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, seek_nth_frame_output )
{
  // Make config block
  auto config = kv::config_block::empty_config();
  config->set_value( "video_input:type", "ffmpeg" );
  config->set_value( "output_nth_frame", nth_frame_output );

  core::video_input_filter input;

  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  test_seek_nth_frame( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, read_video_sublist )
{
  // Make config block
  auto config = kv::config_block::empty_config();
  config->set_value( "video_input:type", "ffmpeg" );
  config->set_value( "start_at_frame", start_at_frame );
  config->set_value( "stop_after_frame", stop_after_frame );

  core::video_input_filter input;

  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  test_seek_frame_sublist( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, read_video_sublist_nth_frame )
{
  // Make config block
  auto config = kv::config_block::empty_config();
  config->set_value( "video_input:type", "ffmpeg" );
  config->set_value( "start_at_frame", start_at_frame );
  config->set_value( "stop_after_frame", stop_after_frame );
  config->set_value( "output_nth_frame", nth_frame_output );

  core::video_input_filter input;

  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  test_read_video_sublist_nth_frame( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, seek_frame_sublist_nth_frame )
{
  // Make config block
  auto config = kv::config_block::empty_config();
  config->set_value( "video_input:type", "ffmpeg" );
  config->set_value( "start_at_frame", start_at_frame );
  config->set_value( "stop_after_frame", stop_after_frame );
  config->set_value( "output_nth_frame", nth_frame_output );

  core::video_input_filter input;

  EXPECT_TRUE( input.check_configuration( config ) );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  test_seek_sublist_nth_frame( input );

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, metadata_map )
{
  ffmpeg::ffmpeg_video_input input;

  input.open( ffmpeg_video_path );

  // Metadata capability is false since no external metadata is present
  auto const& caps = input.get_implementation_capabilities();
  EXPECT_FALSE( caps.capability( kv::algo::video_input::HAS_METADATA ) );

  // Get metadata map
  auto md_map = input.metadata_map()->metadata();

  // Each frame of video should have some metadata;
  // at a minimum just the video name and timestamp
  EXPECT_EQ( md_map.size(), input.num_frames() );

  if( md_map.size() != input.num_frames() )
  {
    std::cout << "Found metadata on these frames: ";
    for( auto md : md_map )
    {
      std::cout << md.first << ", ";
    }

    std::cout << std::endl;
  }
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, sync_metadata )
{
  static std::map< size_t, std::set< uint64_t > > const expected_md =
    { { 0, { 1221515219356000, 1221515219396000 } },
      { 1, { 1221515219426000 } },
      { 2, { 1221515219456000 } },
      { 3, { 1221515219486000 } },
      { 4, { 1221515219516000 } } };

  ffmpeg::ffmpeg_video_input input;

  auto config = input.get_configuration();
  input.set_configuration( config );

  // Open the video
  input.open( aphill_video_path );

  auto const& caps = input.get_implementation_capabilities();
  EXPECT_TRUE( caps.capability( kv::algo::video_input::HAS_METADATA ) );

  kv::timestamp ts;
  size_t frame_count = 0;
  while( input.next_frame( ts ) && frame_count < expected_md.size() )
  {
    auto md_vect = input.frame_metadata();

    EXPECT_TRUE( md_vect.size() > 0 )
      << "Each frame tested should have metadata present";

    for( auto md : md_vect )
    {
      EXPECT_TRUE( md->has( kv::VITAL_META_UNIX_TIMESTAMP ) )
        << "Each of the first five frames should have a UNIX time stamp in"
        << " its metadata";

      for( auto md_item : *md )
      {
        if( md_item.first == kv::VITAL_META_UNIX_TIMESTAMP )
        {
          EXPECT_TRUE(
            expected_md.at( frame_count )
              .count( md_item.second->as_uint64() ) )
            << "UNIX time stamp " << md_item.second->as_uint64()
            << " was not found in metadata for frame " << frame_count;
        }
      }
    }

    ++frame_count;
  }

  input.close();
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, empty_filter_desc )
{
  ffmpeg::ffmpeg_video_input input;
  auto config = input.get_configuration();
  // make the avfilter pipeline empty
  config->set_value( "filter_desc", "" );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  // Get the next frame
  kv::timestamp ts;
  input.next_frame( ts );
  EXPECT_EQ( ts.get_frame(), 1 );

  kv::image_container_sptr frame = input.frame_image();
  EXPECT_EQ( frame->depth(), 3 );
  EXPECT_EQ( frame->get_image().width(), 80 );
  EXPECT_EQ( frame->get_image().height(), 54 );
  EXPECT_EQ( frame->get_image().d_step(), 1 );
  EXPECT_EQ( frame->get_image().h_step(), 80 * 3 );
  EXPECT_EQ( frame->get_image().w_step(), 3 );
  EXPECT_TRUE( frame->get_image().is_contiguous() );

  EXPECT_EQ( decode_barcode( *frame ), 1 );

  input.next_frame( ts );
  frame = input.frame_image();
  EXPECT_EQ( ts.get_frame(), 2 );
  EXPECT_EQ( decode_barcode( *frame ), 2 );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, invalid_filter_desc )
{
  ffmpeg::ffmpeg_video_input input;
  auto config = input.get_configuration();
  config->set_value( "filter_desc", "_invalid_filter_" );
  input.set_configuration( config );

  // Open the video
  EXPECT_THROW( input.open( ffmpeg_video_path ), kv::video_runtime_exception );
}

// ----------------------------------------------------------------------------
// Helper function to make a horizontally flipped image view
// TODO: Make this a more general function within KWIVER
kv::image
hflip_image( kv::image const& image )
{
  auto const w = image.width();
  auto const h = image.height();
  auto const d = image.depth();
  auto const ws = image.w_step();
  auto const hs = image.h_step();
  auto const ds = image.d_step();
  return kv::image(
    image.memory(),
    static_cast< const uint8_t* >( image.first_pixel() ) + ws * ( w - 1 ),
    w, h, d, -ws, hs + ws * w, ds, image.pixel_traits() );
}

// ----------------------------------------------------------------------------
TEST_F ( ffmpeg_video_input, hflip_filter_desc )
{
  ffmpeg::ffmpeg_video_input input;
  auto config = input.get_configuration();

  // Use the hflip filter for horizontal flipping
  config->set_value( "filter_desc", "hflip" );
  input.set_configuration( config );

  // Open the video
  input.open( ffmpeg_video_path );

  // Get the next frame
  kv::timestamp ts;
  input.next_frame( ts );
  EXPECT_EQ( ts.get_frame(), 1 );

  kv::image_container_sptr frame = input.frame_image();
  EXPECT_EQ( frame->depth(), 3 );
  EXPECT_EQ( frame->get_image().width(), 80 );
  EXPECT_EQ( frame->get_image().height(), 54 );
  EXPECT_EQ( frame->get_image().d_step(), 1 );
  EXPECT_EQ( frame->get_image().h_step(), 80 * 3 );
  EXPECT_EQ( frame->get_image().w_step(), 3 );
  EXPECT_TRUE( frame->get_image().is_contiguous() );

  EXPECT_NE( decode_barcode( *frame ), 1 );

  // Undo horizontal flipping and confirm that the frame is now correct
  kv::simple_image_container hflip_frame( hflip_image( frame->get_image() ) );
  EXPECT_EQ( decode_barcode( hflip_frame ), 1 );

  input.next_frame( ts );
  frame = input.frame_image();
  EXPECT_EQ( ts.get_frame(), 2 );
  EXPECT_NE( decode_barcode( *frame ), 2 );

  // Undo horizontal flipping and confirm that the frame is now correct
  hflip_frame =
    kv::simple_image_container( hflip_image( frame->get_image() ) );
  EXPECT_EQ( decode_barcode( hflip_frame ), 2 );
}
