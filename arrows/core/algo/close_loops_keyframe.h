// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining the close_loops_keyframe algorithm

#ifndef KWIVER_ARROWS_CLOSE_LOOPS_KEYFRAME_H_
#define KWIVER_ARROWS_CLOSE_LOOPS_KEYFRAME_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/close_loops.h>

#include <vital/algo/algorithm.txx>

#include <vital/algo/match_features.h>

namespace kwiver {

namespace arrows {

namespace core {

/// Attempts to stitch over previous frames.
///
/// This class attempts close loops with all previous (or as specified) frames
class KWIVER_ALGO_CORE_EXPORT close_loops_keyframe
  : public vital::algo::close_loops
{
public:
  PLUGGABLE_IMPL(
    close_loops_keyframe,
    "Establishes keyframes matches to all keyframes.",
    PARAM_DEFAULT(
      match_req, int,
      "The required number of features needed to be matched for a success.",
      100 ),
    PARAM_DEFAULT(
      search_bandwidth, int,
      "Number of adjacent frames to match to (must be at least 1).",
      10 ),
    PARAM_DEFAULT(
      min_keyframe_misses, size_t,
      "Minimum number of keyframe match misses before creating a new keyframe. "
      "A match miss occurs when the current frame does not match any existing "
      "keyframe (must be at least 1).",
      5 ),
    PARAM_DEFAULT(
      stop_after_match, bool,
      "If set, stop matching additional keyframes after at least "
      "one match is found and then one fails to match.  This "
      "prevents making many comparisons to keyframes that are "
      "likely to fail, but it also misses unexpected matches "
      "that could make the tracks stronger.",
      false ),
    PARAM(
      feature_matcher, kwiver::vital::algo::match_features_sptr,
      "feature_matcher" )
  )

  /// Destructor
  virtual ~close_loops_keyframe() noexcept;

  /// Check that the algorithm's currently configuration is valid
  ///
  /// This checks solely within the provided \c config_block and not against
  /// the current state of the instance. This isn't static for inheritence
  /// reasons.
  ///
  /// \param config  The config block to check configuration of.
  ///
  /// \returns true if the configuration check passed and false if it didn't.
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Perform keyframe guided stitching
  ///
  /// \param [in] frame_number the frame number of the current frame
  /// \param [in] input the input feature track set to stitch
  /// \param [in] image image data for the current frame
  /// \param [in] mask Optional mask image where positive values indicate
  ///                  regions to consider in the input image.
  /// \returns an updated set of feature tracks after the stitching operation
  virtual vital::feature_track_set_sptr
  stitch(
    vital::frame_id_t frame_number,
    vital::feature_track_set_sptr input,
    vital::image_container_sptr image,
    vital::image_container_sptr mask = vital::image_container_sptr() ) const;

private:
  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

} // end namespace core

} // end namespace arrows

} // end namespace kwiver

#endif
