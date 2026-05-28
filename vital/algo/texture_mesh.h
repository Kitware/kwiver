// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to abstract texture mesh algorithm

#ifndef VITAL_ALGO_TEXTURE_MESH_H
#define VITAL_ALGO_TEXTURE_MESH_H

#include <vital/algo/algorithm.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/image_container.h>
#include <vital/types/mesh_container.h>
#include <vital/vital_config.h>

namespace kwiver {

namespace vital {

namespace algo {

/// \brief Project camera frames onto a UV-unwrapped mesh to produce texture
///        atlas images.
class VITAL_ALGO_EXPORT texture_mesh
  : public kwiver::vital::algorithm
{
public:
  texture_mesh();
  PLUGGABLE_INTERFACE( texture_mesh );

  /// Project a single camera frame onto the mesh UV space, writing colors into
  /// \p output_image.
  ///
  /// \param mesh_container Mesh container with UV coordinates.
  /// \param output_image Pre-allocated RGBA output texture atlas (preferably
  ///        square).
  /// \param frame Video frame corresponding to \p camera.
  /// \param camera Perspective camera corresponding to \p frame.
  virtual void
  texture(
    kwiver::vital::mesh_container_sptr mesh_container,
    kwiver::vital::image_container_sptr output_image,
    kwiver::vital::image_container_sptr frame,
    kwiver::vital::camera_perspective_sptr camera ) = 0;

  /// Project multiple frames onto the mesh and aggregate into one or more
  /// texture atlases using the specified mode.
  ///
  /// \param mesh_container Mesh container with UV coordinates.
  /// \param [in,out] output_images Output texture atlas list. In \c "all" mode
  ///   one pre-allocated RGBA image per frame is required. In \c "mean" or
  ///   \c "median" mode only the first element is used and modified.
  /// \param frames Video frames corresponding to \p cameras.
  /// \param cameras Perspective cameras corresponding to \p frames.
  /// \param mode Aggregation mode: \c "all" writes one atlas per frame;
  ///   \c "mean" and \c "median" collapse all frames into a single atlas.
  virtual void
  texture_list(
    kwiver::vital::mesh_container_sptr mesh_container,
    kwiver::vital::image_container_sptr_list& output_images,
    kwiver::vital::image_container_sptr_list const& frames,
    kwiver::vital::camera_sptr_list const& cameras,
    std::string const& mode = "all" ) = 0;

  /// Generate a float texture map encoding the 3D world-space surface position
  /// (XYZ + validity alpha) at each UV texel.
  ///
  /// \param mesh_container Mesh container with UV coordinates.
  /// \param output_image Pre-allocated zero-initialized 4-channel float image;
  ///   RGB channels receive world XYZ, alpha is 1 where mesh surface is
  ///   present.
  virtual void
  texture_xyz(
    kwiver::vital::mesh_container_sptr mesh_container,
    kwiver::vital::image_container_sptr output_image ) = 0;
};

typedef std::shared_ptr< texture_mesh > texture_mesh_sptr;

} // namespace algo

} // namespace vital

} // namespace kwiver

#endif // VITAL_ALGO_TEXTURE_MESH_H
