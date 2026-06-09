// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Header for mesh texturing algorithm

#ifndef KWIVER_ARROWS_CORE_TEXTURE_MESH_H
#define KWIVER_ARROWS_CORE_TEXTURE_MESH_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/texture_mesh.h>
#include <vital/types/camera_perspective.h>
#include <vital/util/enum_converter.h>

#include "vital/plugin_management/pluggable_macro_magic.h"

#include <unordered_map>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

enum texture_list_mode { MODE_all, MODE_mean, MODE_median, };

ENUM_CONVERTER(
  texture_list_mode_converter, texture_list_mode,
  { "all",    MODE_all    },
  { "mean",   MODE_mean   },
  { "median", MODE_median } );

/// Create a UV texture map for a mesh given posed images
class KWIVER_ALGO_CORE_EXPORT texture_mesh
  : public vital::algo::texture_mesh
{
public:
  PLUGGABLE_IMPL(
    texture_mesh,
    "Texture a mesh with UV coordinates with posed images.",
    PARAM_DEFAULT(
      z_threshold, float,
      "The difference in depth between Z Buffer values and mesh points. "
      "If (depth - z_threshold) > z, the given pixel will be untextured. "
      "Values should be small.",
      0.05f ),
    PARAM_DEFAULT(
      mode, std::string,
      "Aggregation mode used by texture_list. "
      "\"all\": write one texture atlas per frame into output_images. "
      "\"mean\": project all frames and write their per-pixel mean into "
      "output_images[0]. "
      "\"median\": project all frames and write their per-pixel median into "
      "output_images[0].",
      texture_list_mode_converter().to_string( MODE_all ) )
  )

  virtual ~texture_mesh();

  bool
  check_configuration( kv::config_block_sptr ) const override
  {
    return true;
  }

  /// \throws invalid_value if \p camera is not a camera_perspective.
  void texture(
    kv::mesh_container_sptr mesh_container,
    kv::image_container_sptr output_image,
    kv::image_container_sptr frame,
    kv::camera_sptr camera ) override;

  /// Behavior is controlled by the \c mode configuration parameter.
  /// In \c "all" mode, one pre-allocated RGBA image per frame is required in
  /// \p output_images. In \c "mean" or \c "median" mode only
  /// \p output_images[0] is written.
  /// \throws invalid_value if any camera is not a camera_perspective.
  void texture_list(
    kv::mesh_container_sptr mesh_container,
    kv::image_container_sptr_list& output_images,
    kv::image_container_sptr_list const& frames,
    kv::camera_sptr_list const& cameras ) override;

  void texture_xyz(
    kv::mesh_container_sptr mesh_container,
    kv::image_container_sptr output_image ) override;

private:
  void initialize() override;

  /// Texture all output images with corresponding frames and perspective
  /// cameras
  void
  texture_list_all(
    kv::image_container_sptr_list const& output_images,
    kv::image_container_sptr_list const& frames,
    kv::camera_sptr_list const& cameras );

  /// Validate inputs, populate mesh_, texture_coords_, and out_scale_, and
  /// rasterize UV triangles into frame_data_map_.
  void prepare(
    kv::mesh_container_sptr mesh_container,
    kv::image_container_sptr output_image );

  /// Rasterize each face's UV triangle into frame_data_map_.
  void generate_triangles();

  /// Fill output_image texture map for a mesh given frame and camera.
  void
  texture_frame(
    kv::image_container_sptr frame,
    kv::camera_perspective_sptr camera,
    kv::image_container_sptr output_image );

  /// Fill output_image texture map with XYZ values using alpha for validity.
  void texture_xyz_impl( kv::image_container_sptr output_image );

  /// Copy a pixel from the source frame to the output texture map.
  void
  sample_pixel(
    kv::image_container_sptr frame_image,
    kv::vector_2d const& frame_position,
    kv::vector_2d const& output_position,
    kv::image_container_sptr output_image );

  /// Set the pixel to XYZA using alpha as validity.
  void
  set_pixel_xyz(
    kv::vector_3d const& mesh_point,
    kv::vector_2d const& output_position,
    kv::image_container_sptr output_image );

  /// Fill all pixels in triangle with XYZA values using alpha as validity.
  void
  fill_triangle_xyz(
    kv::matrix_3x3d const& world_from_uv, int face_id,
    kv::image_container_sptr output_image );

  /// Copy all pixels in camera's view of the face triangle to output_image.
  void
  copy_triangle(
    kv::matrix_3x3d const& camera_from_uv,
    kv::matrix_3x3d const& camera_K,
    kv::camera_intrinsics_sptr intrinsics,
    kv::image_container_sptr frame,
    bool distortion,
    int face_id,
    kv::image_container_sptr output_image );

  /// Aggregate mean image from a list of output textures.
  kv::image_container_sptr
  aggregate_mean( kv::image_container_sptr_list const& textures );

  /// Aggregate median image from a list of output textures.
  kv::image_container_sptr
  aggregate_median( kv::image_container_sptr_list const& textures );

  std::unordered_map< int, std::vector< kv::vector_2d > > frame_data_map_;
  kv::mesh_sptr mesh_;
  std::vector< kv::vector_2d > texture_coords_;
  double out_scale_ = 0.0;
  kv::image z_buffer_;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
