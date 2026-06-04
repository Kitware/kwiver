// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_CONVERT_IMAGE_
#define KWIVER_ARROWS_VXL_CONVERT_IMAGE_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Convert between VXL image formats.
///
/// This can be used, for example, to turn a floating point image into
/// a byte image and vice versa.
class KWIVER_ALGO_VXL_EXPORT convert_image
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    convert_image,
    "Convert image between different formats or scales.",
    PARAM_DEFAULT(
      format, std::string,
      "Output type format: byte, sbyte, float, double, uint16, uint32, etc.",
      "byte" ),
    PARAM_DEFAULT(
      single_channel, bool,
      "Convert input (presumably multi-channel) to contain a single channel, "
      "using either standard RGB to grayscale conversion weights, or "
      "averaging.",
      false ),
    PARAM_DEFAULT(
      scale_factor, double,
      "Optional input value scaling factor",
      0.0 ),
    PARAM_DEFAULT(
      random_grayscale, double,
      "Convert input image to a 3-channel grayscale image randomly with this "
      "percentage between 0.0 and 1.0. This is used for machine learning "
      "augmentation.",
      0.0 ),
    PARAM_DEFAULT(
      percentile_norm, double,
      "If set, between [0, 0.5), perform percentile "
      "normalization such that the output image's min and max "
      "values correspond to the percentiles in the orignal "
      "image at this value and one minus this value, respectively.",
      -1.0 )
  )

  virtual ~convert_image() = default;

  /// Set this algorithm's properties via a config block.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Convert to the right type and optionally transform.
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
