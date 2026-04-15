// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_THRESHOLD_
#define KWIVER_THRESHOLD_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

#include <vital/util/enum_converter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

enum threshold_mode
{
  MODE_absolute,
  MODE_percentile,
};

/// Threshold an image using different schemes.
///
/// Use either an absolute threshold or one based on percentiles.
class KWIVER_ALGO_VXL_EXPORT threshold
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    threshold,
    "Threshold at image at a given percentile or value.",
    PARAM_DEFAULT(
      threshold, double,
      "Threshold to use. Meaning is dependent on type.",
      0.95 ),
    PARAM_DEFAULT(
      type, std::string,
      "Type of thresholding to use. Possible options are: " +
      mode_converter().element_name_string(),
      mode_converter().to_string( MODE_percentile ) )
  )

  virtual ~threshold() = default;

  /// Check that the algorithm's currently configuration is valid.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Binarize the image at a given percentile threshold.
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

  ENUM_CONVERTER(
    mode_converter, threshold_mode, { "absolute", MODE_absolute },
    { "percentile", MODE_percentile } );

private:
  void initialize() override;
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
