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

#include "windowed_utils.h"

#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <exception>

namespace kwiver {
namespace arrows {
namespace ocv {

// =============================================================================
window_settings
::window_settings()
  : mode( DISABLED )
  , scale( 1.0 )
  , chip_width( 1000 )
  , chip_height( 1000 )
  , chip_step_width( 500 )
  , chip_step_height( 500 )
  , chip_edge_filter( -1 )
  , chip_edge_max_prob( -1.0 )
  , chip_adaptive_thresh( 2000000 )
  , batch_size( 1 )
  , min_detection_dim( 2 )
  , original_to_chip_size( false )
  , black_pad( false )
  , preserve_boundary_detections( false )
{}

// -----------------------------------------------------------------------------
vital::config_block_sptr
window_settings
::config() const
{
  vital::config_block_sptr config = vital::config_block::empty_config();

  rescale_option_converter conv;
  config->set_value( "mode", conv.to_string( mode ),
    "Pre-processing resize option, can be: disabled, maintain_ar, scale, "
    "chip, chip_and_original, original_and_resized, or adaptive." );
  config->set_value( "scale", scale,
    "Image scaling factor used when mode is scale or chip." );
  config->set_value( "chip_height", chip_height,
    "When in chip mode, the chip height." );
  config->set_value( "chip_width", chip_width,
    "When in chip mode, the chip width." );
  config->set_value( "chip_step_height", chip_step_height,
    "When in chip mode, the chip step size between chips." );
  config->set_value( "chip_step_width", chip_step_width,
    "When in chip mode, the chip step size between chips." );
  config->set_value( "chip_edge_filter", chip_edge_filter,
    "If using chipping, filter out detections this pixel count near borders." );
  config->set_value( "chip_edge_max_prob", chip_edge_max_prob,
    "If using chipping, maximum type probability for edge detections" );
  config->set_value( "chip_adaptive_thresh", chip_adaptive_thresh,
    "If using adaptive selection, total pixel count at which we start to chip." );
  config->set_value( "batch_size", batch_size,
    "Optional processing batch size to send to the detector." );
  config->set_value( "min_detection_dim", min_detection_dim,
    "Minimum detection dimension in original image space." );
  config->set_value( "original_to_chip_size", original_to_chip_size,
    "Optionally enforce the input image is the specified chip size" );
  config->set_value( "black_pad", black_pad,
    "Black pad the edges of resized chips to ensure consistent dimensions" );
  config->set_value( "preserve_boundary_detections", preserve_boundary_detections,
    "Pass through detections touching tile boundaries unmodified in refiner" );

  return config;
}

// -----------------------------------------------------------------------------
void
window_settings
::set_config( vital::config_block_sptr config )
{
  rescale_option_converter conv;
  mode = conv.from_string( config->get_value< std::string >( "mode" ) );
  scale = config->get_value< double >( "scale" );
  chip_width = config->get_value< int >( "chip_width" );
  chip_height = config->get_value< int >( "chip_height" );
  chip_step_width = config->get_value< int >( "chip_step_width" );
  chip_step_height = config->get_value< int >( "chip_step_height" );
  chip_edge_filter = config->get_value< int >( "chip_edge_filter" );
  chip_edge_max_prob = config->get_value< double >( "chip_edge_max_prob" );
  chip_adaptive_thresh = config->get_value< int >( "chip_adaptive_thresh" );
  batch_size = config->get_value< int >( "batch_size" );
  min_detection_dim = config->get_value< int >( "min_detection_dim" );
  original_to_chip_size = config->get_value< bool >( "original_to_chip_size" );
  black_pad = config->get_value< bool >( "black_pad" );
  preserve_boundary_detections = config->get_value< bool >( "preserve_boundary_detections" );
}

// =============================================================================


double
scale_image_maintaining_ar( const cv::Mat& src, cv::Mat& dst,
                            int width, int height, bool pad )
{
  double scale = 1.0;

  if( src.rows == height && src.cols == width )
  {
    dst = src;
    return scale;
  }

  double original_height = static_cast< double >( src.rows );
  double original_width = static_cast< double >( src.cols );

  if( original_height > height )
  {
    scale = height / original_height;
  }
  if( original_width > width )
  {
    scale = std::min( scale, width / original_width );
  }

  cv::Mat resized;
  cv::resize( src, resized, cv::Size(), scale, scale );

  if( pad )
  {
    dst.create( height, width, src.type() );
    dst.setTo( 0 );

    cv::Rect roi( 0, 0, resized.cols, resized.rows );
    cv::Mat aoi( dst, roi );

    resized.copyTo( aoi );
  }
  else
  {
    dst = resized;
  }

  return scale;
}

double
format_image( const cv::Mat& src, cv::Mat& dst, rescale_option option,
              double scale_factor, int width, int height, bool pad )
{
  double scale = 1.0;

  if( option == MAINTAIN_AR )
  {
    scale = scale_image_maintaining_ar( src, dst, width, height, pad );
  }
  else if( option == CHIP || option == SCALE ||
           option == CHIP_AND_ORIGINAL )
  {
    if( scale_factor == 1.0 )
    {
      dst = src;
    }
    else
    {
      cv::resize( src, dst, cv::Size(), scale_factor, scale_factor );
      scale = scale_factor;
    }
  }
  else
  {
    rescale_option_converter conv;
    throw std::runtime_error( "Invalid resize option: " + conv.to_string( option ) );
  }

  return scale;
}

// -----------------------------------------------------------------------------
vital::detected_object_set_sptr
scale_detections(
  const vital::detected_object_set_sptr detections,
  const windowed_region_prop& region_info,
  double chip_edge_max_prob )
{
  // Apply first scale transformation
  if( region_info.scale1 != 1.0 )
  {
    detections->scale( region_info.scale1 );
  }

  // Apply shift transformation
  if( region_info.shiftx != 0 || region_info.shifty != 0 )
  {
    detections->shift( region_info.shiftx, region_info.shifty );
  }

  // Apply second scale transformation
  if( region_info.scale2 != 1.0 )
  {
    detections->scale( region_info.scale2 );
  }

  const int dist = region_info.edge_filter;

  // If no edge filtering, return detections as-is
  if( dist < 0 )
  {
    return detections;
  }

  // Filter detections near edges
  const cv::Rect& roi = region_info.original_roi;
  std::vector< vital::detected_object_sptr > filtered_dets;

  for( auto det : *detections )
  {
    if( !det )
    {
      continue;
    }

    // Check if detection is near an edge
    if( ( roi.x > 0 && det->bounding_box().min_x() < roi.x + dist ) ||
        ( roi.y > 0 && det->bounding_box().min_y() < roi.y + dist ) ||
        ( !region_info.right_border && det->bounding_box().max_x() > roi.x + roi.width - dist ) ||
        ( !region_info.bottom_border && det->bounding_box().max_y() > roi.y + roi.height - dist ) )
    {
      // If chip_edge_max_prob is disabled (<=0), skip edge detections
      if( chip_edge_max_prob <= 0.0 )
      {
        continue;
      }

      // Clamp detection confidence to chip_edge_max_prob
      if( det->confidence() > chip_edge_max_prob )
      {
        det->set_confidence( chip_edge_max_prob );
      }

      // Clamp detection type scores to chip_edge_max_prob
      if( det->type() )
      {
        auto dot = det->type();
        std::string top_class;
        dot->get_most_likely( top_class );
        double score = dot->score( top_class );

        if( score > chip_edge_max_prob )
        {
          double scale = chip_edge_max_prob / score;

          for( auto name : dot->class_names() )
          {
            dot->set_score( name, dot->score( name ) * scale );
          }
        }
      }
    }

    filtered_dets.push_back( det );
  }

  return vital::detected_object_set_sptr(
    new vital::detected_object_set( filtered_dets ) );
}

// -----------------------------------------------------------------------------
void
prepare_image_regions(
  const cv::Mat& image,
  const window_settings& settings,
  std::vector< cv::Mat >& regions_to_process,
  std::vector< windowed_region_prop >& region_properties )
{
  // Clear output vectors
  regions_to_process.clear();
  region_properties.clear();

  // Determine the processing mode
  rescale_option mode = settings.mode;

  if( mode == ADAPTIVE )
  {
    if( ( image.rows * image.cols ) >= settings.chip_adaptive_thresh )
    {
      mode = CHIP_AND_ORIGINAL;
    }
    else if( settings.original_to_chip_size )
    {
      mode = MAINTAIN_AR;
    }
    else
    {
      mode = DISABLED;
    }
  }

  // Resize image if enabled
  cv::Mat resized_image;
  double scale_factor = 1.0;

  if( mode != DISABLED )
  {
    scale_factor = format_image( image, resized_image,
      ( mode == ORIGINAL_AND_RESIZED ? SCALE : mode ),
      settings.scale, settings.chip_width, settings.chip_height );
  }
  else
  {
    resized_image = image;
  }

  cv::Rect original_dims( 0, 0, image.cols, image.rows );

  // Create regions based on mode
  if( mode == ORIGINAL_AND_RESIZED )
  {
    cv::Mat scaled_original;

    if( image.rows <= settings.chip_height && image.cols <= settings.chip_width )
    {
      regions_to_process.push_back( image );
      region_properties.push_back( windowed_region_prop( original_dims, 1.0 ) );
    }
    else
    {
      if( ( image.rows * image.cols ) >= settings.chip_adaptive_thresh )
      {
        regions_to_process.push_back( resized_image );
        region_properties.push_back( windowed_region_prop( original_dims, 1.0 / scale_factor ) );
      }

      double scaled_original_scale = scale_image_maintaining_ar( image,
        scaled_original, settings.chip_width, settings.chip_height, settings.black_pad );

      regions_to_process.push_back( scaled_original );
      region_properties.push_back( windowed_region_prop( original_dims, 1.0 / scaled_original_scale ) );
    }
  }
  else if( mode != CHIP && mode != CHIP_AND_ORIGINAL )
  {
    regions_to_process.push_back( resized_image );
    region_properties.push_back( windowed_region_prop( original_dims, 1.0 / scale_factor ) );
  }
  else
  {
    // Chip up scaled image
    for( int li = 0;
         li < resized_image.cols - settings.chip_width + settings.chip_step_width;
         li += settings.chip_step_width )
    {
      int ti = std::min( li + settings.chip_width, resized_image.cols );

      for( int lj = 0;
           lj < resized_image.rows - settings.chip_height + settings.chip_step_height;
           lj += settings.chip_step_height )
      {
        int tj = std::min( lj + settings.chip_height, resized_image.rows );

        if( tj-lj < 0 || ti-li < 0 )
        {
          continue;
        }

        cv::Rect resized_roi( li, lj, ti-li, tj-lj );
        cv::Rect original_roi( li / scale_factor,
                               lj / scale_factor,
                               (ti-li) / scale_factor,
                               (tj-lj) / scale_factor );

        cv::Mat cropped_chip = resized_image( resized_roi );
        cv::Mat scaled_crop;

        double scaled_crop_scale = scale_image_maintaining_ar(
          cropped_chip, scaled_crop, settings.chip_width, settings.chip_height,
          settings.black_pad );

        regions_to_process.push_back( scaled_crop );

        region_properties.push_back(
          windowed_region_prop( original_roi,
            settings.chip_edge_filter,
            ( li + settings.chip_step_width ) >=
              ( resized_image.cols - settings.chip_width + settings.chip_step_width ),
            ( lj + settings.chip_step_height ) >=
              ( resized_image.rows - settings.chip_height + settings.chip_step_height ),
            1.0 / scaled_crop_scale,
            li, lj,
            1.0 / scale_factor ) );
      }
    }

    // Extract full sized image chip if enabled
    if( mode == CHIP_AND_ORIGINAL )
    {
      cv::Mat scaled_original;

      if( settings.original_to_chip_size )
      {
        double scaled_original_scale = scale_image_maintaining_ar( image,
          scaled_original, settings.chip_width, settings.chip_height, settings.black_pad );

        regions_to_process.push_back( scaled_original );
        region_properties.push_back( windowed_region_prop( original_dims, 1.0 / scaled_original_scale ) );
      }
      else
      {
        regions_to_process.push_back( image );
        region_properties.push_back( windowed_region_prop( original_dims, 1.0 ) );
      }
    }
  }
}

// -----------------------------------------------------------------------------
vital::detected_object_set_sptr
scale_detections_to_region(
  const vital::detected_object_set_sptr detections,
  const windowed_region_prop& region_info )
{
  if( !detections || detections->empty() )
  {
    return std::make_shared< vital::detected_object_set >();
  }

  const cv::Rect& roi = region_info.original_roi;
  std::vector< vital::detected_object_sptr > region_dets;

  // Filter and transform detections that overlap with this region
  for( auto det : *detections )
  {
    if( !det )
    {
      continue;
    }

    vital::bounding_box_d det_box = det->bounding_box();
    vital::bounding_box_d roi_box( roi.x, roi.y, roi.x + roi.width, roi.y + roi.height );

    // Check if detection overlaps with this region
    vital::bounding_box_d overlap = vital::intersection( roi_box, det_box );
    if( overlap.area() <= 0 )
    {
      continue;
    }

    // Clone the detection so we don't modify the original
    auto region_det = det->clone();

    // Apply inverse scale2 transformation (divide by scale2)
    if( region_info.scale2 != 1.0 )
    {
      vital::detected_object_set_sptr temp_set = std::make_shared< vital::detected_object_set >();
      temp_set->add( region_det );
      temp_set->scale( 1.0 / region_info.scale2 );
      region_det = *temp_set->begin();
    }

    // Apply inverse shift transformation (subtract shift)
    if( region_info.shiftx != 0 || region_info.shifty != 0 )
    {
      vital::detected_object_set_sptr temp_set = std::make_shared< vital::detected_object_set >();
      temp_set->add( region_det );
      temp_set->shift( -region_info.shiftx, -region_info.shifty );
      region_det = *temp_set->begin();
    }

    // Apply inverse scale1 transformation (divide by scale1)
    if( region_info.scale1 != 1.0 )
    {
      vital::detected_object_set_sptr temp_set = std::make_shared< vital::detected_object_set >();
      temp_set->add( region_det );
      temp_set->scale( 1.0 / region_info.scale1 );
      region_det = *temp_set->begin();
    }

    region_dets.push_back( region_det );
  }

  return vital::detected_object_set_sptr(
    new vital::detected_object_set( region_dets ) );
}

// -----------------------------------------------------------------------------
void
separate_boundary_detections(
  const vital::detected_object_set_sptr detections,
  int region_width,
  int region_height,
  vital::detected_object_set_sptr& boundary_detections,
  vital::detected_object_set_sptr& interior_detections )
{
  boundary_detections = std::make_shared< vital::detected_object_set >();
  interior_detections = std::make_shared< vital::detected_object_set >();

  if( !detections )
  {
    return;
  }

  for( auto det : *detections )
  {
    if( !det )
    {
      continue;
    }

    vital::bounding_box_d bbox = det->bounding_box();

    // Check if detection touches any boundary
    bool touches_boundary =
      ( bbox.min_x() <= 0.0 ) ||
      ( bbox.min_y() <= 0.0 ) ||
      ( bbox.max_x() >= region_width - 1 ) ||
      ( bbox.max_y() >= region_height - 1 );

    if( touches_boundary )
    {
      boundary_detections->add( det );
    }
    else
    {
      interior_detections->add( det );
    }
  }
}

} } } // end namespace
