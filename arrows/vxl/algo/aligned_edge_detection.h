// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_VXL_ALIGNED_EDGE_DETECTION_
#define KWIVER_ARROWS_VXL_ALIGNED_EDGE_DETECTION_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// Extract axis-aligned edges.
class KWIVER_ALGO_VXL_EXPORT aligned_edge_detection
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    aligned_edge_detection,
    "Compute axis-aligned edges in an image.",
    PARAM_DEFAULT(
      threshold, float,
      "Minimum edge magnitude required to report as an edge "
      "in any output image.",
      10.0 ),
    PARAM_DEFAULT(
      produce_joint_output, bool,
      "Set to false if we do not want to spend time computing "
      "joint edge images comprised of both horizontal and "
      "vertical information.",
      true ),
    PARAM_DEFAULT(
      smoothing_sigma, double,
      "Smoothing sigma for the output NMS edge density map.",
      1.3 ),
    PARAM_DEFAULT(
      smoothing_half_step, unsigned,
      "Smoothing half step for the output NMS edge density map.",
      2 )
  )

  virtual ~aligned_edge_detection() = default;

  /// Check that the algorithm's current configuration is valid.
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Convert to the right type and optionally transform.
  virtual kwiver::vital::image_container_sptr filter(
    kwiver::vital::image_container_sptr image_data );

private:
  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace vxl

} // namespace arrows

} // namespace kwiver

#endif
