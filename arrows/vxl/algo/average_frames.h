// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_AVERAGE_FRAMES_
#define KWIVER_ARROWS_VXL_AVERAGE_FRAMES_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

#include <vital/util/enum_converter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

enum averager_mode
{
  AVERAGER_cumulative,
  AVERAGER_window,
  AVERAGER_exponential,
};

/// VXL Frame Averaging Process.
///
/// This method contains basic methods for image filtering on top of input
/// images via performing assorted averaging operations.
class KWIVER_ALGO_VXL_EXPORT average_frames
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    average_frames,
    "Use VXL to average frames together.",
    PARAM_DEFAULT(
      type, std::string,
      "Operating mode of this filter, possible values: " +
      averager_converter().element_name_string(),
      averager_converter().to_string( AVERAGER_window ) ),
    PARAM_DEFAULT(
      window_size, unsigned,
      "The window size if computing a windowed moving average.",
      10 ),
    PARAM_DEFAULT(
      exp_weight, double,
      "Exponential averaging coefficient if computing an exp average.",
      0.3 ),
    PARAM_DEFAULT(
      round, bool,
      "Should we spend a little extra time rounding when possible?",
      false ),
    PARAM_DEFAULT(
      output_variance, bool,
      "If set, will compute an estimated variance for each pixel which "
      "will be outputted as either a double-precision or byte image.",
      false )
  )

  virtual ~average_frames() = default;

  /// Check that the algorithm's currently configuration is valid.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Average frames temporally.
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

  ENUM_CONVERTER(
    averager_converter, averager_mode,
    { "cumulative", AVERAGER_cumulative },
    { "window", AVERAGER_window },
    { "exponential", AVERAGER_exponential } );

private:
  void initialize() override;
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
