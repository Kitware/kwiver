// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL essential matrix estimation algorithm (5 point alg)

#ifndef KWIVER_ARROWS_VXL_ESTIMATE_ESSENTIAL_MATRIX_H_
#define KWIVER_ARROWS_VXL_ESTIMATE_ESSENTIAL_MATRIX_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/types/camera_intrinsics.h>

#include <vital/algo/estimate_essential_matrix.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// A class that uses 5 pt algorithm to estimate an initial xform between 2 pt
/// sets
class KWIVER_ALGO_VXL_EXPORT estimate_essential_matrix
  : public vital::algo::estimate_essential_matrix
{
public:
  PLUGGABLE_IMPL(
    estimate_essential_matrix,
    "Use VXL (vpgl) to estimate an essential matrix.",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "debugging information",
      false ),
    PARAM_DEFAULT(
      num_ransac_samples, unsigned,
      "The number of samples to use in RANSAC",
      512 )
  )

  /// Destructor
  virtual ~estimate_essential_matrix() = default;

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Estimate an essential matrix from corresponding points
  ///
  /// \param [in]  pts1 the vector or corresponding points from the first image
  /// \param [in]  pts2 the vector of corresponding points from the second image
  /// \param [in]  cal1 the intrinsic parameters of the first camera
  /// \param [in]  cal2 the intrinsic parameters of the second camera
  /// \param [out] inliers for each point pa:wir, the value is true if
  ///                      this pair is an inlier to the estimate
  /// \param [in]  inlier_scale error distance tolerated for matches to be
  /// inliers
  virtual
  vital::essential_matrix_sptr
  estimate(
    const std::vector< vital::vector_2d >& pts1,
    const std::vector< vital::vector_2d >& pts2,
    const vital::camera_intrinsics_sptr cal1,
    const vital::camera_intrinsics_sptr cal2,
    std::vector< bool >& inliers,
    double inlier_scale = 1.0 ) const;
  using vital::algo::estimate_essential_matrix::estimate;

private:
  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif
