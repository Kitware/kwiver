// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV brute-force feature matcher wrapper

#ifndef KWIVER_ARROWS_MATCH_FEATURES_BRUTEFORCE_H_
#define KWIVER_ARROWS_MATCH_FEATURES_BRUTEFORCE_H_

#include <memory>
#include <vector>

#include <arrows/ocv/algo/match_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {

namespace arrows {

namespace ocv {

/// Feature matcher implementation using OpenCV's brute-force feature matcher
class KWIVER_ALGO_OCV_EXPORT match_features_bruteforce
  : public match_features
{
public:
  PLUGGABLE_IMPL(
    match_features_bruteforce,
    "OpenCV feature matcher using brute force matching (exhaustive search).",

    PARAM_DEFAULT(
      cross_check, bool,
      "Perform cross checking when finding matches to filter "
      "through only the consistent pairs. This is an "
      "alternative to the ratio test used by D. Lowe in the "
      "SIFT paper.",
      false ),

    PARAM_DEFAULT(
      norm_type, int,
      std::string(
        "normalization type enum value. this should be one of the enum values:" )
      +
      list_enum_values
      ,
      cv::NORM_L2 )

  );

  /// Destructor
  virtual ~match_features_bruteforce();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  static const std::string list_enum_values;

protected:
  /// Perform matching based on the underlying OpenCV implementation
  void ocv_match(
    const cv::Mat& descriptors1,
    const cv::Mat& descriptors2,
    std::vector< cv::DMatch >& matches ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;

  cv::Ptr< cv::BFMatcher > matcher;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_ARROWS_MATCH_FEATURES_BRUTEFORCE_H_
