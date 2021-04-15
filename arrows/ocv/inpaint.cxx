// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "inpaint.h"

#include <arrows/ocv/image_container.h>

#include <vital/util/enum_converter.h>
#include <vital/vital_config.h>

#include <opencv2/core/core.hpp>
#include <opencv2/photo.hpp>

namespace kwiver {

namespace arrows {

namespace ocv {

enum inpainting_method
{
  METHOD_mask,
  METHOD_telea,
  METHOD_navier_stokes,
};

ENUM_CONVERTER( method_converter, inpainting_method,
                { "mask", METHOD_mask },
                { "telea", METHOD_telea },
                { "navier_stokes", METHOD_navier_stokes } )

// ----------------------------------------------------------------------------
/// Private implementation class
class inpaint::priv
{
public:
  priv( inpaint* parent )
    : p{ parent }
  {
  }

  inpaint* p;
  // Internal parameters/settings
  inpainting_method method{ METHOD_telea };
  float radius{ 3.0 };
};

// ----------------------------------------------------------------------------
inpaint
::inpaint()
  : d{ new priv{ this } }
{
  attach_logger( "arrows.ocv.inpaint" );
}

// ----------------------------------------------------------------------------
inpaint::~inpaint()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
inpaint
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value( "inpaint_method",
                     method_converter().to_string( d->method ),
                     "Inpainting method, possible values: " +
                     method_converter().element_name_string() );
  config->set_value( "radius", d->radius,
                     "Radius parameter for the inpainting method" );

  return config;
}

// ----------------------------------------------------------------------------
void
inpaint
::set_configuration( vital::config_block_sptr in_config )
{
  // Starting with our generated vital::config_block to ensure that assumed
  // values are present. An alternative is to check for key presence before
  // performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  // Settings for filtering
  d->method = config->get_enum_value< method_converter >( "inpaint_method",
                                                          d->method );
  d->radius = config->get_value< float >( "radius", d->radius );
}

// ----------------------------------------------------------------------------
bool
inpaint
::check_configuration( vital::config_block_sptr config ) const
{
  auto const radius = config->get_value< float >( "radius" );
  if( radius <= 0 )
  {
    LOG_ERROR( logger(),
               "Radius should be positive but instead was " << radius );
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
inpaint
::filter( kwiver::vital::image_container_sptr image )
{
  cv::Mat cv_image = ocv::image_container::vital_to_ocv(
    image->get_image(), ocv::image_container::RGB_COLOR );

  if( cv_image.channels() != 4 )
  {
    LOG_ERROR(
      logger(),
      "Expected 4 image channels but instead there were " <<
        cv_image.channels() );
    return image;
  }
  cv::Mat rgba_channels[ 4 ];
  cv::split( cv_image, rgba_channels );
  cv::merge( rgba_channels, 3, cv_image ); // merge the first three channels
  switch( d->method )
  {
    case METHOD_telea:
    {
      cv::inpaint( cv_image, rgba_channels[ 3 ], cv_image, d->radius,
                   cv::INPAINT_TELEA );
      break;
    }
    case METHOD_navier_stokes:
    {
      cv::inpaint( cv_image, rgba_channels[ 3 ], cv_image, d->radius,
                   cv::INPAINT_NS );
      break;
    }
    case METHOD_mask:
    {
      cv::Mat zeros = cv::Mat::zeros( cv_image.size(), cv_image.type() );
      zeros.copyTo( cv_image, rgba_channels[ 3 ] );
      break;
    }
    default:
    {
      LOG_ERROR( logger(), "Method not supported" );
    }
  }
  return std::make_shared< ocv::image_container >(
    cv_image, ocv::image_container::RGB_COLOR );
}

} // namespace ocv

} // namespace arrows

} // namespace kwiver
