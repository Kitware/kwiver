// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of mesh texturing algorithm

#include "texture_mesh.h"

#include <arrows/core/render_mesh_depth_map.h>
#include <vital/exceptions.h>
#include <vital/util/transform_image.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
namespace {

kv::vector_2d
perspective_divide( const kv::vector_4d& p )
{
  double z = p[ 2 ];
  if( z == 0 )
  {
    return { -1, -1 };
  }
  return { p[ 0 ] / z, p[ 1 ] / z };
}

kv::vector_4d
homogenize( kv::vector_2d const& p )
{
  return { p[ 0 ], p[ 1 ], 1, 1 };
}

kv::vector_3d
homogenize3( kv::vector_2d const& p )
{
  return { p[ 0 ], p[ 1 ], 1 };
}

} // end anonymous namespace

// ----------------------------------------------------------------------------
void
texture_mesh
::initialize()
{
  attach_logger( "arrows.core.texture_mesh" );
}

// ----------------------------------------------------------------------------
texture_mesh::
~texture_mesh() {}

// ----------------------------------------------------------------------------
void
texture_mesh
::prepare(
  kv::mesh_container_sptr mesh_container,
  kv::image_container_sptr output_image )
{
  out_scale_ = std::min( output_image->width(), output_image->height() );
  texture_coords_ = mesh_container->tex_coords();

  mesh_ = std::make_shared< kv::mesh >( mesh_container->mesh() );
  if( !mesh_->faces().has_normals() )
  {
    mesh_->compute_face_normals();
  }

  if( texture_coords_.empty() )
  {
    VITAL_THROW( kv::invalid_value, "No texture coordinates found in mesh." );
  }

  if( mesh_->faces().regularity() != 3 )
  {
    VITAL_THROW( kv::invalid_value, "Input mesh must be triangular." );
  }

  if( mesh_container->has_tex_coords() == kv::mesh::TEX_COORD_NONE )
  {
    VITAL_THROW( kv::invalid_value, "Mesh must have texture coordinates." );
  }

  if( output_image->depth() != 4 )
  {
    VITAL_THROW( kv::invalid_value, "Output image must be RGBA." );
  }

  if( output_image->width() != output_image->height() )
  {
    LOG_WARN(
      logger(), "Output texture map is not square (" <<
        output_image->width() << " x " <<
        output_image->height() << "). Will proceed with " <<
        out_scale_ << " x " << out_scale_ << "." );
  }

  generate_triangles();
}

// ----------------------------------------------------------------------------
void
texture_mesh
::texture(
  kv::mesh_container_sptr mesh_container,
  kv::image_container_sptr output_image,
  kv::image_container_sptr frame,
  kv::camera_perspective_sptr camera )
{
  if( !mesh_container )
  {
    VITAL_THROW( kv::invalid_value, "mesh_container is NULL" );
  }
  if( !output_image )
  {
    VITAL_THROW( kv::invalid_value, "output_image is NULL" );
  }
  if( !frame )
  {
    VITAL_THROW( kv::invalid_value, "frame is NULL" );
  }
  if( !camera )
  {
    VITAL_THROW( kv::invalid_value, "camera is NULL" );
  }

  prepare( mesh_container, output_image );
  texture_frame( frame, camera, output_image );
}

// ----------------------------------------------------------------------------
void
texture_mesh
::texture_xyz(
  kv::mesh_container_sptr mesh_container,
  kv::image_container_sptr output_image )
{
  if( !mesh_container )
  {
    VITAL_THROW( kv::invalid_value, "mesh_container is NULL" );
  }
  if( !output_image )
  {
    VITAL_THROW( kv::invalid_value, "output_image is NULL" );
  }

  prepare( mesh_container, output_image );
  texture_xyz_impl( output_image );
}

// ----------------------------------------------------------------------------
void
texture_mesh
::texture_list(
  kv::mesh_container_sptr mesh_container,
  kv::image_container_sptr_list& output_images,
  kv::image_container_sptr_list const& frames,
  kv::camera_sptr_list const& cameras,
  std::string const& mode )
{
  if( !mesh_container )
  {
    VITAL_THROW( kv::invalid_value, "mesh_container is NULL." );
  }
  if( output_images.empty() )
  {
    VITAL_THROW( kv::invalid_value, "No output images provided." );
  }
  for( auto const& img : output_images )
  {
    if( !img )
    {
      VITAL_THROW( kv::invalid_value, "output_image is NULL." );
    }
  }
  for( auto const& frame : frames )
  {
    if( !frame )
    {
      VITAL_THROW( kv::invalid_value, "frame is NULL." );
    }
  }
  for( auto const& camera : cameras )
  {
    if( !camera )
    {
      VITAL_THROW( kv::invalid_value, "camera is NULL." );
    }
  }

  prepare( mesh_container, output_images[ 0 ] );

  if( mode == "all" )
  {
    texture_list_all( output_images, frames, cameras );
    return;
  }
  else if( mode != "mean" && mode != "median" )
  {
    VITAL_THROW(
      kv::invalid_value, "Invalid mode specified. Valid modes are "
                         "\"all\", \"mean\", and \"median\"." );
  }

  kv::image_container_sptr_list outputs( frames.size() );
  for( size_t i = 0; i < frames.size(); ++i )
  {
    kv::image_of< uint8_t > img( out_scale_, out_scale_, 4 );
    kv::transform_image( img, []( uint8_t ){ return 0; } );
    outputs[ i ] = std::make_shared< kv::simple_image_container >( img );
  }

  texture_list_all( outputs, frames, cameras );

  if( mode == "mean" )
  {
    output_images[ 0 ] = aggregate_mean( outputs );
  }
  else if( mode == "median" )
  {
    output_images[ 0 ] = aggregate_median( outputs );
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::texture_list_all(
  kv::image_container_sptr_list const& output_images,
  kv::image_container_sptr_list const& frames,
  kv::camera_sptr_list const& cameras )
{
  auto const& ref = output_images[ 0 ];
  for( size_t i = 0; i < output_images.size(); ++i )
  {
    if( output_images[ i ]->width() != ref->width() ||
        output_images[ i ]->height() != ref->height() ||
        output_images[ i ]->depth() != ref->depth() )
    {
      VITAL_THROW( kv::invalid_value, "Output images must be the same size." );
    }

    if( i >= frames.size() || i >= cameras.size() )
    {
      LOG_WARN( logger(), "Not enough input frames/cameras provided." );
      break;
    }

    auto camera =
      std::dynamic_pointer_cast< kv::camera_perspective >( cameras[ i ] );

    if( !camera )
    {
      VITAL_THROW( kv::invalid_value, "Camera must be a perspective camera." );
    }

    texture_frame( frames[ i ], camera, output_images[ i ] );
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::texture_frame(
  kv::image_container_sptr frame,
  kv::camera_perspective_sptr camera,
  kv::image_container_sptr output_image )
{
  z_buffer_ = core::render_mesh_depth_map( mesh_, camera )->get_image();

  kv::matrix_4x4d cam_rot = kv::matrix_4x4d::Identity();
  cam_rot.block( 0, 0, 3, 3 ) << camera->rotation().matrix();

  kv::matrix_4x4d cam_trans = kv::matrix_4x4d::Identity();
  cam_trans.col( 3 ).head( 3 ) << -camera->center();

  kv::matrix_4x4d camera_center = cam_rot * cam_trans;

  kv::matrix_4x4d camera_proj = kv::matrix_4x4d::Identity();
  camera_proj.block( 0, 0, 3, 3 ) << camera->intrinsics()->as_matrix();

  bool distortion = camera->intrinsics()->dist_coeffs().size() > 0;
  auto& faces = mesh_->faces();
  auto& verts = mesh_->vertices();

#pragma omp parallel for
  for( size_t f = 0; f < mesh_->num_faces(); ++f )
  {
    std::vector< kv::vector_3d > mesh_triangle;
    for( int i = 0; i < 3; ++i )
    {
      auto fi = faces( f, i );
      mesh_triangle.push_back(
        { verts( fi, 0 ),
          verts( fi, 1 ),
          verts( fi, 2 ) } );
    }

    kv::vector_3d normal = faces.normal( f );
    kv::vector_3d position = ( mesh_triangle[ 0 ] +
                               mesh_triangle[ 1 ] +
                               mesh_triangle[ 2 ] ) / 3.0;
    kv::vector_3d cameraPointVec = position - camera->center();
    if( cameraPointVec.dot( normal ) > 0.0 )
    {
      continue;
    }

    bool out_of_bounds = false;
    for( kv::vector_3d p : mesh_triangle )
    {
      if( camera->depth( p ) <= 0 )
      {
        out_of_bounds = true;
        break;
      }
    }
    if( out_of_bounds )
    {
      continue;
    }

    const auto f3 = 3 * f;
    std::vector< kv::vector_2d > uv_points = {
      out_scale_* texture_coords_[ f3 ],
      out_scale_* texture_coords_[ f3 + 1 ],
      out_scale_* texture_coords_[ f3 + 2 ] };

    kv::matrix_3x3d basis_uv = kv::matrix_3x3d::Identity();
    basis_uv.col( 0 ).head( 2 ) << uv_points[ 0 ] - uv_points[ 2 ];
    basis_uv.col( 1 ).head( 2 ) << uv_points[ 1 ] - uv_points[ 2 ];

    if( !basis_uv.determinant() )
    {
      continue;
    }

    kv::matrix_3x3d basis_3d;
    basis_3d << mesh_triangle[ 0 ] - mesh_triangle[ 2 ],
      mesh_triangle[ 1 ] - mesh_triangle[ 2 ],
      mesh_triangle[ 2 ];

    kv::matrix_3x3d uv_translation = kv::matrix_3x3d::Identity();
    uv_translation.col( 2 ).head( 2 ) << -uv_points[ 2 ];

    kv::matrix_4x4d uv_to_mesh = kv::matrix_4x4d::Identity();
    uv_to_mesh.block( 0, 0, 3, 3 ) << basis_3d *
      basis_uv.inverse() *
      uv_translation;

    kv::matrix_4x4d uv_to_camera = camera_center * uv_to_mesh;

    copy_triangle(
      uv_to_camera, camera_proj, camera, frame, distortion, f,
      output_image );
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::texture_xyz_impl( kv::image_container_sptr output_image )
{
  auto& faces = mesh_->faces();
  auto& verts = mesh_->vertices();
  auto const face_count = mesh_->num_faces();
#pragma omp parallel for
  for( size_t f = 0; f < face_count; ++f )
  {
    std::vector< kv::vector_3d > mesh_triangle;
    for( int i = 0; i < 3; ++i )
    {
      auto fi = faces( f, i );
      mesh_triangle.push_back(
        { verts( fi, 0 ),
          verts( fi, 1 ),
          verts( fi, 2 ) } );
    }

    auto const f3 = 3 * f;
    std::vector< kv::vector_2d > uv_points = {
      out_scale_* texture_coords_[ f3 ],
      out_scale_* texture_coords_[ f3 + 1 ],
      out_scale_* texture_coords_[ f3 + 2 ] };

    kv::matrix_3x3d Q = kv::matrix_3x3d::Ones();
    Q.block( 0, 0, 2, 3 ) << uv_points[ 0 ], uv_points[ 1 ], uv_points[ 2 ];
    if( !Q.determinant() )
    {
      LOG_DEBUG( logger(), "skip degenerate triangle " << f );
      continue;
    }

    kv::matrix_3x3d P;
    P << mesh_triangle[ 0 ], mesh_triangle[ 1 ], mesh_triangle[ 2 ];

    kv::matrix_3x3d const M = P * Q.inverse();
    fill_triangle_xyz( M, f, output_image );
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::generate_triangles()
{
  auto const FaceCnt = mesh_->num_faces();
#pragma omp parallel for
  for( size_t f = 0; f < FaceCnt; ++f )
  {
    auto const f3 = 3 * f;
    std::vector< kv::vector_2d > uv_verts = {
      out_scale_* texture_coords_[ f3 ],
      out_scale_* texture_coords_[ f3 + 1 ],
      out_scale_* texture_coords_[ f3 + 2 ] };
    std::sort(
      uv_verts.begin(), uv_verts.end(),
      []( kv::vector_2d const& a, kv::vector_2d const& b )
      { return a.y() < b.y(); } );

    double const
    a_x = uv_verts[ 0 ].x(), a_y = uv_verts[ 0 ].y(),
      b_x = uv_verts[ 1 ].x(), b_y = uv_verts[ 1 ].y(),
      c_x = uv_verts[ 2 ].x(), c_y = uv_verts[ 2 ].y();

    double const
    step_bc = c_y == b_y ? 0 : ( c_x - b_x ) / ( c_y - b_y ),
      step_ab = b_y == a_y ? 0 : ( b_x - a_x ) / ( b_y - a_y ),
      step_ac = c_y == a_y ? 0 : ( c_x - a_x ) / ( c_y - a_y );

    double const
    d_x = a_x + step_ac * ( b_y - a_y ), d_y = b_y,
      step_dc = c_y == d_y ? 0 : ( c_x - d_x ) / ( c_y - d_y );

    double
      x_step[ 2 ][ 2 ]  = { { step_ac, step_dc }, { step_ab, step_bc } },
      x_bounds[ 2 ][ 2 ] = { { a_x, d_x }, { a_x, b_x } },
      y_bounds[ 2 ][ 2 ] = { { a_y, b_y }, { b_y, c_y } };

    if( b_x < d_x )
    {
      std::swap( x_step[ 0 ],  x_step[ 1 ] );
      std::swap( x_bounds[ 0 ], x_bounds[ 1 ] );
    }

    std::vector< kv::vector_2d > triangle_points;

    for( size_t section = 0; section < 2; ++section )
    {
      auto const
      y_offset = y_bounds[ 0 ][ section ],
        y_max = y_bounds[ 1 ][ section ];
      for( double y = 0; y + y_offset <= y_max; ++y )
      {
        double const
        x_left  = x_bounds[ 0 ][ section ] + y * x_step[ 0 ][ section ] - 1,
          x_right = x_bounds[ 1 ][ section ] + y * x_step[ 1 ][ section ] + 1;
        auto const yoff = y + y_offset;
        for( double x = x_left; x <= x_right; ++x )
        {
          triangle_points.push_back( { x, yoff } );
        }
      }
    }
#pragma omp critical
    {
      frame_data_map_[ f ] = triangle_points;
    }
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::set_pixel_xyz(
  kv::vector_3d const& mesh_point,
  kv::vector_2d const& output_position,
  kv::image_container_sptr output_image )
{
  auto const
  out_x = output_position.x(),
    out_y = output_position.y();
  if( out_x < 0.0 || out_y < 0.0 ||
      out_x >= output_image->width() ||
      out_y >= output_image->height() )
  {
    LOG_DEBUG(
      logger(), "skip output position out of bounds: " <<
        output_position.transpose() );
    return;
  }

  kv::image_of< float > output( output_image->get_image() );
#pragma omp critical
  {
    for( size_t i = 0; i < 3; ++i )
    {
      output( out_x, out_y, i ) = mesh_point[ i ];
    }
    output( out_x, out_y, 3 ) = 1;
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::sample_pixel(
  kv::image_container_sptr frame_image,
  const kv::vector_2d& frame_position,
  const kv::vector_2d& output_position,
  kv::image_container_sptr output_image )
{
  double out_x = output_position.x(), out_y = output_position.y();
  double frame_x = frame_position.x(), frame_y = frame_position.y();

  if( out_x < 0.0 ||
      out_y < 0.0 ||
      out_x >= output_image->width() ||
      out_y >= output_image->height() ||
      frame_x < 0.0 ||
      frame_y < 0.0 ||
      frame_x >= frame_image->width() ||
      frame_y >= frame_image->height() )
  {
    return;
  }

  kv::image_of< uint8_t > output( output_image->get_image() );
  kv::image_of< uint8_t > frame( frame_image->get_image() );
  auto color = frame.at( frame_x, frame_y );

#pragma omp critical
  {
    output( out_x, out_y, 0 ) = color.r;
    output( out_x, out_y, 1 ) = color.g;
    output( out_x, out_y, 2 ) = color.b;
    output( out_x, out_y, 3 ) = 255;
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::fill_triangle_xyz(
  kv::matrix_3x3d const& uv_to_mesh, int face_id,
  kv::image_container_sptr output_image )
{
  auto frame_data_itr = frame_data_map_.find( face_id );
  if( frame_data_itr == frame_data_map_.end() )
  {
    LOG_DEBUG( logger(), "no triangle for face_id=" <<  face_id );
    return;
  }

  auto const& face_points = frame_data_itr->second;
  auto const n = face_points.size();
  if( !n )
  {
    LOG_WARN( logger(), "point count=0 for face_id=" <<  face_id );
  }
  for( size_t i = 0; i < n; ++i )
  {
    kv::vector_2d const
    & point = face_points[ i ],
    & output_position = { point[ 0 ], out_scale_ - point[ 1 ] };
    kv::vector_3d const mesh_point = uv_to_mesh * homogenize3( point );
    set_pixel_xyz( mesh_point, output_position, output_image );
  }
}

// ----------------------------------------------------------------------------
void
texture_mesh
::copy_triangle(
  const kv::matrix_4x4d& uv_to_camera,
  const kv::matrix_4x4d& camera_proj,
  kv::camera_perspective_sptr camera,
  kv::image_container_sptr frame,
  bool distortion,
  int face_id,
  kv::image_container_sptr output_image )
{
  auto frame_data_itr = frame_data_map_.find( face_id );
  if( frame_data_itr == frame_data_map_.end() )
  {
    return;
  }

  const auto& face_points = frame_data_itr->second;

  kv::matrix_4x4d combined_transform = camera_proj * uv_to_camera;

  for( size_t i = 0; i < face_points.size(); ++i )
  {
    const kv::vector_2d& point = face_points[ i ];
    kv::vector_2d frame_position;

    if( distortion )
    {
      kv::vector_2d point_dist = camera->intrinsics()->distort(
        perspective_divide( uv_to_camera * homogenize( point ) ) );

      frame_position =
        ( camera_proj * homogenize( point_dist ) ).head( 2 );
    }
    else
    {
      frame_position =
        perspective_divide( combined_transform * homogenize( point ) );
    }

    int x = frame_position.x();
    int y = frame_position.y();

    if( x < 0 || x >= ( int ) frame->width() ||
        y < 0 || y >= ( int ) frame->height() )
    {
      continue;
    }

    double depth = ( uv_to_camera * homogenize( point ) ).z();
    if( depth - c_z_threshold > z_buffer_.at< double >( x, y ) )
    {
      continue;
    }

    sample_pixel(
      frame, frame_position, { point[ 0 ], out_scale_ - point[ 1 ] },
      output_image );
  }
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
texture_mesh
::aggregate_mean( const kv::image_container_sptr_list& textures )
{
  kv::image_of< uint8_t > img( out_scale_, out_scale_, 4 );
  kv::transform_image( img, []( uint8_t ){ return 0; } );

  size_t n = textures.size();
  if( n == 0 )
  {
    return std::make_shared< kv::simple_image_container >( img );
  }

  for( size_t w = 0; w < out_scale_; ++w )
  {
    for( size_t h = 0; h < out_scale_; ++h )
    {
      size_t transparent_count = 0;

      for( size_t d = 0; d < 3; ++d )
      {
        transparent_count = 0;

        uint64_t sum = 0;

        for( const auto& texture : textures )
        {
          if( texture->get_image().at< uint8_t >( w, h, 3 ) == 0 )
          {
            ++transparent_count;
            continue;
          }
          sum += texture->get_image().at< uint8_t >( w, h, d );
        }

        if( transparent_count == n )
        {
          img( w, h, d ) = 0;
        }
        else
        {
          img( w, h, d ) = sum / ( n - transparent_count );
        }
      }

      img( w, h, 3 ) = transparent_count == n ? 0 : 255;
    }
  }

  return std::make_shared< kv::simple_image_container >( img );
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
texture_mesh
::aggregate_median( const kv::image_container_sptr_list& textures )
{
  kv::image_of< uint8_t > img( out_scale_, out_scale_, 4 );
  kv::transform_image( img, []( uint8_t ){ return 0; } );

  size_t n = textures.size();
  if( n == 0 )
  {
    return std::make_shared< kv::simple_image_container >( img );
  }

  for( size_t w = 0; w < out_scale_; ++w )
  {
    for( size_t h = 0; h < out_scale_; ++h )
    {
      size_t transparent_count = 0;

      for( size_t d = 0; d < 3; ++d )
      {
        transparent_count = 0;

        std::vector< uint8_t > values;
        for( size_t i = 0; i < n; ++i )
        {
          if( textures[ i ]->get_image().at< uint8_t >( w, h, 3 ) == 0 )
          {
            ++transparent_count;
            continue;
          }
          values.push_back(
            textures[ i ]->get_image().at< uint8_t >(
              w, h,
              d ) );
        }

        if( transparent_count == n )
        {
          img( w, h, d ) = 0;
          continue;
        }

        std::sort( values.begin(), values.end() );
        img( w, h, d ) = values[ values.size() / 2 ];
      }

      img( w, h, 3 ) = transparent_count == n ? 0 : 255;
    }
  }

  return std::make_shared< kv::simple_image_container >( img );
}

// ----------------------------------------------------------------------------

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
