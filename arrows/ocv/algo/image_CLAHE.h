// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV CLAHE image filter wrapper

#ifndef KWIVER_ARROWS_OCV_IMAGE_CLAHE_H_
#define KWIVER_ARROWS_OCV_IMAGE_CLAHE_H_

#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <vital/algo/image_filter.h>

#include <opencv2/imgproc.hpp>

namespace kwiver {

namespace arrows {

namespace ocv {

/// Filter images using OpenCV CLAHE contrast adjustment.
///
/// This can be used to adaptively enhance image contrast.
class KWIVER_ALGO_OCV_EXPORT image_CLAHE
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    image_CLAHE,
    "Adaptively equalize image contrast using OpenCV CLAHE.",
    // Defaults from Eugene's track_feature.conf
    PARAM_DEFAULT(
      clip_limit, double,
      "Threshold for contrast limiting. Must be positive.",
      40.0 ),

    PARAM_DEFAULT(
      tile_grid_width, int,
      "Number of tiles in the horizontal direction. Must be positive.",
      8 ),

    PARAM_DEFAULT(
      tile_grid_height, int,
      "Number of tiles in the vertical direction. Must be positive.",
      8 )
  );

  virtual ~image_CLAHE();

  bool check_configuration( vital::config_block_sptr config ) const override;

  vital::image_container_sptr filter(
    vital::image_container_sptr img ) override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;

  cv::Ptr< cv::CLAHE > clahe_;
};

} // namespace ocv

} // namespace arrows

} // namespace kwiver

#endif
