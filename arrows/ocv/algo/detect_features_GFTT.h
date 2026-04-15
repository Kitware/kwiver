// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV GFTT feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_GFTT_H_
#define KWIVER_ARROWS_DETECT_FEATURES_GFTT_H_

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_GFTT
  : public ocv::detect_features
{
public:
  PLUGGABLE_IMPL(
    detect_features_GFTT,
    "OpenCV feature detection via the GFTT algorithm",
    PARAM_DEFAULT( max_corners, int, "max_corners", 1000 ),
    PARAM_DEFAULT( quality_level, double, "quality_level", 0.01 ),
    PARAM_DEFAULT( min_distance, double, "min_distance", 1.0 ),
    PARAM_DEFAULT( block_size, int, "block_size", 3 ),
    PARAM_DEFAULT( use_harris_detector, bool, "use_harris_detector", false ),
    PARAM_DEFAULT( k, double, "k", 0.04 )
  );

  /// Destructor
  virtual ~detect_features_GFTT();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
  void initialize() override;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_ARROWS_DETECT_FEATURES_GFTT_H_
