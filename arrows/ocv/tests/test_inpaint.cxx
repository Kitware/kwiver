// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/ocv/image_container.h>
#include <arrows/ocv/image_io.h>
#include <arrows/ocv/inpaint.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <opencv2/core/core.hpp>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

kv::path_t g_data_dir;
static std::string test_image_name = "images/small_logo_color.png";
static std::string test_mask_name = "images/small_logo_mask.png";

static std::string expected_telea_result_file = "images/inpaint_telea.png";
static std::string expected_navier_stokes_result_file =
  "images/inpaint_navier_stokes.png";
static std::string expected_mask_result_file = "images/inpaint_mask.png";

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
class inpaint : public ::testing::Test
{
  void SetUp();

  TEST_ARG( data_dir );

  ka::ocv::image_io io;
  ka::ocv::inpaint inpainter;
  kv::image_container_sptr input_image;
  kv::image_container_sptr input_mask;
  kv::config_block_sptr config;

public:
  void test_inpaint_type( kv::config_block_sptr const& config,
                          kv::path_t expected_basename );
};

// ----------------------------------------------------------------------------
void
inpaint
::SetUp()
{
  auto const image_file = data_dir + "/" + test_image_name;
  auto const mask_file = data_dir + "/" + test_mask_name;

  input_image = io.load( image_file );
  input_mask = io.load( mask_file );

  config = kv::config_block::empty_config();
}

// ----------------------------------------------------------------------------
void
inpaint
::test_inpaint_type( kv::config_block_sptr const& config,
                     kv::path_t expected_basename )
{
  inpainter.set_configuration( config );

  auto const inpainted_image_ptr = inpainter.merge( input_image, input_mask );

  auto expected_filename = data_dir + "/" + expected_basename;
  auto const expected_image_ptr = io.load( expected_filename );
  EXPECT_TRUE( equal_content( inpainted_image_ptr->get_image(),
                              expected_image_ptr->get_image() ) );
}

// ----------------------------------------------------------------------------
TEST_F(inpaint, navier_stokes)
{
  config->set_value( "inpaint_method", "navier_stokes" );
  test_inpaint_type( config, expected_navier_stokes_result_file );
}

// ----------------------------------------------------------------------------
TEST_F(inpaint, mask)
{
  config->set_value( "inpaint_method", "mask" );
  test_inpaint_type( config, expected_mask_result_file );
}
