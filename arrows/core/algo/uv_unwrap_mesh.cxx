// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation for mesh uv unwrapping

#include "uv_unwrap_mesh.h"

#include <algorithm>
#include <cmath>
#include <Eigen/Dense>
#include <iostream>
#include <numeric>

#include <vital/exceptions.h>
#include <vital/types/matrix.h>
#include <vital/vital_config.h>

using namespace kwiver::vital;

namespace {

/// This structure is used to represent a 2D triangle
struct triangle_t
{
  kwiver::vital::vector_2d a = { 0, 0 };
  kwiver::vital::vector_2d b = { 0, 0 };
  kwiver::vital::vector_2d c = { 0, 0 };
  unsigned int face_id = 0;
  double height = 0;
  double width = 0;
};

} // namespace

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------
class uv_unwrap_mesh::priv
{
public:
  priv( uv_unwrap_mesh& parent )
    : parent( parent )
  {}

  uv_unwrap_mesh& parent;
  double c_spacing()        { return parent.c_spacing; }
  bool   c_sort_descending() { return parent.c_sort_descending; }
  bool   c_compact()        { return parent.c_compact; }
  double c_padding_ratio()  { return parent.c_padding_ratio; }
  int    c_iterations()     { return parent.c_iterations; }

  ~priv() = default;
};

// ----------------------------------------------------------------------------
void
uv_unwrap_mesh
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.core.uv_unwrap_mesh" );
}

// Destructor
uv_unwrap_mesh
::~uv_unwrap_mesh()
{}

// Check that the algorithm's configuration vital::config_block is valid
bool
uv_unwrap_mesh
::check_configuration( vital::config_block_sptr config ) const
{
  bool ok = true;

  double spacing =
    config->get_value< double >( "spacing", d_->c_spacing() );
  if( spacing <= 0.0 || spacing > 1.0 )
  {
    LOG_ERROR(
      logger(),
      "spacing parameter is " << spacing
                              << ", needs to be in (0.0, 1.0]." );
    ok = false;
  }

  double padding_ratio =
    config->get_value< double >( "padding_ratio", d_->c_padding_ratio() );
  if( padding_ratio <= 0.0 || padding_ratio > 1.0 )
  {
    LOG_ERROR(
      logger(),
      "padding_ratio parameter is " << padding_ratio
                                    << ", needs to be in (0.0, 1.0]." );
    ok = false;
  }

  int iterations =
    config->get_value< int >( "iterations", d_->c_iterations() );
  if( iterations < 1 )
  {
    LOG_ERROR(
      logger(),
      "iterations parameter is " << iterations
                                 << ", needs to be >= 1." );
    ok = false;
  }

  return ok;
}

// Unwrap a mesh
void
uv_unwrap_mesh
::unwrap( kwiver::vital::mesh_container_sptr mesh_container ) const
{
  auto mesh = mesh_container->mesh();
  if( mesh.faces().regularity() != 3 )
  {
    VITAL_THROW(
      algorithm_exception, this->interface_name(), this->plugin_name(),
      "This algorithm expects a regular mesh with triangular faces." );
  }

  auto const& faces = mesh.faces();
  auto const& vertices =
    dynamic_cast< const mesh_vertex_array< 3 >& >( mesh.vertices() );

  bool const use_compact        = d_->c_compact();
  bool const sort_descending    = d_->c_sort_descending();
  double const spacing          = d_->c_spacing();
  double const padding_ratio    = d_->c_padding_ratio();
  int const iterations          = d_->c_iterations();

  // Map each triangle in 2D. The longest edge is placed horizontally with its
  // left point at (0, 0) and its apex pointing up.
  auto const num_faces = mesh.num_faces();
  std::vector< triangle_t > triangles( num_faces );
  double total_area = 0.0;

  for( unsigned int f = 0; f < num_faces; ++f )
  {
    // face 3D points
    auto const pt1 = vertices[ faces( f, 0 ) ];
    auto const pt2 = vertices[ faces( f, 1 ) ];
    auto const pt3 = vertices[ faces( f, 2 ) ];

    // triangle edges
    auto const pt1pt2 = pt2 - pt1;
    auto const pt1pt3 = pt3 - pt1;
    auto const pt2pt3 = pt3 - pt2;

    auto const n12 = pt1pt2.norm();
    auto const n13 = pt1pt3.norm();
    auto const n23 = pt2pt3.norm();

    // Find the longest edge and assign it to AB, C is the other point.
    vector_3d AB, AC;
    int longest_edge;
    if( n12 >= n13 && n12 >= n23 )
    {
      // pt1 is A, pt2 is B, pt3 is C
      AB = pt1pt2;
      AC = pt1pt3;
      longest_edge = 0;
    }
    else if( n23 >= n13 )
    {
      // pt1 is C, pt2 is A, pt3 is B
      AB = pt2pt3;
      AC = -pt1pt2;
      longest_edge = 1;
    }
    else
    {
      // pt1 is B, pt2 is C, pt3 is A
      AB = -pt1pt3;
      AC = -pt2pt3;
      longest_edge = 2;
    }

    // Transform the face to 2D.
    double const w = AB.norm();
    double const nAC = AC.norm();
    if( w == 0.0 || nAC == 0.0 )
    {
      triangles[ f ] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, f, 0, 0 };
      continue;
    }

    vector_2d a( 0.0, 0.0 );
    vector_2d b( w, 0.0 );
    double proj = AC.dot( AB ) / w;
    if( std::isnan( proj ) ) { proj = 0; }

    double const h = ( AC - proj * AB.normalized() ).norm();

    total_area += w * h;

    if( longest_edge == 0 )
    {
      triangles[ f ] = { a, b, { proj, h }, f, h, w };
    }
    else if( longest_edge == 1 )
    {
      triangles[ f ] = { { proj, h }, a, b, f, h, w };
    }
    else
    {
      triangles[ f ] = { b, { proj, h }, a, f, h, w };
    }
  }

  // Sort triangles by height (then width as tiebreak).
  std::vector< unsigned int > face_indices( num_faces );
  std::iota( face_indices.begin(), face_indices.end(), 0 );

  if( sort_descending )
  {
    std::sort(
      face_indices.begin(), face_indices.end(),
      [ &triangles ]( unsigned int i, unsigned int j ){
        auto const& ti = triangles[ i ], & tj = triangles[ j ];
        return ti.height > tj.height ||
               ( ti.height == tj.height && ti.width > tj.width );
      } );
  }
  else
  {
    std::sort(
      face_indices.begin(), face_indices.end(),
      [ &triangles ]( unsigned int i, unsigned int j ){
        return triangles[ i ].height < triangles[ j ].height;
      } );
  }

  // Estimate max width for a roughly square texture atlas.
  // When compact=false, use the legacy ceil-based formula so that
  // output is bit-for-bit identical to the original implementation.
  double max_width;
  double margin;
  if( !use_compact )
  {
    max_width = std::ceil( std::sqrt( total_area ) );
    margin = max_width * spacing;

    double correction = 0.0;
    for( auto const& t : triangles )
    {
      correction += margin * ( t.width + t.height );
    }
    max_width = std::ceil( std::sqrt( total_area + correction ) );
  }
  else
  {
    max_width = std::sqrt( total_area );
    margin = max_width * spacing;

    double const m2 = margin * margin;
    double correction = 0.0;
    for( auto const& t : triangles )
    {
      if( !t.width || !t.height ) { continue; }
      correction += margin * ( t.width + t.height ) + m2;
    }
    max_width = std::sqrt( total_area + correction );
  }

  double const padding = margin * padding_ratio;

  // 180-degree rotation helper (for compact packing)
  vital::matrix_2x2d rot_180;
  rot_180 << -1, 0, 0, -1;

  auto rotate_triangle = [ &rot_180 ]( triangle_t& t ){
                           vector_2d box_center = { t.width / 2.0,
                                                    t.height / 2.0 };
                           t.a = ( rot_180 * ( t.a - box_center ) ) +
                                 box_center;
                           t.b = ( rot_180 * ( t.b - box_center ) ) +
                                 box_center;
                           t.c = ( rot_180 * ( t.c - box_center ) ) +
                                 box_center;
                         };

  // When compact, simulate packing to find the width with minimum wasted space.
  if( use_compact )
  {
    auto const max_height = triangles[ face_indices.back() ].height;
    double min_waste = 1.0, best_width = max_width;

    for( int iter = 0; iter < iterations; ++iter )
    {
      double h = margin, w = margin, last_height = margin;
      auto prev_tri = triangles[ face_indices.front() ];
      std::vector< double > xs, prev_xs;
      int count = -1;

      for( auto f : face_indices )
      {
        auto t = triangles[ f ];
        if( t.width <= 0.0 || t.height <= 0.0 ) { continue; }
        ++count;

        bool rot_tri = ( count % 2 != 0 );
        if( rot_tri ) { rotate_triangle( t ); }

        if( count > 0 && w > margin )
        {
          xs = { t.a( 0 ), t.b( 0 ), t.c( 0 ) };
          std::sort( xs.begin(), xs.end() );
          prev_xs = { prev_tri.a( 0 ), prev_tri.b( 0 ), prev_tri.c( 0 ) };
          std::sort( prev_xs.begin(), prev_xs.end() );

          double s = 0.0;
          if( rot_tri )
          {
            double const
            d1 = xs[ 1 ] - xs[ 0 ],
              d2 = ( prev_xs[ 2 ] - prev_xs[ 1 ] ) * t.height / prev_tri.height;
            s = std::min( d1, d2 );
          }
          else
          {
            double const
            d1 = prev_xs[ 2 ] - prev_xs[ 1 ],
              x  = prev_xs[ 1 ] + t.height * d1 / prev_tri.height,
              d2 = prev_xs[ 2 ] + xs[ 1 ] - x;
            s = std::min( d1, d2 );
          }
          w -= s - padding;
        }

        if( w + t.width + margin > max_width )
        {
          w = margin;
          h += last_height + margin;
        }

        last_height = t.height;
        w += t.width + margin;
        prev_tri = t;
      }
      if( w != margin ) { h += last_height + margin; }

      double const
      max_dim = std::max( h, max_width ),
        area    = max_dim * max_dim,
        waste   = max_dim * std::abs( h - max_width ) +
                  max_height * ( max_width - w );

      if( waste / area < min_waste )
      {
        min_waste  = waste / area;
        best_width = max_width;
      }
      max_width = std::sqrt( max_width * h );
    }
    max_width = best_width;
  }

  // Pack triangles.
  std::vector< vector_2d > tcoords( num_faces * 3 );
  double current_u = margin;
  double current_v = margin;
  double next_v    = current_v;
  double max_u     = 0.0, max_v = 0.0;
  vector_2d shift( 0.0, 0.0 );

  auto prev_tri = triangles[ face_indices.front() ];
  std::vector< double > xs, prev_xs;
  int count = -1;

  for( auto f : face_indices )
  {
    auto& triangle = triangles[ f ];
    if( triangle.width <= 0.0 || triangle.height <= 0.0 ) { continue; }

    if( use_compact )
    {
      ++count;

      bool const rot_tri = ( count % 2 != 0 );
      if( rot_tri ) { rotate_triangle( triangle ); }

      if( count > 0 && current_u > margin )
      {
        xs = { triangle.a( 0 ), triangle.b( 0 ), triangle.c( 0 ) };
        std::sort( xs.begin(), xs.end() );
        prev_xs = { prev_tri.a( 0 ), prev_tri.b( 0 ), prev_tri.c( 0 ) };
        std::sort( prev_xs.begin(), prev_xs.end() );

        double s = 0.0;
        if( rot_tri )
        {
          double const
          d1 = xs[ 1 ] - xs[ 0 ],
            d2 = ( prev_xs[ 2 ] - prev_xs[ 1 ] ) * triangle.height /
                 prev_tri.height;
          s = std::min( d1, d2 );
        }
        else
        {
          double const
          d1 = prev_xs[ 2 ] - prev_xs[ 1 ],
            x  = prev_xs[ 1 ] + triangle.height * d1 / prev_tri.height,
            d2 = prev_xs[ 2 ] + xs[ 1 ] - x;
          s = std::min( d1, d2 );
        }
        current_u -= s - padding;
      }
    }

    if( current_u + triangle.width + margin > max_width )
    {
      current_u = margin;
      current_v = next_v + margin;
    }

    shift( 0 ) = current_u;
    shift( 1 ) = current_v;
    triangle.a += shift;
    triangle.b += shift;
    triangle.c += shift;

    if( current_u + triangle.width > max_u )
    {
      max_u = current_u + triangle.width;
    }
    if( current_v + triangle.height > max_v )
    {
      max_v = current_v + triangle.height;
    }

    next_v    = std::max( next_v, current_v + triangle.height );
    current_u += triangle.width + margin;

    prev_tri = triangle;
  }

  // Normalize texture coordinates.
  double const width     = max_u + margin;
  double const height    = max_v + margin;
  double const normalize = 1.0 / std::max( width, height );

  for( unsigned int i = 0; i < triangles.size(); ++i )
  {
    tcoords[ i * 3 + 0 ] = triangles[ i ].a * normalize;
    tcoords[ i * 3 + 1 ] = triangles[ i ].b * normalize;
    tcoords[ i * 3 + 2 ] = triangles[ i ].c * normalize;
  }
  mesh_container->set_tex_coords( tcoords );
}

} // namespace core

} // namespace arrows

} // namespace kwiver
