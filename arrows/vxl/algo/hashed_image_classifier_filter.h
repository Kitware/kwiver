// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_HASHED_IMAGE_CLASSIFIER_FILTER_
#define KWIVER_ARROWS_VXL_HASHED_IMAGE_CLASSIFIER_FILTER_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Classify an image of features using a sum of linear classifiers.
class KWIVER_ALGO_VXL_EXPORT hashed_image_classifier_filter
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    hashed_image_classifier_filter,
    "Perform per-pixel classification on an image of features.",
    PARAM_DEFAULT(
      model_file, std::string,
      "Model file from which to load weights.",
      "" ),
    PARAM_DEFAULT(
      offset, double,
      "Value to initialize the response map with.",
      0.0 )
  )

  virtual ~hashed_image_classifier_filter() = default;

  /// Check that the algorithm's currently configuration is valid.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Perform per-pixel classification.
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

private:
  void initialize() override;
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
