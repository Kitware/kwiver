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
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
kwiver::vital::mesh_sptr
generate_mesh()
{
  std::unique_ptr< mesh_vertex_array_base >
  verts( new mesh_vertex_array< 3 >( { { 0, 0, 0 }, { 2, 0, 0 }, { 0, 1, 0 },
                                       { 0, 0, 1 } } ) );

  std::unique_ptr< mesh_regular_face_array< 3 > >
  faces( new mesh_regular_face_array< 3 >( { { 0, 2, 1 }, { 0, 1, 3 },
                                             { 0, 3, 2 }, { 1, 2, 3 } } ) );

  return std::make_shared< mesh >( std::move( verts ), std::move( faces ) );
}

// ----------------------------------------------------------------------------
TEST ( mesh_intersect, intersect_triangle )
{
  point_3d p( 2, 3, 4 );
  vector_3d d( -2, -3, -4 );
  d.normalize();
  point_3d a( 2, 0, 0 ), b( -1, 1, 0 ), c( 0, -3, 0 );

  vector_3d n( ( b.value() - a.value() ).cross( c.value() - a.value() ) );
  double dist = std::numeric_limits< double >::infinity();
  double dist2 = dist;
  double u1, v1, u2, v2;

  EXPECT_TRUE( mesh_intersect_triangle( p, d, a, b, c, n, dist, u1, v1 ) );

  vector_3d r1 = p.value() + dist * d;
  vector_3d r2 = ( 1 - u1 - v1 ) * a.value() + u1 * b.value() + v1 * c.value();

  EXPECT_NEAR( ( r2 - r1 ).norm(), 0.0, 1e-14 );

  EXPECT_TRUE( mesh_intersect_triangle_min_dist( p, d, a, b, c, n, dist2, u2,
                                                 v2 ) );

  dist2 -= 0.001;

  EXPECT_FALSE( mesh_intersect_triangle_min_dist( p, d, a, b, c, n, dist2, u2,
                                                  v2 ) );

  // Test triangle in different plane
  p = { 2, 1, 1 };
  d = { -1, 0, 0 };
  a = { 0, 0, 0 }, b = { 0, 3, 0 }, c = { 0, 0, 2 };
  n = ( b.value() - a.value() ).cross( c.value() - a.value() );
  dist = std::numeric_limits< double >::infinity();

  EXPECT_TRUE( mesh_intersect_triangle( p, d, a, b, c, n, dist, u1, v1 ) );

  r1 = p.value() + dist * d;
  r2 = ( 1 - u1 - v1 ) * a.value() + u1 * b.value() + v1 * c.value();

  EXPECT_NEAR( ( r2 - r1 ).norm(), 0.0, 1e-14 );
}

// ----------------------------------------------------------------------------
TEST ( mesh_intersect, triangle_closest_point )
{
  point_3d a( 0, 0, 0 ), b( 3, 0, 0 ), c( -1, 1, 0 );
  point_3d p1( 0, 0.25, 2 ); // cp inside
  point_3d p2( 0.5, -1, -1 ); // cp on ab
  point_3d p3( -0.5, -1, -1 ); // cp on a
  point_3d p4( 4, -1, 2 ); // cp on b
  point_3d p5( 3, 1, -2 ); // cp on bc
  point_3d p6( -1, 2, -2 ); // cp on c
  point_3d p7( -1, 0, -3 ); // cp on ac
  double u, v, dist;

  unsigned char i = mesh_triangle_closest_point( p1, a, b, c, dist, u, v );
  EXPECT_EQ( i, 7 );
  EXPECT_NEAR( dist, 2., 1e-14 );
  point_3d cp = mesh_triangle_closest_point( p1, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  point_3d( 0, 0.25,
                                         0 ).value() ).norm(), 0., 1e-14 );

  i = mesh_triangle_closest_point( p2, a, b, c, dist, u, v );
  EXPECT_EQ( i, 3 );
  EXPECT_NEAR( dist, std::sqrt( 2. ), 1e-14 );
  cp = mesh_triangle_closest_point( p2, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  point_3d( 0.5, 0,
                                         0 ).value() ).norm(), 0., 1e-14 );

  i = mesh_triangle_closest_point( p3, a, b, c, dist, u, v );
  EXPECT_EQ( i, 1 );
  EXPECT_NEAR( dist, 1.5, 1e-14 );
  cp = mesh_triangle_closest_point( p3, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  a.value() ).norm(), 0., 1e-14 );

  i = mesh_triangle_closest_point( p4, a, b, c, dist, u, v );
  EXPECT_EQ( i, 2 );
  EXPECT_NEAR( dist, std::sqrt( 6. ), 1e-14 );
  cp = mesh_triangle_closest_point( p4, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  b.value() ).norm(), 0., 1e-14 );

  i = mesh_triangle_closest_point( p5, a, b, c, dist, u, v );
  EXPECT_EQ( i, 6 );
  point_3d exp_cp( 2.75 + 0.25 / 17., 1. / 17., 0 );
  EXPECT_NEAR( dist, ( exp_cp.value() - p5.value() ).norm(), 1e-14 );
  cp = mesh_triangle_closest_point( p5, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  exp_cp.value() ).norm(), 0., 1e-14 );

  i = mesh_triangle_closest_point( p6, a, b, c, dist, u, v );
  EXPECT_EQ( i, 4 );
  EXPECT_NEAR( dist, std::sqrt( 5. ), 1e-14 );
  cp = mesh_triangle_closest_point( p6, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  c.value() ).norm(), 0., 1e-14 );

  i = mesh_triangle_closest_point( p7, a, b, c, dist, u, v );
  EXPECT_EQ( i, 5 );
  EXPECT_NEAR( dist, std::sqrt( 9.5 ), 1e-14 );
  cp = mesh_triangle_closest_point( p7, a, b, c, dist );
  EXPECT_NEAR( ( cp.value() -  point_3d( -0.5, 0.5,
                                         0 ).value() ).norm(), 0., 1e-14 );
}

// ----------------------------------------------------------------------------
TEST ( mesh_intersect, mesh_closest_point )
{
  mesh_sptr mesh = generate_mesh();

  point_3d p1( 1, 1, 1 ); // On face 3
  point_3d p2( 1, 0, -1 ); // On edge of face 0 and 1
  point_3d p3( 0, 1, 1 ); // On edge of face 2
  point_3d p4( -0.1, -0.1, -0.1 ); // At corner shared by 0, 1, and 2
  point_3d p5( 0.1, 0.25, 0.25 ); // Inside mesh, on face 2
  point_3d cp( 0, 0, 0 );
  double u = 0., v = 0.;

  EXPECT_EQ( mesh_closest_point( p1, *mesh, cp, u, v ), 3 );
  EXPECT_NEAR( u, 1. / 3., 1e-14 );
  EXPECT_NEAR( v, 1. / 3., 1e-14 );
  point_3d exp_cp( 2. / 3., 1. / 3., 1. / 3. );
  EXPECT_NEAR( ( cp.value() -  exp_cp.value() ).norm(), 0., 1e-14 );

  EXPECT_EQ( mesh_closest_point( p2, *mesh, cp, u, v ), 0 );
  EXPECT_NEAR( u, 0.0, 1e-14 );
  EXPECT_NEAR( v, 0.5, 1e-14 );
  exp_cp.set_value( { 1., 0., 0. } );
  EXPECT_NEAR( ( cp.value() -  exp_cp.value() ).norm(), 0., 1e-14 );

  EXPECT_EQ( mesh_closest_point( p3, *mesh, cp, u, v ), 2 );
  EXPECT_NEAR( u, 0.5, 1e-14 );
  EXPECT_NEAR( v, 0.5, 1e-14 );
  exp_cp.set_value( { 0.0, 0.5, 0.5 } );
  EXPECT_NEAR( ( cp.value() -  exp_cp.value() ).norm(), 0., 1e-14 );

  EXPECT_EQ( mesh_closest_point( p4, *mesh, cp, u, v ), 0 );
  EXPECT_NEAR( u, 0.0, 1e-14 );
  EXPECT_NEAR( v, 0.0, 1e-14 );
  exp_cp.set_value( { 0.0, 0.0, 0.0 } );
  EXPECT_NEAR( ( cp.value() -  exp_cp.value() ).norm(), 0., 1e-14 );

  EXPECT_EQ( mesh_closest_point( p5, *mesh, cp, u, v ), 2 );
  EXPECT_NEAR( u, 0.25, 1e-14 );
  EXPECT_NEAR( v, 0.25, 1e-14 );
  exp_cp.set_value( { 0.0, 0.25, 0.25 } );
  EXPECT_NEAR( ( cp.value() -  exp_cp.value() ).norm(), 0., 1e-14 );
}

// ----------------------------------------------------------------------------

TEST ( mesh_intersect, mesh_intersect )
{
  mesh_sptr mesh = generate_mesh();

  point_3d p1( 1, 1, 1 ); // Intersects face 3
  vector_3d d1( 0, -1, -1 );
  d1.normalize();
  point_3d p2( -1, 0.5, 0.5 ); // Intersects edge of face 2
  vector_3d d2( 1, 0, 0 );
  point_3d p3( 1, 0.4, -0.5 ); // Intersects face 0
  vector_3d d3( 0, 0, 1 );
  point_3d p4( 2, 1, 1 ); // Parallel to face 3
  vector_3d d4( -2, 1, 1 );
  d4.normalize();
  double dist = std::numeric_limits< double >::infinity();
  double u = 0., v = 0.;

  // Errors out due to lack of normals
  EXPECT_EQ( mesh_intersect( p1, d1, *mesh, dist, u, v ), -1 );

  mesh->compute_face_normals( false );
  EXPECT_EQ( mesh_intersect( p1, d1, *mesh, dist, u, v ), 3 );
  EXPECT_NEAR( u, 0.25, 1e-14 );
  EXPECT_NEAR( v, 0.25, 1e-14 );
  EXPECT_NEAR( dist, 0.75 * std::sqrt( 2. ), 1e-14 );

  EXPECT_EQ( mesh_intersect( p2, d2, *mesh, dist, u, v ), 2 );
  EXPECT_NEAR( u, 0.5, 1e-14 );
  EXPECT_NEAR( v, 0.5, 1e-14 );
  EXPECT_NEAR( dist, 1.0, 1e-14 );

  EXPECT_EQ( mesh_intersect( p3, d3, *mesh, dist, u, v ), 0 );
  EXPECT_NEAR( u, 0.4, 1e-14 );
  EXPECT_NEAR( v, 0.5, 1e-14 );
  EXPECT_NEAR( dist, 0.5, 1e-14 );

  EXPECT_EQ( mesh_intersect( p4, d4, *mesh, dist, u, v ), -1 );
}
