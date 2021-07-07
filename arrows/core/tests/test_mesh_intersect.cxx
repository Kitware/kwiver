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

// ----------------------------------------------------------------------------
TEST(mesh_intersect, triangle_closest_point)
{
  point_3d a(0,0,0), b(3,0,0), c(-1,1,0);
  point_3d p1(0,0.25,2); // cp inside
  point_3d p2(0.5,-1,-1); // cp on ab
  point_3d p3(-0.5,-1,-1); // cp on a
  point_3d p4(4,-1,2); // cp on b
  point_3d p5(3,1,-2); // cp on bc
  point_3d p6(-1,2,-2); // cp on c
  point_3d p7(-1,0,-3); // cp on ac
  double u,v,dist;

  unsigned char i = mesh_triangle_closest_point(p1, a, b, c, dist, u, v);
  EXPECT_EQ(i, 7);
  EXPECT_NEAR(dist, 2., 1e-14);
  point_3d cp = mesh_triangle_closest_point(p1, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  point_3d(0,0.25,0).value()).norm(), 0., 1e-14);

  i = mesh_triangle_closest_point(p2, a, b, c, dist, u, v);
  EXPECT_EQ(i, 3);
  EXPECT_NEAR(dist, std::sqrt(2.), 1e-14);
  cp = mesh_triangle_closest_point(p2, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  point_3d(0.5,0,0).value()).norm(), 0., 1e-14);

  i = mesh_triangle_closest_point(p3, a, b, c, dist, u, v);
  EXPECT_EQ(i, 1);
  EXPECT_NEAR(dist, 1.5, 1e-14);
  cp = mesh_triangle_closest_point(p3, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  a.value()).norm(), 0., 1e-14);

  i = mesh_triangle_closest_point(p4, a, b, c, dist, u, v);
  EXPECT_EQ(i, 2);
  EXPECT_NEAR(dist, std::sqrt(6.), 1e-14);
  cp = mesh_triangle_closest_point(p4, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  b.value()).norm(), 0., 1e-14);

  i = mesh_triangle_closest_point(p5, a, b, c, dist, u, v);
  EXPECT_EQ(i, 6);
  point_3d exp_cp(2.75 + 0.25/17., 1./17., 0);
  EXPECT_NEAR(dist, (exp_cp.value() - p5.value()).norm(), 1e-14);
  cp = mesh_triangle_closest_point(p5, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  exp_cp.value()).norm(), 0., 1e-14);

  i = mesh_triangle_closest_point(p6, a, b, c, dist, u, v);
  EXPECT_EQ(i, 4);
  EXPECT_NEAR(dist, std::sqrt(5.), 1e-14);
  cp = mesh_triangle_closest_point(p6, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  c.value()).norm(), 0., 1e-14);

  i = mesh_triangle_closest_point(p7, a, b, c, dist, u, v);
  EXPECT_EQ(i, 5);
  EXPECT_NEAR(dist, std::sqrt(9.5), 1e-14);
  cp = mesh_triangle_closest_point(p7, a, b, c, dist);
  EXPECT_NEAR( (cp.value() -  point_3d(-0.5,0.5,0).value()).norm(), 0., 1e-14);
}
