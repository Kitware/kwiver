// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_COLOR_COMMONALITY_FILTER_
#define KWIVER_ARROWS_VXL_COLOR_COMMONALITY_FILTER_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// VXL Color Commonality Filter.
///
/// This method produces an output image where each pixel corresponds
/// to how frequent the pixel's color is in the entire image.
class KWIVER_ALGO_VXL_EXPORT color_commonality_filter
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    color_commonality_filter,
    "Filter image based on color frequency or commonality.",
    PARAM_DEFAULT(
      color_resolution_per_channel, unsigned,
      "Resolution of the utilized histogram (per channel) if the input "
      "contains 3 channels. Must be a power of two.",
      8 ),
    PARAM_DEFAULT(
      intensity_resolution, unsigned,
      "Resolution of the utilized histogram if the input "
      "contains 1 channel. Must be a power of two.",
      16 ),
    PARAM_DEFAULT(
      output_scale, unsigned,
      "Scale the output image (typically, values start in the range [0,1]) "
      "by this amount. Enter 0 for type-specific default.",
      0 ),
    PARAM_DEFAULT(
      grid_image, bool,
      "Instead of calculating which colors are more common "
      "in the entire image, should we do it for smaller evenly "
      "spaced regions?",
      false ),
    PARAM_DEFAULT(
      grid_resolution_height, unsigned,
      "Divide the height of the image into x regions, if enabled.",
      5 ),
    PARAM_DEFAULT(
      grid_resolution_width, unsigned,
      "Divide the width of the image into x regions, if enabled.",
      6 )
  )

  virtual ~color_commonality_filter() = default;

  /// Check that the algorithm's currently configuration is valid.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Perform pixel frequency computation for one frame.
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
