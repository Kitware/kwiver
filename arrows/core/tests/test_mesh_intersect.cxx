// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <gtest/gtest.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <arrows/core/mesh_intersect.h>
#include <Eigen/Geometry>

using namespace kwiver::vital;
using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(mesh_intersect, intersect_triangle)
{
  point_3d p(2,3,4);
  vector_3d d(-2,-3,-4);
  d.normalize();
  point_3d a(2,0,0), b(-1,1,0), c(0,-3,0);

  vector_3d n((b.value()-a.value()).cross(c.value()-a.value()));
  double dist = std::numeric_limits<double>::infinity();
  double dist2 = dist;
  double u1, v1, u2, v2;

  EXPECT_TRUE( mesh_intersect_triangle(p, d, a, b, c, n, dist, u1, v1) );

  vector_3d r1 = p.value() + dist*d;
  vector_3d r2 = (1 - u1 - v1)*a.value() + u1*b.value() + v1*c.value();

  EXPECT_NEAR( (r2 - r1).norm(), 0.0, 1e-14 );

  EXPECT_TRUE( mesh_intersect_triangle_min_dist(p, d, a, b, c, n, dist2, u2, v2) );

  dist2 -= 0.001;

  EXPECT_FALSE( mesh_intersect_triangle_min_dist(p, d, a, b, c, n, dist2, u2, v2) );
}
