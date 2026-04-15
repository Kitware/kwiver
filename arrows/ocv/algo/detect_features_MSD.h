// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV MSD feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_MSD_H_
#define KWIVER_ARROWS_DETECT_FEATURES_MSD_H_

// Only available in OpenCV 3.x xfeatures2d
#include <opencv2/opencv_modules.hpp>
#ifdef HAVE_OPENCV_XFEATURES2D

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_MSD
  : public ocv::detect_features
{
public:
  PLUGGABLE_IMPL(
    detect_features_MSD,
    "OpenCV feature detection via the MSD algorithm",
    PARAM_DEFAULT( patch_radius, int, "patch_radius", 3 ),
    PARAM_DEFAULT( search_area_radius, int, "search_area_radius", 5 ),
    PARAM_DEFAULT( nms_radius, int, "nms_radius", 5 ),
    PARAM_DEFAULT( nms_scale_radius, int, "nms_scale_radius", 0 ),
    PARAM_DEFAULT( th_saliency, float, "th_saliency", 250.0f ),
    PARAM_DEFAULT( knn, int, "knn", 4 ),
    PARAM_DEFAULT( scale_factor, float, "scale_factor", 1.25f ),
    PARAM_DEFAULT( n_scales, int, "n_scales", -1 ),
    PARAM_DEFAULT( compute_orientation, bool, "compute_orientation", false )
  )

  /// Destructor
  virtual ~detect_features_MSD();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
};

#define KWIVER_OCV_HAS_MSD

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D

#endif // KWIVER_ARROWS_DETECT_FEATURES_MSD_H_
