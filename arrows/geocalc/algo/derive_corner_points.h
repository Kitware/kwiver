// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of derive_corner_points algorithm.

#ifndef KWIVER_ARROWS_GEOCALC_DERIVE_CORNER_POINTS_H_
#define KWIVER_ARROWS_GEOCALC_DERIVE_CORNER_POINTS_H_

#include <arrows/geocalc/kwiver_algo_geocalc_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/metadata_filter.h>

namespace kwiver {

namespace arrows {

namespace geocalc {

// ----------------------------------------------------------------------------
class KWIVER_ALGO_GEOCALC_EXPORT derive_corner_points
  : public vital::algo::metadata_filter
{
public:
  PLUGGABLE_IMPL(
    derive_corner_points,
    "Calculates corner points from other metadata when possible.",
    PARAM_DEFAULT(
      overwrite, bool,
      "When set to true, will replace existing corner point metadata.",
      false
    )
  )

  bool check_configuration( vital::config_block_sptr config ) const override;

  vital::metadata_vector filter(
    vital::metadata_vector const& input_metadata,
    vital::image_container_scptr const& input_image ) override;

private:
  void initialize() override;
};

} // namespace geocalc

} // namespace arrows

} // namespace kwiver

#endif
