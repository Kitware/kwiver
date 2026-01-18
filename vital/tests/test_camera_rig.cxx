// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test camera_rig class

#include <tests/test_gtest.h>

#include <vital/types/camera_perspective.h>
#include <vital/types/camera_rig.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( camera_rig, default_constructor )
{
  camera_rig rig;
  EXPECT_TRUE( rig.empty() );
  EXPECT_EQ( 0, rig.size() );
}

// ----------------------------------------------------------------------------
TEST ( camera_rig, add_camera )
{
  camera_rig rig;

  auto cam1 = std::make_shared< simple_camera_perspective >();
  auto cam2 = std::make_shared< simple_camera_perspective >();

  rig.add( "camera1", cam1 );
  EXPECT_EQ( 1, rig.size() );
  EXPECT_FALSE( rig.empty() );

  rig.add( "camera2", cam2 );
  EXPECT_EQ( 2, rig.size() );
}

// ----------------------------------------------------------------------------
TEST ( camera_rig, get_camera )
{
  camera_rig rig;

  auto cam1 = std::make_shared< simple_camera_perspective >();
  rig.add( "test_cam", cam1 );

  auto retrieved = rig.camera( "test_cam" );
  EXPECT_EQ( cam1, retrieved );

  auto not_found = rig.camera( "nonexistent" );
  EXPECT_EQ( nullptr, not_found );
}

// ----------------------------------------------------------------------------
TEST ( camera_rig, remove_camera )
{
  camera_rig rig;

  auto cam1 = std::make_shared< simple_camera_perspective >();
  rig.add( "to_remove", cam1 );
  EXPECT_EQ( 1, rig.size() );

  auto removed = rig.remove( "to_remove" );
  EXPECT_EQ( cam1, removed );
  EXPECT_EQ( 0, rig.size() );
  EXPECT_TRUE( rig.empty() );

  // Removing non-existent camera returns nullptr
  auto not_found = rig.remove( "nonexistent" );
  EXPECT_EQ( nullptr, not_found );
}

// ----------------------------------------------------------------------------
TEST ( camera_rig, cameras_collection )
{
  camera_rig rig;

  auto cam1 = std::make_shared< simple_camera_perspective >();
  auto cam2 = std::make_shared< simple_camera_perspective >();

  rig.add( "left", cam1 );
  rig.add( "right", cam2 );

  auto const& cameras = rig.cameras();
  EXPECT_EQ( 2, cameras.size() );
  EXPECT_NE( cameras.find( "left" ), cameras.end() );
  EXPECT_NE( cameras.find( "right" ), cameras.end() );
}

// ----------------------------------------------------------------------------
TEST ( camera_rig_stereo, constructor )
{
  auto left_cam = std::make_shared< simple_camera_perspective >();
  auto right_cam = std::make_shared< simple_camera_perspective >();

  camera_rig_stereo stereo_rig( left_cam, right_cam );

  EXPECT_EQ( 2, stereo_rig.size() );
  EXPECT_EQ( left_cam, stereo_rig.left() );
  EXPECT_EQ( right_cam, stereo_rig.right() );
}

// ----------------------------------------------------------------------------
TEST ( camera_rig_stereo, access_by_tag )
{
  auto left_cam = std::make_shared< simple_camera_perspective >();
  auto right_cam = std::make_shared< simple_camera_perspective >();

  camera_rig_stereo stereo_rig( left_cam, right_cam );

  // Should also be accessible via camera() method
  EXPECT_EQ( left_cam, stereo_rig.camera( "left" ) );
  EXPECT_EQ( right_cam, stereo_rig.camera( "right" ) );
}
