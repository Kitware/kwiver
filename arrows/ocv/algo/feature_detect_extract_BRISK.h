// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV BRISK feature detector and extractor wrapper

#ifndef KWIVER_ARROWS_FEATURE_DETECT_EXTRACT_BRISK_H_
#define KWIVER_ARROWS_FEATURE_DETECT_EXTRACT_BRISK_H_

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/algo/extract_descriptors.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_BRISK
  : public ocv::detect_features
{
public:
  PLUGGABLE_IMPL(
    detect_features_BRISK,
    "OpenCV feature detection via the BRISK algorithm",

    PARAM_DEFAULT(
      threshold, int,
      "AGAST detection threshold score.",
      30 ),

    PARAM_DEFAULT(
      octaves, int,
      "detection octaves. Use 0 to do single scale.",
      3 ),

    PARAM_DEFAULT(
      pattern_scale, float,
      "apply this scale to the pattern used for sampling the "
      "neighbourhood of a keypoint.",
      1.0f )
  );

  /// Destructor
  virtual ~detect_features_BRISK();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
  cv::Ptr< cv::BRISK > create() const;
};

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_BRISK
  : public ocv::extract_descriptors
{
public:
  PLUGGABLE_IMPL(
    extract_descriptors_BRISK,
    "OpenCV feature-point descriptor extraction via the BRISK algorithm",

    PARAM_DEFAULT(
      threshold, int,
      "AGAST detection threshold score.",
      30 ),

    PARAM_DEFAULT(
      octaves, int,
      "detection octaves. Use 0 to do single scale.",
      3 ),

    PARAM_DEFAULT(
      pattern_scale, float,
      "apply this scale to the pattern used for sampling the "
      "neighbourhood of a keypoint.",
      1.0f )
  );

  /// Destructor
  virtual ~extract_descriptors_BRISK();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_extractor_parameters() const override;
  cv::Ptr< cv::BRISK > create() const;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif
