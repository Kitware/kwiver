// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV simple blob feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_SIMPLE_BLOB_H_
#define KWIVER_ARROWS_DETECT_FEATURES_SIMPLE_BLOB_H_

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_simple_blob
  : public ocv::detect_features
{
public:
  PLUGGABLE_IMPL(
    detect_features_simple_blob,
    "OpenCV feature detection via the simple_blob algorithm.",

    PARAM_DEFAULT(
      threshold_step,
      float,
      "Defines stepping between min and max threshold when "
      "converting the source image to binary images by "
      "applying thresholding with several thresholds from "
      "min_threshold (inclusive) to max_threshold (exclusive) "
      " with distance threshold_step between neighboring "
      "thresholds.",
      default_params.thresholdStep
    ),

    PARAM_DEFAULT(
      threshold_min,
      float,
      "threshold_min",
      default_params.minThreshold
    ),

    PARAM_DEFAULT(
      threshold_max,
      float,
      "threshold_max",
      default_params.maxThreshold
    ),

    PARAM_DEFAULT(
      min_repeatability,
      std::size_t,
      "min_repeatability",
      default_params.minRepeatability
    ),

    PARAM_DEFAULT(
      min_dist_between_blocks,
      float,
      "Close centers form one group that corresponds to one "
      "blob, controlled by this distance value.",
      default_params.minDistBetweenBlobs
    ),
    PARAM_DEFAULT(
      filter_by_color,
      bool,
      "Enable blob filtering by intensity of the binary image "
      "at the center of the blob to blob_color. If they "
      "differ, the blob is filtered out. Use blob_color = 0 "
      "to extract dark blobs and blob_color = 255 to extract "
      "light blobs",
      default_params.filterByColor
    ),
    PARAM_DEFAULT(
      blob_color,
      unsigned char,
      "blob_color",
      default_params.blobColor
    ),

    PARAM_DEFAULT(
      filter_by_area,
      bool,
      "Enable blob filtering by area to those between "
      "min_area (inclusive) and max_area (exclusive).",
      default_params.filterByArea
    ),

    PARAM_DEFAULT(
      min_area,
      float,
      "min_area",
      default_params.minArea
    ),

    PARAM_DEFAULT(
      max_area,
      float,
      "max_area",
      default_params.maxArea
    ),

    PARAM_DEFAULT(
      filter_by_circularity,
      bool,
      "Enable blob filtering by circularity to those between "
      "min_circularity (inclusive) and max_circularity "
      "(exclusive).",
      default_params.filterByCircularity
    ),

    PARAM_DEFAULT(
      min_circularity,
      float,
      "min_circularity",
      default_params.minCircularity
    ),

    PARAM_DEFAULT(
      max_circularity,
      float,
      "max_circularity",
      default_params.maxCircularity
    ),

    PARAM_DEFAULT(
      filter_by_inertia,
      bool,
      "Enable blob filtering by the ratio of inertia between "
      "min_inertia_ratio (inclusive) and max_inertia_ratio "
      "(exclusive).",
      default_params.filterByInertia
    ),

    PARAM_DEFAULT(
      min_inertia_ratio,
      float,
      "min_inertia_ratio",
      default_params.minInertiaRatio
    ),

    PARAM_DEFAULT(
      max_inertia_ratio,
      float,
      "max_inertia_ratio",
      default_params.maxInertiaRatio
    ),

    PARAM_DEFAULT(
      filter_by_convexity,
      bool,
      "Enable filtering by convexity where blobs have "
      "convexity (area / area of blob convex hull) between "
      "min_convexity (inclusive) and max_convexity "
      "(exclusive).",
      default_params.filterByConvexity
    ),

    PARAM_DEFAULT(
      min_convexity,
      float,
      "min_convexity",
      default_params.minConvexity
    ),

    PARAM_DEFAULT(
      max_convexity,
      float,
      "max_convexity",
      default_params.maxConvexity
    )
  );

  /// Destructor
  virtual ~detect_features_simple_blob();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  static const cv::SimpleBlobDetector::Params default_params;

private:
  void update_detector_parameters() const override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void initialize() override;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_ARROWS_DETECT_FEATURES_SIMPLE_BLOB_H_
