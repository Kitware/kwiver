// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for Ceres bundle adjustment algorithm

#ifndef KWIVER_ARROWS_CERES_BUNDLE_ADJUST_H_
#define KWIVER_ARROWS_CERES_BUNDLE_ADJUST_H_

#include <arrows/ceres/kwiver_algo_ceres_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/bundle_adjust.h>

#include <arrows/ceres/config_options_helpers.txx>
#include <arrows/ceres/options.h>

#include <memory>

namespace kwiver {

namespace arrows {

namespace ceres {

/// A class for bundle adjustment of feature tracks using Ceres
class KWIVER_ALGO_CERES_EXPORT bundle_adjust
  : public vital::algo::bundle_adjust
{
public:
  PLUGGABLE_IMPL(
    bundle_adjust,
    "Uses Ceres Solver to bundle adjust camera and landmark parameters.",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "optimization progress at each iteration.", false ),
    PARAM_DEFAULT(
      log_full_report, bool,
      "If true, log a full report of optimization stats at "
      "the end of optimization.", false ),
    PARAM_DEFAULT(
      loss_function_type, LossFunctionType,
      "Robust loss function type to use.",
      TRIVIAL_LOSS ),
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
  virtual ~bundle_adjust() = default;

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Optimize the camera and landmark parameters given a set of feature tracks
  /// \param [in,out] cameras The cameras to optimize.
  /// \param [in,out] landmarks The landmarks to optimize.
  /// \param [in] tracks The feature tracks to use as constraints.
  /// \param [in] metadata The frame metadata to use as constraints.
  virtual void
  optimize(
    vital::camera_map_sptr& cameras,
    vital::landmark_map_sptr& landmarks,
    vital::feature_track_set_sptr tracks,
    vital::sfm_constraints_sptr constraints = nullptr ) const;

  /// Optimize the camera and landmark parameters given a set of feature tracks
  /// \param [in,out] cameras the cameras to optimize
  /// \param [in,out] landmarks the landmarks to optimize
  /// \param [in] tracks the feature tracks to use as constraints
  /// \param [in] fixed_cameras frame ids for cameras to be fixed in the
  /// optimization
  /// \param [in] fixed_landmarks landmark ids for landmarks to be fixed in the
  /// optimization
  /// \param [in] metadata the frame metadata to use as constraints
  virtual void
  optimize(
    kwiver::vital::simple_camera_perspective_map& cameras,
    kwiver::vital::landmark_map::map_landmark_t& landmarks,
    vital::feature_track_set_sptr tracks,
    const std::set< vital::frame_id_t >& fixed_cameras,
    const std::set< vital::landmark_id_t >& fixed_landmarks,
    kwiver::vital::sfm_constraints_sptr constraints = nullptr ) const;

  /// Set a callback function to report intermediate progress
  virtual void set_callback( callback_t cb );

  /// This function is called by a Ceres callback to trigger a kwiver callback
  bool trigger_callback();

protected:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;

private:
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

typedef std::shared_ptr< bundle_adjust >
  bundle_adjust_sptr;

} // namespace ceres

} // namespace arrows

} // namespace kwiver

#endif
