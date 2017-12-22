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
 * \brief test camera interpolation
 */

#include <arrows/core/interpolate_camera.h>

#include <gtest/gtest.h>

#include <iostream>

#define _USE_MATH_DEFINES
#include <cmath>

#if defined M_PIl
#define LOCAL_PI M_PIl
#else
#define LOCAL_PI M_PI
#endif

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(interpolate_camera, interpolation)
{
  using namespace kwiver;
  using namespace std;
  using vital::vector_3d;
  using vital::vector_4d;
  using vital::rotation_d;
  using vital::simple_camera;

  const double pi = LOCAL_PI;
  simple_camera a(vector_3d(-1, -1, -1),
                  rotation_d(vector_4d(0, 0, 0, 1))),  // no rotation
                b(vector_3d(3, 3, 3),
                  rotation_d(-pi / 2, vector_3d(0, 0, 1))),  // rotated around z-axis 90 degrees
                c;
  c = kwiver::arrows::interpolate_camera(a, b, 0.5);

  cerr << "a.rotation: " << a.rotation().axis() << ' '  << a.rotation().angle() << endl;
  cerr << "b.rotation: " << b.rotation().axis() << ' '  << b.rotation().angle() << endl;

  cerr << "c.center  : " << c.center() << endl;
  EXPECT_NEAR( 1.0, c.center().x(), 1e-16 );
  EXPECT_NEAR( 1.0, c.center().y(), 1e-16 );
  EXPECT_NEAR( 1.0, c.center().z(), 1e-16 );

  cerr << "c.rotation (aa): " << c.rotation().axis() << ' ' << c.rotation().angle() << endl;
  cerr << "c.rotation  (q): " << c.rotation() << endl;
  EXPECT_NEAR(  0.0, c.rotation().axis().x(), 1e-15 );
  EXPECT_NEAR(  0.0, c.rotation().axis().y(), 1e-15 );
  EXPECT_NEAR( -1.0, c.rotation().axis().z(), 1e-15 );
  EXPECT_NEAR( pi / 4.0, c.rotation().angle(), 1e-15 );
}

// ----------------------------------------------------------------------------
TEST(interpolate_camera, multiple_interpolations)
{
  using namespace kwiver;
  using namespace std;
  using vital::vector_3d;
  using vital::vector_4d;
  using vital::rotation_d;
  using vital::simple_camera;

  const double pi = LOCAL_PI;
  simple_camera a(vector_3d(-1, -1, -1),
                  rotation_d(vector_4d(0, 0, 0, 1))),        // no rotation
                b(vector_3d(3, 3, 3),
                  rotation_d(-pi / 2, vector_3d(0, 0, 1)));  // rotated around z-axis 90 degrees
  vector<simple_camera> cams;

  cams.push_back(a);
  kwiver::arrows::interpolated_cameras(a, b, 3, cams);
  cams.push_back(b);

  cerr << "Vector size: " << cams.size() << endl;
  EXPECT_EQ(5, cams.size());
  for (auto const& cam : cams)
  {
    cerr << "\t" << cam.center() << " :: " << cam.rotation().axis() << " " << cam.rotation().angle() << endl;
  }

  simple_camera i1 = cams[1],
           i2 = cams[2],
           i3 = cams[3];
  cerr << "i1 .25 c : " << i1.center() << " :: " << i1.rotation().axis() << ' ' << i1.rotation().angle() << endl;
  cerr << "i2 .25 c : " << i2.center() << " :: " << i2.rotation().axis() << ' ' << i2.rotation().angle() << endl;
  cerr << "i3 .25 c : " << i3.center() << " :: " << i3.rotation().axis() << ' ' << i3.rotation().angle() << endl;

  EXPECT_NEAR(  0.0, i1.center().x(), 1e-15 );
  EXPECT_NEAR(  0.0, i1.center().y(), 1e-15 );
  EXPECT_NEAR(  0.0, i1.center().z(), 1e-15 );
  EXPECT_NEAR(  0.0, i1.rotation().axis().x(), 1e-15 );
  EXPECT_NEAR(  0.0, i1.rotation().axis().y(), 1e-15 );
  EXPECT_NEAR( -1.0, i1.rotation().axis().z(), 1e-15 );
  EXPECT_NEAR( pi / 8.0, i1.rotation().angle(), 1e-15 );

  EXPECT_NEAR(  1.0, i2.center().x(), 1e-15 );
  EXPECT_NEAR(  1.0, i2.center().y(), 1e-15 );
  EXPECT_NEAR(  1.0, i2.center().z(), 1e-15 );
  EXPECT_NEAR(  0.0, i2.rotation().axis().x(), 1e-15 );
  EXPECT_NEAR(  0.0, i2.rotation().axis().y(), 1e-15 );
  EXPECT_NEAR( -1.0, i2.rotation().axis().z(), 1e-15 );
  EXPECT_NEAR( pi / 4.0, i2.rotation().angle(), 1e-15 );

  EXPECT_NEAR(  2.0, i3.center().x(), 1e-15 );
  EXPECT_NEAR(  2.0, i3.center().y(), 1e-15 );
  EXPECT_NEAR(  2.0, i3.center().z(), 1e-15 );
  EXPECT_NEAR(  0.0, i3.rotation().axis().x(), 1e-15 );
  EXPECT_NEAR(  0.0, i3.rotation().axis().y(), 1e-15 );
  EXPECT_NEAR( -1.0, i3.rotation().axis().z(), 1e-15 );
  EXPECT_NEAR( 3.0 * pi / 8.0, i3.rotation().angle(), 1e-15 );
}

// TODO: Full test case for camera interpolation would be to create a full,
// connected ring of cameras looking at a point (or even multiple loops), and
// checking that the rotation angle returned by the getter function of the
// rotation between each cameras is less than pi. It would be even more
// detailed to do this for camera rings along each major axis plane, as well
// as for a stare-point that is not at along the axis of rotation for the
// camera ring.
