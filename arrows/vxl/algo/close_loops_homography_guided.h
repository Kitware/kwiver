// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining the vxl \link
/// arrows::vxl::close_loops_homography_guided
///        close_loops \endlink algorithm

#ifndef KWIVER_ARROWS_VXL_CLOSE_LOOPS_HOMOGRAPHY_GUIDED_H_
#define KWIVER_ARROWS_VXL_CLOSE_LOOPS_HOMOGRAPHY_GUIDED_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/types/feature_track_set.h>
#include <vital/types/image_container.h>

#include <vital/algo/close_loops.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Attempts to stitch feature tracks over a long period of time.
///
/// This class attempts to make longer-term loop closures by utilizing a
/// variety of techniques, one of which involves using homographies to
/// estimate potential match locations in the past, followed up by additional
/// filtering.
class KWIVER_ALGO_VXL_EXPORT close_loops_homography_guided
  : public vital::algo::close_loops
{
public:
  PLUGGABLE_IMPL(
    close_loops_homography_guided,
    "Use VXL to estimate a sequence of ground plane homographies to identify "
    "frames to match for loop closure.",
    PARAM_DEFAULT(
      enabled, bool,
      "Is long term loop closure enabled?",
      true ),
    PARAM_DEFAULT(
      max_checkpoint_frames, unsigned,
      "Maximum past search distance in terms of number of checkpoints.",
      10000 ),
    PARAM_DEFAULT(
      checkpoint_percent_overlap, double,
      "Term which controls when we make new loop closure checkpoints. "
      "Everytime the percentage of tracked features drops below this "
      "threshold, we generate a new checkpoint.",
      0.70 ),
    PARAM_DEFAULT(
      homography_filename, std::string,
      "Optional output location for a homography text file.",
      "" )
  );

  /// Destructor
  virtual ~close_loops_homography_guided() = default;

  /// Check that the algorithm's currently configuration is valid
  ///
  /// This checks solely within the provided \c vital::config_block and not
  /// against
  /// the current state of the instance. This isn't static for inheritence
  /// reasons.
  ///
  /// \param config  The config block to check configuration of.
  ///
  /// \returns true if the configuration check passed and false if it didn't.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Perform loop closure operation.
  ///
  /// \param frame_number the frame number of the current frame
  /// \param input the input feature track set to stitch
  /// \param image image data for the current frame
  /// \param mask Optional mask image where positive values indicate
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
  /// Class for storing other internal variables
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif
