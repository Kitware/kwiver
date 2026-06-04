// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for VXL bundle adjustment algorithm

#ifndef KWIVER_ARROWS_VXL_BUNDLE_ADJUST_H_
#define KWIVER_ARROWS_VXL_BUNDLE_ADJUST_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/bundle_adjust.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// A class for bundle adjustment of feature tracks using VXL
class KWIVER_ALGO_VXL_EXPORT bundle_adjust
  : public vital::algo::bundle_adjust
{
public:
  PLUGGABLE_IMPL(
    bundle_adjust,
    "Use VXL (vpgl) to bundle adjust cameras and landmarks.",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "optimization progress at each iteration",
      false ),
    PARAM_DEFAULT(
      use_m_estimator, bool,
      "If true, use a M-estimator for a robust loss function. "
      "Currently only the Beaton-Tukey loss function is supported.",
      false ),
    PARAM_DEFAULT(
      m_estimator_scale, double,
      "The scale of the M-estimator, if enabled, in pixels. "
      "Inlier landmarks should project to within this distance "
      "from the feature point.",
      1.0 ),
    PARAM_DEFAULT(
      estimate_focal_length, bool,
      "If true, estimate a shared intrinsic focal length for all "
      "cameras.  Warning: there is often a depth/focal length "
      "ambiguity which can lead to long optimizations.",
      false ),
    PARAM_DEFAULT(
      normalize_data, bool,
      "Normalize the data for numerical stability. "
      "There is no reason not enable this option, except "
      "for testing purposes.",
      true ),
    PARAM_DEFAULT(
      max_iterations, unsigned,
      "Termination condition: maximum number of LM iterations",
      1000 ),
    PARAM_DEFAULT(
      x_tolerance, double,
      "Termination condition: Relative change is parameters. "
      "Exit when (mag(delta_params) / mag(params) < x_tol).",
      1e-8 ),
    PARAM_DEFAULT(
      g_tolerance, double,
      "Termination condition: Maximum gradient magnitude. "
      "Exit when (max(grad_params) < g_tol)",
      1e-8 )
  )

  /// Destructor
  virtual ~bundle_adjust() = default;

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Optimize the camera and landmark parameters given a set of feature tracks
  ///
  /// \param [in,out] cameras the cameras to optimize
  /// \param [in,out] landmarks the landmarks to optimize
  /// \param [in] tracks the feature tracks to use as constraints
  /// \param [in] metadata the frame metadata to use as constraints
  virtual void
  optimize(
    vital::camera_map_sptr& cameras,
    vital::landmark_map_sptr& landmarks,
    vital::feature_track_set_sptr tracks,
    vital::sfm_constraints_sptr constraints = nullptr ) const;

  using vital::algo::bundle_adjust::optimize;

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
