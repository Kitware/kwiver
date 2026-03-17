// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header defining the hierarchical_bundle_adjust algorithm
 */

#ifndef KWIVER_ARROWS_MVG_HIERARCHICAL_BUNDLE_ADJUST_H_
#define KWIVER_ARROWS_MVG_HIERARCHICAL_BUNDLE_ADJUST_H_

#include <arrows/mvg/kwiver_algo_mvg_export.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/bundle_adjust.h>
#include <vital/algo/optimize_cameras.h>
#include <vital/algo/triangulate_landmarks.h>
#include <vital/config/config_block.h>

namespace kwiver {

namespace arrows {

namespace mvg {

class KWIVER_ALGO_MVG_EXPORT hierarchical_bundle_adjust
  : public vital::algo::bundle_adjust
{
public:
  PLUGGABLE_IMPL(
    hierarchical_bundle_adjust,
    "Run a bundle adjustment algorithm in a temporally hierarchical fashion"
    " (useful for video)",
    PARAM_DEFAULT(
      initial_sub_sample, long,
      "Sub-sample the given cameras by this factor. Gaps will "
      "then be filled in by iterations of interpolation.", 1 ),

    PARAM_DEFAULT(
      interpolation_rate, long,
      "Number of cameras to fill in each iteration. When this "
      "is set to 0, we will interpolate all missing cameras "
      "at the first moment possible.", 0 ),

    PARAM_DEFAULT(
      rmse_reporting_enabled, bool,
      "Enable the reporting of RMSE statistics at various "
      "stages of this algorithm. Constant calculating of RMSE "
      "may effect run time of the algorithm.", false ),
    PARAM(
      sba_impl, vital::algo::bundle_adjust_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      camera_optimizer, vital::algo::optimize_cameras_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      lm_triangulator, vital::algo::triangulate_landmarks_sptr,
      "pointer to the nested algorithm" )
  )
  /// Destructor
  virtual ~hierarchical_bundle_adjust() noexcept;

  /// Check that the algorithm's configuration vital::config_block is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Optimize the camera and landmark parameters given a set of tracks
  virtual void optimize(
    vital::camera_map_sptr& cameras,
    vital::landmark_map_sptr& landmarks,
    vital::feature_track_set_sptr tracks,
    vital::sfm_constraints_sptr constraints = nullptr ) const;

  using vital::algo::bundle_adjust::optimize;

protected:
  void initialize() override;

private:
  // private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

/// Type definition for shared pointer for hierarchical_bundle_adjust algorithm
typedef std::shared_ptr< hierarchical_bundle_adjust >
  hierarchical_bundle_adjust_sptr;

} // end namespace mvg

} // end namespace arrows

} // end namespace kwiver

#endif
