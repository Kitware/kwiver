// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL fundamental matrix estimation algorithm (5 point alg)

#ifndef KWIVER_ARROWS_VXL_ESTIMATE_FUNDAMENTAL_MATRIX_H_
#define KWIVER_ARROWS_VXL_ESTIMATE_FUNDAMENTAL_MATRIX_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/types/camera_intrinsics.h>

#include <vital/algo/estimate_fundamental_matrix.h>

#include <vital/util/enum_converter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

enum method_t { EST_7_POINT, EST_8_POINT, };

/// A class that uses 5 pt algorithm to estimate an initial xform between 2 pt
/// sets
class KWIVER_ALGO_VXL_EXPORT estimate_fundamental_matrix
  : public vital::algo::estimate_fundamental_matrix
{
public:
  PLUGGABLE_IMPL(
    estimate_fundamental_matrix,
    "Use VXL (vpgl) to estimate a fundamental matrix.",
    PARAM_DEFAULT(
      precondition, bool,
      "If true, precondition the data before estimating the "
      "fundamental matrix",
      true ),
    PARAM_DEFAULT(
      method, std::string,
      "Fundamental matrix estimation method to use. "
      "(Note: does not include RANSAC).  Choices are: " +
      method_converter().element_name_string(),
      method_converter().to_string( EST_8_POINT ) )
  )

  /// Destructor
  virtual ~estimate_fundamental_matrix() = default;

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Estimate an fundamental matrix from corresponding points
  ///
  /// \param [in]  pts1 the vector or corresponding points from the first image
  /// \param [in]  pts2 the vector of corresponding points from the second image
  /// \param [out] inliers for each point pair, the value is true if
  ///                      this pair is an inlier to the estimate
  /// \param [in]  inlier_scale error distance tolerated for matches to be
  /// inliers
  virtual
  vital::fundamental_matrix_sptr
  estimate(
    const std::vector< vital::vector_2d >& pts1,
    const std::vector< vital::vector_2d >& pts2,
    std::vector< bool >& inliers,
    double inlier_scale = 1.0 ) const;
  using vital::algo::estimate_fundamental_matrix::estimate;

  /// Test corresponding points against a fundamental matrix and mark inliers
  ///
  /// \param [in]  fm   the fundamental matrix
  /// \param [in]  pts1 the vector or corresponding points from the first image
  /// \param [in]  pts2 the vector of corresponding points from the second image
  /// \param [out] inliers for each point pair, the value is true if
  ///                      this pair is an inlier to the estimate
  /// \param [in]  inlier_scale error distance tolerated for matches to be
  /// inliers
  static void
  mark_inliers(
    vital::fundamental_matrix_sptr const& fm,
    std::vector< vital::vector_2d > const& pts1,
    std::vector< vital::vector_2d > const& pts2,
    std::vector< bool >& inliers,
    double inlier_scale = 1.0 );

  ENUM_CONVERTER(
    method_converter, method_t,
    { "EST_7_POINT",   EST_7_POINT },
    { "EST_8_POINT",   EST_8_POINT } )

private:
  void initialize() override;
  /// private implementation class
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif
