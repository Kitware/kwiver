// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_PIXEL_FEATURE_EXTRACTOR_
#define KWIVER_ARROWS_VXL_PIXEL_FEATURE_EXTRACTOR_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Extract multiple features from an image
class KWIVER_ALGO_VXL_EXPORT pixel_feature_extractor
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    pixel_feature_extractor,
    "Extract various local pixel-wise features from an image.",
    PARAM_DEFAULT(
      enable_color, bool,
      "Enable color channels.",
      true ),
    PARAM_DEFAULT(
      enable_gray, bool,
      "Enable grayscale channel.",
      true ),
    PARAM_DEFAULT(
      enable_aligned_edge, bool,
      "Enable aligned_edge_detection filter.",
      true ),
    PARAM_DEFAULT(
      enable_average, bool,
      "Enable average_frames filter.",
      true ),
    PARAM_DEFAULT(
      enable_color_commonality, bool,
      "Enable color_commonality_filter filter.",
      true ),
    PARAM_DEFAULT(
      enable_high_pass_box, bool,
      "Enable high_pass_filter filter.",
      true ),
    PARAM_DEFAULT(
      enable_high_pass_bidir, bool,
      "Enable high_pass_filter filter.",
      true ),
    PARAM_DEFAULT(
      enable_normalized_variance, bool,
      "Enable the normalized variance since the last shot break. "
      "This will be a scalar multiple with the normal variance until "
      "shot breaks are implemented.",
      true ),
    PARAM_DEFAULT(
      enable_spatial_prior, bool,
      "Enable an image which encodes the location",
      true ),
    PARAM_DEFAULT(
      variance_scale_factor, float,
      "The multiplicative value for the normalized varaince",
      0.32f ),
    PARAM_DEFAULT(
      grid_length, unsigned,
      "The number of grids in each directions of the spatial "
      "prior",
      5 )
  )

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// extract local pixel-wise features
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
