// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test reading video from a list of images.

#include <test_gtest.h>

#include <arrows/core/video_input_image_list.h>
#include <arrows/tests/test_video_input.h>
#include <vital/algo/algorithm_factory.h>
#include <vital/plugin_loader/plugin_manager.h>
#include <vital/vital_config.h>

#include <memory>
#include <string>
#include <iostream>
#include <fstream>

kwiver::vital::path_t g_data_dir;

namespace algo = kwiver::vital::algo;
namespace kac = kwiver::arrows::core;
static std::string list_file_name = "video_as_images/frame_list.txt";
static std::string images_folder_name = "video_as_images/images";

// ----------------------------------------------------------------------------
int
main(int argc, char* argv[])
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class video_input_image_list : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, create)
{
  EXPECT_NE( nullptr, algo::video_input::create("image_list") );
}

// ----------------------------------------------------------------------------
static
bool
set_config(kwiver::vital::config_block_sptr config,
           VITAL_UNUSED std::string const& data_dir)
{
  if ( kwiver::vital::has_algorithm_impl_name( "image_io", "ocv" ) )
  {
    config->set_value( "image_reader:type", "ocv" );
  }
  else if ( kwiver::vital::has_algorithm_impl_name( "image_io", "vxl" ) )
  {
    config->set_value( "image_reader:type", "vxl" );
  }
  else
  {
    std::cout << "Skipping tests since there is no image reader." << std::endl;
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, read_list)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;
  viil.open( list_file );

  kwiver::vital::timestamp ts;

  int num_frames = 0;
  while ( viil.next_frame( ts ) )
  {
    auto img = viil.frame_image();
    auto md = viil.frame_metadata();

    if (md.size() > 0)
    {
      std::cout << "-----------------------------------\n" << std::endl;
      kwiver::vital::print_metadata( std::cout, *md[0] );
    }

    ++num_frames;
    EXPECT_EQ( num_frames, ts.get_frame() )
      << "Frame numbers should be sequential";
    EXPECT_EQ( ts.get_frame(), decode_barcode(*img) )
      << "Frame number should match barcode in frame image";
    EXPECT_EQ( ts.get_time_usec(), viil.frame_timestamp().get_time_usec() );
    EXPECT_EQ( ts.get_frame(), viil.frame_timestamp().get_frame() );
  }
  EXPECT_EQ( num_expected_frames, num_frames );
  EXPECT_EQ( num_expected_frames, viil.num_frames() );
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, read_directory)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + images_folder_name;
  viil.open( list_file );

  kwiver::vital::timestamp ts;

  int num_frames = 0;
  while ( viil.next_frame( ts ) )
  {
    auto img = viil.frame_image();
    auto md = viil.frame_metadata();

    if (md.size() > 0)
    {
      std::cout << "-----------------------------------\n" << std::endl;
      kwiver::vital::print_metadata( std::cout, *md[0] );
    }

    ++num_frames;
    EXPECT_EQ( num_frames, ts.get_frame() )
      << "Frame numbers should be sequential";
    EXPECT_EQ( ts.get_frame(), decode_barcode(*img) )
      << "Frame number should match barcode in frame image";
    EXPECT_EQ( ts.get_time_usec(), viil.frame_timestamp().get_time_usec() );
    EXPECT_EQ( ts.get_frame(), viil.frame_timestamp().get_frame() );
  }
  EXPECT_EQ( num_expected_frames, num_frames );
  EXPECT_EQ( num_expected_frames, viil.num_frames() );
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, is_good)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;
  kwiver::vital::timestamp ts;

  EXPECT_FALSE( viil.good() )
    << "Video state before open";

  // open the video
  viil.open( list_file );
  EXPECT_FALSE( viil.good() )
    << "Video state after open but before first frame";

  // step one frame
  viil.next_frame( ts );
  EXPECT_TRUE( viil.good() )
    << "Video state on first frame";

  // close the video
  viil.close();
  EXPECT_FALSE( viil.good() )
    << "Video state after close";

  // Reopen the video
  viil.open( list_file );

  int num_frames = 0;
  while ( viil.next_frame( ts ) )
  {
    ++num_frames;
    EXPECT_TRUE( viil.good() )
      << "Video state on frame " << ts.get_frame();
  }
  EXPECT_EQ( num_expected_frames, num_frames );
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, seek_frame)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;

  // Open the video
  viil.open( list_file );

  test_seek_frame( viil );

  viil.close();
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, seek_then_next_frame)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;

  // Open the video
  viil.open( list_file );

  test_seek_then_next( viil );

  viil.close();
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, next_then_seek_frame)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;

  // Open the video
  viil.open( list_file );

  test_next_then_seek( viil );

  viil.close();
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, next_then_seek_then_next)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;

  // Open the video
  viil.open( list_file );

  test_next_then_seek_then_next( viil );

  viil.close();
}

// ----------------------------------------------------------------------------
TEST_F(video_input_image_list, metadata_map)
{
  // make config block
  auto config = kwiver::vital::config_block::empty_config();

  if( !set_config(config, data_dir) )
  {
    return;
  }

  kwiver::arrows::core::video_input_image_list viil;

  EXPECT_TRUE( viil.check_configuration( config ) );
  viil.set_configuration( config );

  kwiver::vital::path_t list_file = data_dir + "/" + list_file_name;

  // Open the video
  viil.open( list_file );

  // Get metadata map
  auto md_map = viil.metadata_map()->metadata();

  EXPECT_EQ( md_map.size(), num_expected_frames )
    << "There should be metadata for every frame";

  // Open the list file directly and compare name to metadata
  std::ifstream list_file_stream( list_file );
  int frame_number = 1;
  std::string file_name;
  while ( std::getline( list_file_stream, file_name ) )
  {
    auto md_file_name = md_map[frame_number][0]->find(
        kwiver::vital::VITAL_META_IMAGE_URI).as_string();
    EXPECT_TRUE( md_file_name.find( file_name ) != std::string::npos )
      << "File path in metadata should contain " << file_name;
    frame_number++;
  }
  list_file_stream.close();

  viil.close();
}
