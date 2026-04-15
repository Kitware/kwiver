// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_ESTIMATE_CANONICAL_TRANSFORM_H_
#define KWIVER_ARROWS_VXL_ESTIMATE_CANONICAL_TRANSFORM_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/estimate_canonical_transform.h>

#include <vital/util/enum_converter.h>

/// \file
/// \brief Header defining the VXL estimate_canonical_transform algorithm

namespace kwiver {

namespace arrows {

namespace vxl {

enum rrel_methodtypes { RANSAC, LMS, IRLS, };

/// Algorithm for estimating a canonical transform for cameras and landmarks
///
///  A canonical transform is a repeatable transformation that can be recovered
///  from data.  In this case we assume at most a similarity transformation.
///  If data sets P1 and P2 are equivalent up to a similarity transformation,
///  then applying a canonical transform to P1 and separately a
///  canonical transform to P2 should bring the data into the same coordinates.
///
///  This implementation first fits a "ground" plane to the landmark points
///  using robust estimation methods provided by the rrel library in VXL.
///  It then estimates the remaining degrees of freedom using PCA much like
///  the implementation in the core plugin.  The scale is set to normalize the
///  landmarks to unit standard deviation.
class KWIVER_ALGO_VXL_EXPORT estimate_canonical_transform
  : public vital::algo::estimate_canonical_transform
{
public:
  PLUGGABLE_IMPL(
    estimate_canonical_transform,
    "Use VXL (rrel) to robustly estimate a ground plane for a canonical transform.",
    PARAM_DEFAULT(
      estimate_scale, bool,
      "Estimate the scale to normalize the data. "
      "If disabled the estimate transform is rigid",
      true ),
    PARAM_DEFAULT(
      trace_level, int,
      "Integer value controlling the verbosity of the "
      "plane search algorithms (0->no output, 3->max output).",
      0 ),
    PARAM_DEFAULT(
      rrel_method, std::string,
      "The robust estimation algorithm to use for plane "
      "fitting. Options are: " +
      rrel_converter().element_name_string(),
      rrel_converter().to_string( IRLS ) ),
    PARAM_DEFAULT(
      desired_prob_good, double,
      "The desired probability of finding the correct plane fit.",
      0.99 ),
    PARAM_DEFAULT(
      max_outlier_frac, double,
      "The maximum fraction of the landmarks that is expected "
      "outliers to the ground plane.",
      0.75 ),
    PARAM_DEFAULT(
      prior_inlier_scale, double,
      "The initial estimate of inlier scale for RANSAC "
      "fitting of the ground plane.",
      0.1 ),
    PARAM_DEFAULT(
      irls_max_iterations, int,
      "The maximum number if iterations when using IRLS",
      15 ),
    PARAM_DEFAULT(
      irls_iterations_for_scale, int,
      "The number of IRLS iterations in which to estimate scale",
      2 ),
    PARAM_DEFAULT(
      irls_conv_tolerance, double,
      "The convergence tolerance for IRLS",
      1e-4 )
  )

  virtual ~estimate_canonical_transform() = default;

  /// Set this algorithm's properties via a config block
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Estimate a canonical similarity transform for cameras and points
  ///
  /// \param cameras The camera map containing all the cameras
  /// \param landmarks The landmark map containing all the 3D landmarks
  /// \throws algorithm_exception When the data is insufficient or degenerate.
  /// \returns An estimated similarity transform mapping the data to the
  ///          canonical space.
  /// \note This algorithm does not apply the transformation, it only estimates
  /// it.
  virtual kwiver::vital::similarity_d
  estimate_transform(
    kwiver::vital::camera_map_sptr const cameras,
    kwiver::vital::landmark_map_sptr const landmarks ) const;

  ENUM_CONVERTER(
    rrel_converter, rrel_methodtypes,
    { "RANSAC", RANSAC },
    { "LMS", LMS },
    { "IRLS", IRLS } );

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
