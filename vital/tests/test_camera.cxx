/*ckwg +29
 * Copyright 2014-2017 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief test core camera class
 */

#include <test_eigen.h>

#include <vital/types/camera.h>
#include <vital/io/camera_io.h>

#include <iostream>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(camera, clone)
{
  vector_2d pp{ 300, 400 };
  simple_camera_intrinsics K{ 1000, pp };
  simple_camera cam{ vector_3d{ 3, -4, 7 }, rotation_d{}, K };

  auto cam_clone = cam.clone();
  EXPECT_MATRIX_EQ( cam.center(), cam_clone->center() );
  EXPECT_MATRIX_EQ( cam.rotation().quaternion(),
                    cam_clone->rotation().quaternion() );
  EXPECT_EQ( cam.intrinsics(), cam_clone->intrinsics() );
}

// ----------------------------------------------------------------------------
TEST(camera, clone_look_at)
{
  vector_2d pp{ 300, 400 };
  simple_camera_intrinsics K{ 1000, pp };
  vector_3d focus{ 0, 1, -2 };

  auto cam_in = simple_camera{ vector_3d{ 3, -4, 7 }, rotation_d{}, K };
  auto cam = cam_in.clone_look_at( focus );

  EXPECT_MATRIX_NEAR( pp, cam->project( focus ), 1e-12 );

  vector_2d ifocus_up = cam->project( focus + vector_3d{ 0, 0, 2 } );
  vector_2d clone_look_at_vertical = ifocus_up - pp;
  EXPECT_NEAR( 0.0, clone_look_at_vertical.x(), 1e-12 )
    << "clone_look_at vertical should project vertical";
  // "Up" in image space is actually negative Y because the Y axis is inverted
  EXPECT_GT( 0.0, clone_look_at_vertical.y() )
    << "clone_look_at up should project up";
}

// ----------------------------------------------------------------------------
TEST(camera, look_at)
{
  vector_2d pp{ 300, 400 };
  simple_camera_intrinsics K{ 1000, pp };
  vector_3d focus{ 0, 1, -2 };
  simple_camera cam{ vector_3d{ 3, -4, 7 }, rotation_d{}, K };
  cam.look_at( focus );

  EXPECT_MATRIX_NEAR( pp, cam.project( focus ), 1e-12 );

  vector_2d ifocus_up = cam.project( focus + vector_3d{ 0, 0, 2 } );
  vector_2d look_at_vertical = ifocus_up - pp;
  EXPECT_NEAR( 0.0, look_at_vertical.x(), 1e-12 )
    << "look_at vertical should project vertical";
  // "Up" in image space is actually negative Y because the Y axis is inverted
  EXPECT_GT( 0.0, look_at_vertical.y() )
    << "look_at up should project up";
}

// ----------------------------------------------------------------------------
TEST(camera, projection)
{
  vector_2d pp{ 300, 400 };
  simple_camera_intrinsics K{ 1000, pp };
  vector_3d focus{ 0, 1, -2 };
  simple_camera cam{ vector_3d{ 3, -4, 7 }, rotation_d{}, K };
  cam.look_at( focus );

  matrix_3x4d P{ cam.as_matrix() };

  auto test = [&]( vector_3d const& test_pt ){
    vector_4d test_hpt{ test_pt.x(), test_pt.y(), test_pt.z(), 1.0 };

    vector_3d proj_hpt = P * test_hpt;
    vector_2d proj_pt{ proj_hpt.x() / proj_hpt.z(),
                       proj_hpt.y() / proj_hpt.z() };

    EXPECT_MATRIX_NEAR( proj_pt, cam.project( test_pt ), 1e-12 )
      << "Camera projection should be equivalent to matrix multiplication";
  };

  test( { 1, 2, 3 } );
  test( { 0, 1, -2 } );
  test( { 5, -42, 67 } );
}
