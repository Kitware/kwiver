// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_MORPHOLOGY_
#define KWIVER_ARROWS_VXL_MORPHOLOGY_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

#include <vital/util/enum_converter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

enum morphology_mode
{
  MORPHOLOGY_erode,
  MORPHOLOGY_dilate,
  MORPHOLOGY_open,
  MORPHOLOGY_close,
  MORPHOLOGY_none,
};

enum element_mode
{
  ELEMENT_disk,
  ELEMENT_jline,
  ELEMENT_iline,
};

enum combine_mode
{
  COMBINE_none,
  COMBINE_union,
  COMBINE_intersection,
};

/// Convert between VXL image formats.
///
/// This can be used, for example, to turn a floating point image into
/// a byte image and vice versa.
class KWIVER_ALGO_VXL_EXPORT morphology
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    morphology,
    "Apply channel-wise morphological operations and "
    "optionally merge across channels.",
    PARAM_DEFAULT(
      morphology, std::string,
      "Morphological operation to apply. Possible options are: " +
      morphology_converter().element_name_string(),
      morphology_converter().to_string( MORPHOLOGY_dilate ) ),
    PARAM_DEFAULT(
      element_shape, std::string,
      "Shape of the structuring element. Possible options are: " +
      element_converter().element_name_string(),
      element_converter().to_string( ELEMENT_disk ) ),
    PARAM_DEFAULT(
      channel_combination, std::string,
      "Method for combining multiple binary channels. Possible options are: " +
      combine_converter().element_name_string(),
      combine_converter().to_string( COMBINE_none ) ),
    PARAM_DEFAULT(
      kernel_radius, double,
      "Radius of morphological kernel.",
      1.5 )
  )

  virtual ~morphology() = default;

  /// Check that the algorithm's currently configuration is valid.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Convert to the right type and optionally transform.
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

  ENUM_CONVERTER(
    morphology_converter, morphology_mode,
    { "erode", MORPHOLOGY_erode }, { "dilate", MORPHOLOGY_dilate },
    { "open", MORPHOLOGY_open }, { "close", MORPHOLOGY_close },
    { "none", MORPHOLOGY_none } );

  ENUM_CONVERTER(
    element_converter, element_mode, { "disk", ELEMENT_disk },
    { "iline", ELEMENT_iline }, { "jline", ELEMENT_jline } );

  ENUM_CONVERTER(
    combine_converter, combine_mode, { "none", COMBINE_none },
    { "union", COMBINE_union },
    { "intersection", COMBINE_intersection } );

private:
  void initialize() override;
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
