// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining Ceres algorithm implementation of camera optimization.

#ifndef KWIVER_ARROWS_CERES_OPTIMIZE_CAMERAS_H_
#define KWIVER_ARROWS_CERES_OPTIMIZE_CAMERAS_H_

#include <arrows/ceres/kwiver_algo_ceres_export.h>

#include <vital/algo/optimize_cameras.h>

#include <memory>

namespace kwiver {
namespace arrows {
namespace ceres {

/// A class for optimization of camera paramters using Ceres
class KWIVER_ALGO_CERES_EXPORT optimize_cameras
: public vital::algo::optimize_cameras
{
public:
  /// Constructor
  optimize_cameras();

  /// Destructor
  virtual ~optimize_cameras();

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block
  virtual void set_configuration(vital::config_block_sptr config);
  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration(vital::config_block_sptr config) const;

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
  optimize(kwiver::vital::camera_map_sptr & cameras,
           kwiver::vital::feature_track_set_sptr tracks,
           kwiver::vital::landmark_map_sptr landmarks,
           kwiver::vital::sfm_constraints_sptr constraints = nullptr) const;

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
  optimize(vital::camera_perspective_sptr & camera,
           const std::vector<vital::feature_sptr>& features,
           const std::vector<vital::landmark_sptr>& landmarks,
           kwiver::vital::sfm_constraints_sptr constraints = nullptr) const;

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d_;
};

} // end namespace ceres
} // end namespace arrows
} // end namespace kwiver

#endif
