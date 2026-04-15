// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV LUCID descriptor extractor wrapper

#ifndef KWIVER_ARROWS_EXTRACT_DESCRIPTORS_LUCID_H_
#define KWIVER_ARROWS_EXTRACT_DESCRIPTORS_LUCID_H_

#include <opencv2/opencv_modules.hpp>
#ifdef HAVE_OPENCV_XFEATURES2D

#include <memory>
#include <string>

#include <arrows/ocv/algo/extract_descriptors.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_LUCID
  : public ocv::extract_descriptors
{
public:
  // "ocv_LUCID",

  PLUGGABLE_IMPL(
    extract_descriptors_LUCID,
    "OpenCV feature-point descriptor extraction via the LUCID algorithm",

    PARAM_DEFAULT(
      lucid_kernel, int,
      "kernel for descriptor construction, where 1=3x3, "
      "2=5x5, 3=7x7 and so forth",
      1 ),

    PARAM_DEFAULT(
      blur_kernel, int,
      "kernel for blurring image prior to descriptor "
      "construction, where 1=3x3, 2=5x5, 3=7x7 and so forth",
      1 )
  );

  /// Destructor
  virtual ~extract_descriptors_LUCID();

  /// Check that the algorithm's configuration config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_extractor_parameters() const override;
};

#define KWIVER_OCV_HAS_LUCID

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D

#endif // KWIVER_ARROWS_EXTRACT_DESCRIPTORS_LUCID_H_
