// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of OCV refine detections watershed algorithm

#include "refine_detections_watershed.h"

#include <algorithm>

#include <vital/vital_config.h>
#include <vital/exceptions/io.h>

#include <arrows/ocv/image_container.h>
#include <arrows/ocv/refine_detections_util.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv {

// ----------------------------------------------------------------------------
// Private implementation class
class refine_detections_watershed::priv
{
public:
  priv()
  {
  }

  bool seed_with_existing_masks = true;
  double seed_scale_factor = 0.2;
};

// ----------------------------------------------------------------------------
// Constructor
refine_detections_watershed
::refine_detections_watershed()
: d_( new priv() )
{
}


// Destructor
refine_detections_watershed
::~refine_detections_watershed()
{
}

// ----------------------------------------------------------------------------
// Get this algorithm's vital::config_block configuration block
vital::config_block_sptr
refine_detections_watershed
::get_configuration() const
{
  vital::config_block_sptr config = vital::algo::refine_detections::get_configuration();
  config->set_value( "seed_scale_factor", d_->seed_scale_factor,
                     "Amount to scale the detection by to produce "
                     "a high-confidence seed region" );
  config->set_value( "seed_with_existing_masks", d_->seed_with_existing_masks,
                     "If true, use existing masks as seed regions" );
  return config;
}

// ----------------------------------------------------------------------------
// Set this algorithm's properties via a config block
void
refine_detections_watershed
::set_configuration( vital::config_block_sptr in_config )
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d_->seed_with_existing_masks =
    config->get_value< bool >( "seed_with_existing_masks" );
  d_->seed_scale_factor =
    config->get_value< double >( "seed_scale_factor" );
}

// ----------------------------------------------------------------------------
// Check that the algorithm's current configuration is valid
bool
refine_detections_watershed
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}

// ----------------------------------------------------------------------------
// Set detection segmentation masks using cv::watershed
vital::detected_object_set_sptr
refine_detections_watershed
::refine( vital::image_container_sptr image_data,
          vital::detected_object_set_sptr detections ) const
{
  using ic = ocv::image_container;

  if( !detections || !image_data )
  {
    return detections;
  }

  cv::Mat img = ic::vital_to_ocv( image_data->get_image(), ic::BGR_COLOR );
  cv::Rect img_rect( 0, 0, img.cols, img.rows );

  bounding_box< double > vital_img_rect( 0, 0, img.cols, img.rows );
  cv::Mat background( img.size(), CV_8UC1, 255 );
  // Explicitly convert 0 to a Scalar to avoid interpretation as NULL
  cv::Mat markers( img.size(), CV_32SC1, cv::Scalar( 0 ) );

  std::vector< cv::Mat > seeds;
  auto result = std::make_shared< vital::detected_object_set >();
  auto valid_detections = std::make_shared< vital::detected_object_set >();
  size_t i;
  for( i = 0; i < detections->size(); ++i )
  {
    auto&& det = detections->at( i );
    auto&& bbox = intersection( det->bounding_box(), vital_img_rect );
    auto rect = bbox_to_mask_rect( bbox );
    // Invalid rectangle, simply return the unmodified input
    if( rect.empty() )
    {
      result->add( std::move( det ) );
    }
    else
    {
      background( rect & img_rect ) = 0;
      cv::Mat m = markers( rect & img_rect );
      cv::Mat already_set = m != 0;
      cv::Mat seed;
      if( d_->seed_with_existing_masks && det->mask() )
      {
        // Clone because this is modified below (crop_mask.setTo)
        seed = get_standard_mask( det ).clone();
      }
      else
      {
        auto seed_bbox = vital::scale_about_center( bbox, d_->seed_scale_factor );
        seed = cv::Mat( rect.size(), CV_8UC1, cv::Scalar( 0 ) );
        seed( ( bbox_to_mask_rect( seed_bbox ) & rect ) - rect.tl() ) = 1;
      }
      m.setTo( i + 1, seed );
      m.setTo( -1, seed & already_set );
      seeds.push_back( std::move( seed ) );
      valid_detections->add( std::move( det ) );
    }
  }
  markers = cv::max( markers, 0 );
  markers.setTo( i + 1, background );
  cv::watershed( img, markers );

  for( i = 0; i < valid_detections->size(); ++i )
  {
    auto&& det = valid_detections->at( i );
    auto&& bbox = intersection( det->bounding_box(), vital_img_rect );
    auto rect = bbox_to_mask_rect( bbox );
    auto crop_rect = rect & img_rect;
    auto& mask = seeds[ i ];
    cv::Mat crop_mask = mask( crop_rect - rect.tl() );
    crop_mask.setTo( 1, markers( crop_rect ) == i + 1 );
    // Add detection with mask to the output
    auto new_det = det->clone();
    // mask is a single-channel image, so the ic::ColorMode argument
    // should be irrelevant
    new_det->set_mask( std::make_shared< ic >( mask, ic::OTHER_COLOR ) );
    result->add( std::move( new_det ) );
  }
  return result;
}

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
