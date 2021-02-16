// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "hashed_image_classifier.h"
#include "hashed_image_classifier_filter.h"

#include <arrows/vxl/image_container.h>

#include <vital/vital_config.h>

#include <vil/algo/vil_threshold.h>
#include <vil/vil_convert.h>
#include <vil/vil_image_view.h>
#include <vil/vil_math.h>
#include <vil/vil_plane.h>

#include <cstdlib>
#include <limits>
#include <type_traits>

namespace kwiver {

namespace arrows {

namespace vxl {

// ----------------------------------------------------------------------------
/// Private implementation class
class hashed_image_classifier_filter::priv
{
public:

  // --------------------------------------------------------------------------
  priv( hashed_image_classifier_filter* parent ) : p{ parent }
  {
  }

  // Convert the type
  template < typename ipix_t > vil_image_view< ipix_t >
  convert( vil_image_view_base_sptr& view );

  // Scale and convert the image
  bool
  load_model();

  hashed_image_classifier_filter* p;

  vidtk::hashed_image_classifier< vxl_byte, double > hashed_classifier;
  bool model_loaded = false;

  bool use_variable_models = false;

  std::string default_filename = "";
};

// ----------------------------------------------------------------------------
bool
hashed_image_classifier_filter::priv
::load_model()
{
  if( !model_loaded )
  {
    if( !hashed_classifier.load_from_file( default_filename ) )
    {
      LOG_ERROR( p->logger(),
                 "Could not load default_filename model" );
      return false;
    }
    model_loaded = true;
  }
  return true;
}

// ----------------------------------------------------------------------------
hashed_image_classifier_filter
::hashed_image_classifier_filter()
  : d( new priv( this ) )
{
  attach_logger( "arrows.vxl.hashed_image_classifier_filter" );
}

// ----------------------------------------------------------------------------
hashed_image_classifier_filter
::~hashed_image_classifier_filter()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
hashed_image_classifier_filter
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value( "use_variable_models", d->use_variable_models,
                     "Set to true if we should use different models "
                     "for different GSDs and modalities." );
  config->set_value( "default_filename", d->default_filename,
                     "Filename for the default model to use." );

  return config;
}

// ----------------------------------------------------------------------------
void
hashed_image_classifier_filter
::set_configuration( vital::config_block_sptr in_config )
{
  // Start with our generated vital::config_block to ensure that assumed values
  // are present. An alternative would be to check for key presence before
  // performing a get_value() call.

  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d->use_variable_models =
    config->get_value< bool >( "use_variable_models" );
  d->default_filename =
    config->get_value< std::string >( "default_filename" );
}

// ----------------------------------------------------------------------------
bool
hashed_image_classifier_filter
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
hashed_image_classifier_filter
::filter( kwiver::vital::image_container_sptr image_data )
{
  // Perform Basic Validation
  if( !image_data )
  {
    LOG_ERROR( logger(), "Image pointer was null" );
    return kwiver::vital::image_container_sptr();
  }

  // Get input image
  vil_image_view_base_sptr view =
    vxl::image_container::vital_to_vxl( image_data->get_image() );

  if( !view )
  {
    LOG_ERROR( logger(), "Data contained in the image container is null" );
    return nullptr;
  }

  if( view->pixel_format() != VIL_PIXEL_FORMAT_BYTE )
  {
    LOG_ERROR( logger(), "Only byte images can be proccessed" );
    return nullptr;
  }

  if( !d->load_model() )
  {
    return nullptr;
  }

  vil_image_view< double > weight_image;

  d->hashed_classifier.classify_images( view, weight_image, 0.0 );

  return std::make_shared< vxl::image_container >( weight_image );
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
