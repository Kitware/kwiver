// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV DAISY descriptor extractor wrapper

#ifndef KWIVER_ARROWS_EXTRACT_DESCRIPTORS_DAISY_H_
#define KWIVER_ARROWS_EXTRACT_DESCRIPTORS_DAISY_H_

#include <opencv2/opencv_modules.hpp>
#ifdef HAVE_OPENCV_XFEATURES2D
#include <opencv2/xfeatures2d.hpp>

#include <memory>
#include <string>

#include <arrows/ocv/algo/extract_descriptors.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_DAISY
  : public ocv::extract_descriptors
{
public:
  PLUGGABLE_IMPL(
    extract_descriptors_DAISY,
    "OpenCV feature-point descriptor extraction via the DAISY algorithm",

    PARAM_DEFAULT(
      radius, float,
      "radius of the descriptor at the initial scale",
      15.0f ),

    PARAM_DEFAULT(
      q_radius, int,
      "amount of radial range division quantity",
      3 ),

    PARAM_DEFAULT(
      q_theta, int,
      "amount of angular range division quantity",
      3 ),

    PARAM_DEFAULT(
      q_hist, int,
      "amount of gradient orientations range division quantity",
      8 ),

    PARAM_DEFAULT(
      norm,
      int,
      "descriptor normalization type. valid choices:\n" +
      list_norm_options,
      cv::xfeatures2d::DAISY::NRM_NONE ),

    PARAM_DEFAULT(
      interpolation, bool,
      "", true ),

    PARAM_DEFAULT(
      use_orientation, bool,
      "", false ),
  );

  /// Destructor
  virtual ~extract_descriptors_DAISY();

  /// Check that the algorithm's configuration config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  static const std::string list_norm_options;

private:
  void initialize() override;
  void update_extractor_parameters() const override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
};

#define KWIVER_OCV_HAS_DAISY

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D

#endif // KWIVER_ARROWS_EXTRACT_DESCRIPTORS_DAISY_H_
