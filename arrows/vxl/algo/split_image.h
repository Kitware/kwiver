// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for VXL split_image algorithm

#ifndef KWIVER_ARROWS_VXL_SPLIT_IMAGE_H_
#define KWIVER_ARROWS_VXL_SPLIT_IMAGE_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/split_image.h>

#include <vital/vital_config.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// A class for drawing various information about feature tracks
class KWIVER_ALGO_VXL_EXPORT split_image
  : public vital::algo::split_image
{
public:
  PLUGGABLE_IMPL(
    split_image,
    "Split a larger image into multiple smaller images" )

  /// Destructor
  virtual ~split_image() = default;

  bool
  check_configuration(
    VITAL_UNUSED kwiver::vital::config_block_sptr config ) const override
  {
    return true;
  }

  /// Split image
  virtual std::vector< kwiver::vital::image_container_sptr >
  split( kwiver::vital::image_container_sptr img ) const;

private:
  void initialize() override;
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif
