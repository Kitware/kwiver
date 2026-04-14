// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of OCV CLAHE image filter

#include "image_CLAHE.h"

#include <arrows/ocv/image_container.h>

namespace kwiver {

namespace arrows {

namespace ocv {

// ----------------------------------------------------------------------------
void
image_CLAHE
::initialize()
{
  attach_logger( "arrows.ocv.image_CLAHE" );
  clahe_ = cv::createCLAHE();
}

// ----------------------------------------------------------------------------
image_CLAHE::~image_CLAHE()
{}

// ----------------------------------------------------------------------------
void
image_CLAHE
::set_configuration_internal( vital::config_block_sptr )
{
  clahe_->setClipLimit( this->get_clip_limit() );
  clahe_->setTilesGridSize(
    cv::Size( this->get_tile_grid_width(), this->get_tile_grid_height() ) );
}

// ----------------------------------------------------------------------------
bool
image_CLAHE
::check_configuration( vital::config_block_sptr config ) const
{
  bool valid = true;

  auto const clip_limit = config->get_value< double >( "clip_limit" );
  if( clip_limit <= 0 )
  {
    LOG_ERROR(
      logger(),
      "clip_limit=" << clip_limit << ", but must be positive." );
    valid = false;
  }

  auto const w = config->get_value< int >( "tile_grid_width" );
  if( w <= 0 )
  {
    LOG_ERROR(
      logger(),
      "tile_grid_width=" << w << ", but must be positive." );
    valid = false;
  }

  auto const h = config->get_value< int >( "tile_grid_height" );
  if( h <= 0 )
  {
    LOG_ERROR(
      logger(),
      "tile_grid_height=" << h << ", but must be positive." );
    valid = false;
  }

  return valid;
}

// ----------------------------------------------------------------------------
vital::image_container_sptr
image_CLAHE
::filter( vital::image_container_sptr img )
{
  cv::Mat src = image_container::vital_to_ocv(
    img->get_image(),
    image_container::OTHER_COLOR );
  cv::Mat dst;
  clahe_->apply( src, dst );
  return std::make_shared< image_container >(
    image_container( dst, image_container::OTHER_COLOR ) );
}

} // namespace ocv

} // namespace arrows

} // namespace kwiver
