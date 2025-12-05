/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "windowed_refiner.h"
#include "windowed_utils.h"

#include <vital/util/wall_timer.h>
#include <vital/exceptions/io.h>
#include <vital/config/config_block_formatter.h>

#include <arrows/ocv/image_container.h>
#include <kwiversys/SystemTools.hxx>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <algorithm>
#include <string>
#include <sstream>
#include <exception>
#include <limits>

namespace kwiver {
namespace arrows {
namespace ocv {

// =============================================================================
class windowed_refiner::priv
{
public:
  priv()
  {}

  ~priv() {}

  // Settings from the config
  window_settings m_settings;

  // Helper functions
  vital::detected_object_set_sptr scale_detections(
    const vital::detected_object_set_sptr detections,
    const windowed_region_prop& roi );

  vital::algo::refine_detections_sptr m_refiner;
  vital::logger_handle_t m_logger;
};


// =============================================================================
windowed_refiner
::windowed_refiner()
  : d( new priv() )
{
  attach_logger( "arrows.ocv.windowed_refiner" );

  d->m_logger = logger();
}


windowed_refiner
::~windowed_refiner()
{}


// -----------------------------------------------------------------------------
vital::config_block_sptr
windowed_refiner
::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  // Merge window settings configuration
  config->merge_config( d->m_settings.config() );

  vital::algo::refine_detections::get_nested_algo_configuration(
    "refiner", config, d->m_refiner );

  return config;
}


// -----------------------------------------------------------------------------
void
windowed_refiner
::set_configuration( vital::config_block_sptr config_in )
{
  // Starting with our generated config_block to ensure that assumed values
  // are present. An alternative is to check for key presence before performing
  // a get_value() call.
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );

  // Set window settings from configuration
  d->m_settings.set_config( config );

  vital::algo::refine_detections::set_nested_algo_configuration(
    "refiner", config, d->m_refiner );
}


// -----------------------------------------------------------------------------
bool
windowed_refiner
::check_configuration( vital::config_block_sptr config ) const
{
  return vital::algo::refine_detections::check_nested_algo_configuration(
    "refiner", config );
}


// -----------------------------------------------------------------------------
vital::detected_object_set_sptr
windowed_refiner
::refine( vital::image_container_sptr image_data,
          vital::detected_object_set_sptr detections ) const
{
  vital::scoped_wall_timer t( "Time to Refine Objects" );

  if( !image_data )
  {
    LOG_WARN( d->m_logger, "Input image is empty." );
    return std::make_shared< vital::detected_object_set >();
  }

  cv::Mat cv_image = arrows::ocv::image_container::vital_to_ocv(
    image_data->get_image(), arrows::ocv::image_container::RGB_COLOR );

  if( cv_image.rows == 0 || cv_image.cols == 0 )
  {
    LOG_WARN( d->m_logger, "Input image is empty." );
    return std::make_shared< vital::detected_object_set >();
  }

  // Prepare image regions using utility function
  std::vector< cv::Mat > regions_to_process;
  std::vector< windowed_region_prop > region_properties;

  prepare_image_regions( cv_image, d->m_settings, regions_to_process, region_properties );

  // Run refiner
  vital::detected_object_set_sptr refined_detections = std::make_shared< vital::detected_object_set >();

  // Process all regions
  for( unsigned i = 0; i < regions_to_process.size(); i++ )
  {
    // Scale input detections to this region
    vital::detected_object_set_sptr region_detections =
      scale_detections_to_region( detections, region_properties[i] );

    // Skip empty regions if there are no detections
    if( !region_detections || region_detections->empty() )
    {
      continue;
    }

    vital::detected_object_set_sptr detections_to_refine = region_detections;
    vital::detected_object_set_sptr boundary_dets;

    // Optionally separate boundary detections to pass through unmodified
    if( d->m_settings.preserve_boundary_detections )
    {
      vital::detected_object_set_sptr interior_dets;
      separate_boundary_detections( region_detections,
        regions_to_process[i].cols, regions_to_process[i].rows,
        boundary_dets, interior_dets );
      detections_to_refine = interior_dets;

      // If boundary detections exist, scale them back and add to output
      if( boundary_dets && !boundary_dets->empty() )
      {
        refined_detections->add( d->scale_detections( boundary_dets,
          region_properties[i] ) );
      }

      // Skip refinement if no interior detections
      if( !interior_dets || interior_dets->empty() )
      {
        continue;
      }
    }

    // Convert region to image container
    vital::image_container_sptr region_image(
      new ocv::image_container( regions_to_process[i],
        ocv::image_container::RGB_COLOR ) );

    // Refine detections in this region
    vital::detected_object_set_sptr region_refined =
      d->m_refiner->refine( region_image, detections_to_refine );

    // Scale refined detections back to original image space
    if( region_refined && !region_refined->empty() )
    {
      refined_detections->add( d->scale_detections( region_refined,
        region_properties[i] ) );
    }
  }

  const int min_dim = d->m_settings.min_detection_dim;

  refined_detections->filter([&min_dim](kwiver::vital::detected_object_sptr dos)
  {
    return !dos || dos->bounding_box().width() < min_dim
                || dos->bounding_box().height() < min_dim;
  });

  return refined_detections;
} // windowed_refiner::refine


vital::detected_object_set_sptr
windowed_refiner::priv
::scale_detections(
  const vital::detected_object_set_sptr dets,
  const windowed_region_prop& info )
{
  return ocv::scale_detections( dets, info, m_settings.chip_edge_max_prob );
}


} } } // end namespace
