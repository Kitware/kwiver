// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV AGAST feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_AGAST_H_
#define KWIVER_ARROWS_DETECT_FEATURES_AGAST_H_

// Only available in OpenCV 3.x
#if KWIVER_OPENCV_VERSION_MAJOR >= 3

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_AGAST
  : public ocv::detect_features
{
public:
  PLUGGABLE_IMPL(
    detect_features_AGAST,
    "OpenCV feature detection via the AGAST algorithm",

    PARAM_DEFAULT(
      threshold, int,
      "Integer threshold on difference between intensity of "
      "the central pixel and pixels of a circle around this "
      "pixel", 10 ),

    PARAM_DEFAULT(
      nonmax_suppression, bool,
      "if true, non-maximum suppression is applied to "
      "detected corners (keypoints)", true ),

    PARAM_DEFAULT(
      type, int,
      "Neighborhood pattern type. Should be one of the "
      "following enumeration type values:\n" +
      list_agast_types +  " (default)",
      static_cast< int >( cv::AgastFeatureDetector::OAST_9_16 )
    )

  );

  /// Destructor
  virtual ~detect_features_AGAST();

  bool check_configuration( vital::config_block_sptr config ) const override;

  static const std::string list_agast_types;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
};

#define KWIVER_OCV_HAS_AGAST

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_OPENCV_VERSION_MAJOR >= 3

#endif // KWIVER_ARROWS_DETECT_FEATURES_AGAST_H_
