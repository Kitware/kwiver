/*ckwg +29
 * Copyright 2014-2018 by Kitware, Inc.
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

#include <vital/types/camera_affine.h>
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
TEST(camera_affine, clone)
{
  simple_camera_affine cam({0, 0, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0}, {10, 10}, 10, 10);
  cam.set_viewing_distance(50);
  auto cam_clone = std::dynamic_pointer_cast<camera_affine>( cam.clone() );
  EXPECT_MATRIX_EQ( cam.center(), cam_clone->center() );
  EXPECT_MATRIX_EQ( cam.get_matrix(), cam_clone->get_matrix() );
  EXPECT_EQ( cam.get_viewing_distance(), cam_clone->get_viewing_distance() );
}

// ----------------------------------------------------------------------------
TEST(camera_affine, projection)
{
  simple_camera_affine cam({1, 1, -1},  // ray_dir
                           {0, 0, 1},     // up vector
                           {0, 0, 0},     // star point
                           {50, 50},      // its projection in the camera
                           {1, 1},        // scale
                           100, 100);     // image dimension

  vector_3d pt(0, 0, 0);
  auto res = cam.project(pt);
  EXPECT_MATRIX_EQ( vector_2d(50, 50), res );

  vector_3d pt2(1, 1, 0);
  res = cam.project(pt2);
  double d = sqrt(2.0) * sin(0.6154797086703873);
  EXPECT_MATRIX_EQ( vector_2d(50, 50.0 - d), res);
}

// ----------------------------------------------------------------------------
TEST(camera_affine, depth)
{
  simple_camera_affine cam({1, 1, -1},  // ray_dir
                           {0, 0, 1},     // up vector
                           {0, 0, 0},     // star point
                           {50, 50},      // its projection in the camera
                           {1, 1},        // scale
                           100, 100);     // image dimension
  double view_dist = 10.0;
  cam.set_viewing_distance(view_dist);

  vector_3d pt(0, 0, 0);
  auto res = cam.depth(pt);
  EXPECT_NEAR( view_dist, res, 1e-5 );

  vector_3d pt2(1, 1, 0);
  double d = sqrt(2.0) * cos(0.6154797086703873);
  res = cam.depth(pt2);
  EXPECT_NEAR( view_dist + d, res, 1e-5);
}
