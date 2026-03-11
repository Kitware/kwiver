// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header for older basic camera and landmark initialization algorithm
 */

#ifndef KWIVER_ARROWS_MVG_INITIALIZE_CAMERAS_LANDMARKS_BASIC_H_
#define KWIVER_ARROWS_MVG_INITIALIZE_CAMERAS_LANDMARKS_BASIC_H_

#include <arrows/mvg/kwiver_algo_mvg_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/bundle_adjust.h>
#include <vital/algo/estimate_essential_matrix.h>
#include <vital/algo/initialize_cameras_landmarks.h>
#include <vital/algo/optimize_cameras.h>
#include <vital/algo/triangulate_landmarks.h>
#include <vital/config/config_camera_helpers.txx>
#include <vital/types/camera_intrinsics.h>
#include <vital/types/vector.h>

namespace kwiver {

namespace arrows {

namespace mvg {

/// A class for initialization of cameras and landmarks
class KWIVER_ALGO_MVG_EXPORT initialize_cameras_landmarks_basic
  : public vital::algo::initialize_cameras_landmarks
{
public:
  PLUGGABLE_IMPL(
    initialize_cameras_landmarks_basic,
    "Run SfM to iteratively estimate new cameras and landmarks"
    " using feature tracks.",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "debugging information", false ),

    PARAM_DEFAULT(
      init_from_last, bool,
      "If true, and a camera optimizer is specified, initialize "
      "the camera using the closest exiting camera and optimize", false ),

    PARAM_DEFAULT(
      retriangulate_all, bool,
      "If true, re-triangulate all landmarks observed by a newly "
      "initialized camera.  Otherwise, only triangulate or "
      "re-triangulate landmarks that are marked for initialization.", false ),
    PARAM_DEFAULT(
      reverse_ba_error_ratio, double,
      "After final bundle adjustment, if the Necker reversal of "
      "the solution increases the RMSE by less than this factor, "
      "then run a bundle adjustment on the reversed data and "
      "choose the final solution with the lowest error.  Set to "
      "zero to disable.", 2.0 ),

    PARAM_DEFAULT(
      next_frame_max_distance, size_t,
      "Limit the selection of the next frame to initialize to "
      "within this many frames of an already initialized frame. "
      "If no valid frames are found, double the search range "
      "until a valid frame is found. "
      "A value of zero disables this limit", 0 ),
    PARAM_DEFAULT(
      global_ba_rate, double,
      "Run a global bundle adjustment every time the number of "
      "cameras in the system grows by this multiple.", 1.5 ),

    PARAM_DEFAULT(
      interim_reproj_thresh, double,
      "Threshold for rejecting landmarks based on reprojection "
      "error (in pixels) during intermediate processing steps.", 5.0 ),

    PARAM_DEFAULT(
      final_reproj_thresh, double,
      "Relative threshold for rejecting landmarks based on "
      "reprojection error relative to the median error after "
      "the final bundle adjustment.  For example, a value of 2 "
      "mean twice the median error", 2.0 ),

    PARAM_DEFAULT(
      zoom_scale_thresh, double,
      "Threshold on image scale change used to detect a camera "
      "zoom. If the resolution on target changes by more than "
      "this fraction create a new camera intrinsics model.", 0.1 ),

    PARAM_DEFAULT(
      base_camera_focal_length, double,
      "focal length of the base camera model", 1.0 ),

    PARAM_DEFAULT(
      base_camera_principal_point, vital::vector_2d,
      "The principal point of the base camera model \"x y\".\n"
      "It is usually safe to assume this is the center of the "
      "image.", vital::vector_2d( { 0, 0 } ) ),

    PARAM_DEFAULT(
      base_camera_aspect_ratio, double,
      "the pixel aspect ratio of the base camera model", 1.5 ),

    PARAM_DEFAULT(
      base_camera_skew, double,
      "The skew factor of the base camera model.\n"
      "This is almost always zero in any real camera.", 0.0 ),

    PARAM(
      base_camera, vital::camera_intrinsics_sptr,
      "base camera model parameters group" ),

    // nested algorithm configurations
    PARAM(
      essential_mat_estimator, vital::algo::estimate_essential_matrix_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      camera_optimizer, vital::algo::optimize_cameras_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      lm_triangulator, vital::algo::triangulate_landmarks_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      bundle_adjuster, vital::algo::bundle_adjust_sptr,
      "pointer to the nested algorithm" )
  )
  /// Destructor
  virtual ~initialize_cameras_landmarks_basic();

  /// Copy Constructor
  initialize_cameras_landmarks_basic(
    const initialize_cameras_landmarks_basic& other );

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Initialize the camera and landmark parameters given a set of feature
  /// tracks

  /**
   * The algorithm creates an initial estimate of any missing cameras and
   * landmarks using the available cameras, landmarks, and feature tracks.
   * If the input cameras map is a NULL pointer then the algorithm should try
   * to initialize all cameras covered by the track set.  If the input camera
   * map exists then the algorithm should only initialize cameras on frames for
   * which the camera is set to NULL.  Frames not in the map will not be
   * initialized.  This allows the caller to control which subset of cameras to
   * initialize without needing to manipulate the feature tracks.
   * The analogous behavior is also applied to the input landmarks map to
   * select which track IDs should be used to initialize landmarks.
   *
   * \note This algorithm may optionally revise the estimates of existing
   * cameras and landmarks passed as input.
   *
   * \param [in,out] cameras the cameras to initialize
   * \param [in,out] landmarks the landmarks to initialize
   * \param [in] tracks the feature tracks to use as constraints
   * \param [in] metadata the frame metadata to use as constraints
   */
  virtual void
  initialize(
    vital::camera_map_sptr& cameras,
    vital::landmark_map_sptr& landmarks,
    vital::feature_track_set_sptr tracks,
    vital::sfm_constraints_sptr constraints = nullptr ) const;

  /// Set a callback function to report intermediate progress
  virtual void set_callback( callback_t cb );

protected:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;

private:
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

} // end namespace mvg

} // end namespace arrows

} // end namespace kwiver

#endif
