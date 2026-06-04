// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_scene.h>

#include <arrows/cuda/algo/integrate_depth_maps.h>
#include <arrows/tests/test_integrate_depth_maps.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

#include <algorithm>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( integrate_depth_maps, create )
{
  using namespace kwiver::vital;

  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kwiver::vital::create_algorithm< kwiver::vital::algo::integrate_depth_maps >
      ( "cuda" ) );
}

// ----------------------------------------------------------------------------
// Test depth map integration
TEST ( integrate_depth_maps, integrate )
{
  namespace cuda = kwiver::arrows::cuda;


  std::vector< image_container_sptr > depth_maps;
  std::vector< camera_perspective_sptr > cams;
  vector_3d min_pt, max_pt;
  auto K = simple_camera_intrinsics(
    200, { 80, 60 }, 1.0, 0.0, {}, 160, 120 );
  make_test_data( depth_maps, cams, min_pt, max_pt, K );


  cuda::integrate_depth_maps algorithm;
  config_block_sptr config = algorithm.get_configuration();
  config->set_value( "voxel_spacing_factor", 1.0 );
  algorithm.set_configuration( config );


  image_container_sptr volume = nullptr;
  vector_3d spacing{ 1.0, 1.0, 1.0 };
  cpu_timer timer;
  timer.start();
  algorithm.integrate(
    min_pt, max_pt,
    depth_maps, {}, cams, volume, spacing );
  timer.stop();
  std::cout << "integration time: " << timer.elapsed() << std::endl;

  evaluate_volume( volume, min_pt, max_pt, spacing, 6.0 );
}

// ----------------------------------------------------------------------------
// Test depth map integration
TEST ( integrate_depth_maps, integrate_weighted )
{
  namespace cuda = kwiver::arrows::cuda;


  std::vector< image_container_sptr > depth_maps;
  std::vector< image_container_sptr > weights;
  std::vector< camera_perspective_sptr > cams;
  vector_3d min_pt, max_pt;
  auto K = simple_camera_intrinsics(
    200, { 80, 60 }, 1.0, 0.0, {}, 160, 120 );
  make_test_data( depth_maps, cams, min_pt, max_pt, K );


  image_of< double > weight( depth_maps[ 0 ]->width(),
    depth_maps[ 0 ]->height() );
  transform_image( weight, [](double){ return 1.0; } );
  for( unsigned i = 0; i < depth_maps.size(); ++i )
  {
    weights.push_back( std::make_shared< simple_image_container >( weight ) );
  }


  cuda::integrate_depth_maps algorithm;
  config_block_sptr config = algorithm.get_configuration();
  config->set_value( "voxel_spacing_factor", 1.0 );
  algorithm.set_configuration( config );


  image_container_sptr volume = nullptr;
  vector_3d spacing{ 1.0, 1.0, 1.0 };
  cpu_timer timer;
  timer.start();
  algorithm.integrate(
    min_pt, max_pt,
    depth_maps, weights, cams, volume, spacing );
  timer.stop();
  std::cout << "integration time: " << timer.elapsed() << std::endl;

  evaluate_volume( volume, min_pt, max_pt, spacing, 6.0 );
}

// ----------------------------------------------------------------------------
// Test depth map integration
TEST ( integrate_depth_maps, integrate_distorted )
{
  namespace cuda = kwiver::arrows::cuda;


  std::vector< image_container_sptr > depth_maps;
  std::vector< camera_perspective_sptr > cams;
  vector_3d min_pt, max_pt;
  Eigen::VectorXd dist( 1 );
  dist[ 0 ] = 0.0;


  auto K = simple_camera_intrinsics(
    200, { 80, 60 }, 1.0, 0.0, dist, 160, 120 );
  make_test_data( depth_maps, cams, min_pt, max_pt, K );


  cuda::integrate_depth_maps algorithm;
  config_block_sptr config = algorithm.get_configuration();
  config->set_value( "voxel_spacing_factor", 1.0 );
  algorithm.set_configuration( config );


  image_container_sptr volume = nullptr;
  vector_3d spacing{ 1.0, 1.0, 1.0 };
  cpu_timer timer;
  timer.start();
  algorithm.integrate(
    min_pt, max_pt,
    depth_maps, {}, cams, volume, spacing );
  timer.stop();
  std::cout << "integration time: " << timer.elapsed() << std::endl;

  evaluate_volume( volume, min_pt, max_pt, spacing, 6.0 );
}
