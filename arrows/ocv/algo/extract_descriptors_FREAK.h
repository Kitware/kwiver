// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV FREAK descriptor extractor wrapper

#ifndef KWIVER_ARROWS_EXTRACT_DESCRIPTORS_FREAK_H_
#define KWIVER_ARROWS_EXTRACT_DESCRIPTORS_FREAK_H_

#include <opencv2/opencv_modules.hpp>
#if KWIVER_OPENCV_VERSION_MAJOR < 3 || defined( HAVE_OPENCV_XFEATURES2D )

#include <memory>
#include <string>

#include <arrows/ocv/algo/extract_descriptors.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_FREAK
  : public extract_descriptors
{
public:
  PLUGGABLE_IMPL(
    extract_descriptors_FREAK,
    "OpenCV feature-point descriptor extraction via the FREAK algorithm",

    PARAM_DEFAULT(
      orientation_normalized, bool,
      "enable orientation normalization",
      true ),

    PARAM_DEFAULT(
      scale_normalized, bool,
      "enable scale normalization",
      true ),

    PARAM_DEFAULT(
      pattern_scale, float,
      "scaling of the description pattern",
      22.0f ),

    PARAM_DEFAULT(
      n_octaves, int,
      "number of octaves covered by the detected keypoints",
      4 )
  );

  /// Destructor
  virtual ~extract_descriptors_FREAK();

  /// Check that the algorithm's configuration config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_extractor_parameters() const override;
};

#define KWIVER_OCV_HAS_FREAK

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // has OCV support

#endif // KWIVER_ARROWS_EXTRACT_DESCRIPTORS_FREAK_H_
