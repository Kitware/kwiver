// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header file for compute depth

#ifndef KWIVER_ARROWS_CUDA_INTEGRATE_DEPTH_MAPS_H_
#define KWIVER_ARROWS_CUDA_INTEGRATE_DEPTH_MAPS_H_

#include <arrows/cuda/kwiver_algo_cuda_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/integrate_depth_maps.h>
#include <vital/types/vector.h>
#include <vital/vital_config.h>

namespace kwiver {

namespace arrows {

namespace cuda {

class KWIVER_ALGO_CUDA_EXPORT integrate_depth_maps
  : public vital::algo::integrate_depth_maps
{
public:
  using array3 = std::array< double, 3 >;
  PLUGGABLE_IMPL(
    integrate_depth_maps,
    "depth map fusion",
    PARAM_DEFAULT(
      ray_potential_thickness, double,
      "Distance that the TSDF covers sloping from Rho to zero. "
      "Units are in voxels.", 20.0 ),
    PARAM_DEFAULT(
      ray_potential_rho, double,
      "Maximum magnitude of the TDSF", 1.0 ),
    PARAM_DEFAULT(
      ray_potential_eta, double,
      "Fraction of rho to use for free space constraint. "
      "Requires 0 <= Eta <= 1.", 1.0 ),
    PARAM_DEFAULT(
      ray_potential_epsilon, double,
      "Fraction of rho to use in occluded space. "
      "Requires 0 <= Epsilon <= 1.", 0.01 ),
    PARAM_DEFAULT(
      ray_potential_delta, double,
      "Distance from the surface before the TSDF is truncate. "
      "Units are in voxels", 200.0 ),
    PARAM_DEFAULT(
      voxel_spacing_factor, double,
      "Multiplier on voxel spacing.  Set to 1.0 for voxel "
      "sizes that project to 1 pixel on average.", 1.0 ),
    PARAM_DEFAULT(
      max_voxels_per_launch, unsigned,
      "Maximum number of voxels to process in a single kernel "
      "launch.  Processing too much data at once on the GPU "
      "can cause the GPU to time out.  Set to zero for "
      "unlimited.", 20000000 ),
    PARAM_DEFAULT(
      grid_spacing, array3,
      "Relative spacing for each dimension of the grid",
      array3( { 1., 1., 1. } ) )
  )

  /// Destructor
  virtual ~integrate_depth_maps() = default;

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Integrate multiple depth maps with per-pixel weights into a common volume
  ///
  /// The weight maps in this variant encode how much weight to give each depth
  /// pixel in the integration sum.  If the vector of weight_maps is empty then
  /// all depths are given full weight.
  ///
  /// \param [in]     minpt_bound the min point of the bounding region
  /// \param [in]     maxpt_bound the max point of the bounding region
  /// \param [in]     depth_maps  the set of floating point depth map images
  /// \param [in]     weight_maps the set of floating point [0,1] weight maps
  /// \param [in]     cameras     the set of cameras, one for each depth map
  /// \param [in,out] volume      the fused volumetric data
  /// \param [out]    spacing     the spacing between voxels in each dimension
  ///
  /// \note the volume data is stored as a 3D image.  Metadata fields on the
  /// image specify the origin and scale of the volume in world coordinates.
  virtual void
  integrate(
    kwiver::vital::vector_3d const& minpt_bound,
    kwiver::vital::vector_3d const& maxpt_bound,
    std::vector< kwiver::vital::image_container_sptr > const& depth_maps,
    std::vector< kwiver::vital::image_container_sptr > const& weight_maps,
    std::vector< kwiver::vital::camera_perspective_sptr > const& cameras,
    kwiver::vital::image_container_sptr& volume,
    kwiver::vital::vector_3d& spacing ) const;

protected:
  void initialize() override;
};

}  // end namespace cuda

}  // end namespace arrows

}  // end namespace kwiver

#endif
