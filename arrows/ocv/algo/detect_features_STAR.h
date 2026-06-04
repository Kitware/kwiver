// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV Star feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_STAR_H_
#define KWIVER_ARROWS_DETECT_FEATURES_STAR_H_

#include <opencv2/opencv_modules.hpp>
#if KWIVER_OPENCV_VERSION_MAJOR < 3 || defined( HAVE_OPENCV_XFEATURES2D )

#include <memory>
#include <string>

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_STAR
  : public ocv::detect_features
{
public:
  PLUGGABLE_IMPL(
    detect_features_STAR,
    "OpenCV feature detection via the STAR algorithm",

    PARAM_DEFAULT( max_size, int, "max_size", 45 ),
    PARAM_DEFAULT( response_threshold, int, "response_threshold", 30 ),
    PARAM_DEFAULT(
      line_threshold_projected, int, "line_threshold_projected",
      10 ),
    PARAM_DEFAULT(
      line_threshold_binarized, int, "line_threshold_binarized",
      8 ),
    PARAM_DEFAULT( suppress_nonmax_size, int, "suppress_nonmax_size", 5 )
  );

  /// Destructor
  virtual ~detect_features_STAR();

  /// Check that the algorithm's configuration config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
};

#define KWIVER_OCV_HAS_STAR

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // has OCV support

#endif // KWIVER_ARROWS_DETECT_FEATURES_STAR_H_
