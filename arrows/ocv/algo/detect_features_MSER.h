// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV MSER feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_MSER_H_
#define KWIVER_ARROWS_DETECT_FEATURES_MSER_H_

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_MSER
  : public detect_features
{
public:
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
#define DETECT_FEATURES_MSER_EXTRA_PARAMETERS \
,                                             \
PARAM_DEFAULT(                                \
    pass2only,                                \
    bool,                                     \
    "Undocumented",                           \
    false )
#else
#define DETECT_FEATURES_MSER_EXTRA_PARAMETERS
#endif
  PLUGGABLE_IMPL(
    detect_features_MSER,
    "OpenCV feature detection via the MSER algorithm",

    PARAM_DEFAULT(
      delta,
      int,
      "Compares (size[i] - size[i-delta]) / size[i-delta]",
      5 ),

    PARAM_DEFAULT(
      min_area,
      int,
      "Prune areas smaller than this",
      60 ),
    PARAM_DEFAULT(
      max_area,
      int,
      "Prune areas larger than this",
      14400 ),
    PARAM_DEFAULT(
      max_variation,
      double,
      "Prune areas that have similar size to its children",
      0.25 ),
    PARAM_DEFAULT(
      min_diversity,
      double,
      "For color images, trace back to cut off MSER with "
      "diversity less than min_diversity",
      0.2 ),
    PARAM_DEFAULT(
      max_evolution,
      int,
      "The color images, the evolution steps.",
      200 ),
    PARAM_DEFAULT(
      area_threshold,
      double,
      "For color images, the area threshold to cause "
      "re-initialization",
      1.01 ),
    PARAM_DEFAULT(
      min_margin,
      double,
      "For color images, ignore too-small regions.",
      0.003 ),
    PARAM_DEFAULT(
      edge_blur_size,
      int,
      "For color images, the aperture size for edge blur",
      5 )
    DETECT_FEATURES_MSER_EXTRA_PARAMETERS
  );

  /// Destructor
  virtual ~detect_features_MSER();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void update_detector_parameters() const override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void initialize() override;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#undef DETECT_FEATURES_MSER_EXTRA_PARAMETERS

#endif // KWIVER_ARROWS_DETECT_FEATURES_MSER_H_
