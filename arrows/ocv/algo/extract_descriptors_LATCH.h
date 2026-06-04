// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV LATCH descriptor extractor wrapper

#ifndef KWIVER_ARROWS_EXTRACT_DESCRIPTORS_LATCH_H_
#define KWIVER_ARROWS_EXTRACT_DESCRIPTORS_LATCH_H_

#include <opencv2/opencv_modules.hpp>
#ifdef HAVE_OPENCV_XFEATURES2D

#include <memory>
#include <string>

#include <arrows/ocv/algo/extract_descriptors.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_LATCH
  : public ocv::extract_descriptors
{
public:
  PLUGGABLE_IMPL(
    extract_descriptors_LATCH,
    "OpenCV feature-point descriptor extraction via the LATCH algorithm",

    PARAM_DEFAULT(
      bytes, int,
      "bytes",
      32 ),

    PARAM_DEFAULT(
      rotation_invariance, bool,
      "rotation_invariance",
      true ),

    PARAM_DEFAULT(
      half_ssd_size, int,
      "half_ssd_size",
      3 ),
  );

  /// Destructor
  virtual ~extract_descriptors_LATCH();

  /// Check that the algorithm's configuration config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_extractor_parameters() const override;
};

#define KWIVER_OCV_HAS_LATCH

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // HAVE_OPENCV_XFEATURES2D

#endif // KWIVER_ARROWS_EXTRACT_DESCRIPTORS_LATCH_H_
