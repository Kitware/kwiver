// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_HIGH_PASS_FILTER_
#define KWIVER_ARROWS_VXL_HIGH_PASS_FILTER_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

#include <vital/util/enum_converter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

enum filter_mode
{
  MODE_box,
  MODE_bidir,
};

/// VXL High Pass Filtering Process
///
/// This method contains basic methods for high pass image filtering
/// on top of input images
class KWIVER_ALGO_VXL_EXPORT high_pass_filter
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    high_pass_filter,
    "Use VXL to create an image based on high-frequency information.",
    PARAM_DEFAULT(
      mode, std::string,
      "Operating mode of this filter, possible values: " +
      mode_converter().element_name_string(),
      mode_converter().to_string( MODE_box ) ),
    PARAM_DEFAULT(
      kernel_width, unsigned,
      "Pixel width of smoothing kernel",
      7 ),
    PARAM_DEFAULT(
      kernel_height, unsigned,
      "Pixel height of smoothing kernel",
      7 ),
    PARAM_DEFAULT(
      treat_as_interlaced, bool,
      "Process alternating rows independently",
      false ),
    PARAM_DEFAULT(
      output_net_only, bool,
      "If set to false, the output image will contain multiple "
      "planes, each representing the modal filter applied at "
      "different orientations, as opposed to a single plane "
      "image representing the sum of filters applied in all "
      "directions.",
      false )
  )

  virtual ~high_pass_filter() = default;

  /// Check that the algorithm's currently configuration is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Perform high pass filtering
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

  ENUM_CONVERTER(
    mode_converter, filter_mode,
    { "box", MODE_box }, { "bidir", MODE_bidir } )

private:
  void initialize() override;
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
