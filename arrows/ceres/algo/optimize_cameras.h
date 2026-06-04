// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining Ceres algorithm implementation of camera
/// optimization.

#ifndef KWIVER_ARROWS_CERES_OPTIMIZE_CAMERAS_H_
#define KWIVER_ARROWS_CERES_OPTIMIZE_CAMERAS_H_

#include <arrows/ceres/kwiver_algo_ceres_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/optimize_cameras.h>

#include <arrows/ceres/config_options_helpers.txx>
#include <arrows/ceres/options.h>

#include <memory>

namespace kwiver {

namespace arrows {

namespace ceres {

/// A class for optimization of camera paramters using Ceres
class KWIVER_ALGO_CERES_EXPORT optimize_cameras
  : public vital::algo::optimize_cameras
{
public:
  PLUGGABLE_IMPL(
    optimize_cameras,
    "Uses Ceres Solver to optimize camera parameters",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "optimization progress at each iteration", false ),
    PARAM_DEFAULT(
      loss_function_type, LossFunctionType,
      "Robust loss function type to use.", TRIVIAL_LOSS ),
    PARAM_DEFAULT(
      loss_function_scale, double,
      "Robust loss function scale factor.", 1.0 ),
    PARAM(
      solver_options, solver_options_sptr,
      "pointer to the nested config options for solver" ),
    PARAM(
      camera_options, camera_options_sptr,
      "pointer to the nested config options for camera" )
  )
  /// Destructor
  virtual ~optimize_cameras() = default;

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Optimize camera parameters given sets of landmarks and feature tracks
  ///
  /// We only optimize cameras that have associating tracks and landmarks in
  /// the given maps.  The default implementation collects the corresponding
  /// features and landmarks for each camera and calls the single camera
  /// optimize function.
  ///
  /// \throws invalid_value When one or more of the given pointer is Null.
  ///
  /// \param[in,out] cameras   Cameras to optimize.
  /// \param[in]     tracks    The feature tracks to use as constraints.
  /// \param[in]     landmarks The landmarks the cameras are viewing.
  /// \param[in]     metadata  The optional metadata to constrain the
  ///                          optimization.
  virtual void
  optimize(
    kwiver::vital::camera_map_sptr& cameras,
    kwiver::vital::feature_track_set_sptr tracks,
    kwiver::vital::landmark_map_sptr landmarks,
    kwiver::vital::sfm_constraints_sptr constraints = nullptr ) const;

  /// Optimize a single camera given corresponding features and landmarks
  ///
  /// This function assumes that 2D features viewed by this camera have
  /// already been put into correspondence with 3D landmarks by aligning
  /// them into two parallel vectors
  ///
  /// \param[in,out] camera    The camera to optimize.
  /// \param[in]     features  The vector of features observed by \p camera
  ///                          to use as constraints.
  /// \param[in]     landmarks The vector of landmarks corresponding to
  ///                          \p features.
  /// \param[in]     metadata  The optional metadata to constrain the
  ///                          optimization.
  virtual void
  optimize(
    vital::camera_perspective_sptr& camera,
    const std::vector< vital::feature_sptr >& features,
    const std::vector< vital::landmark_sptr >& landmarks,
    kwiver::vital::sfm_constraints_sptr constraints = nullptr ) const;

protected:
  void initialize() override;

private:
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

} // end namespace ceres

} // end namespace arrows

} // end namespace kwiver

#endif
