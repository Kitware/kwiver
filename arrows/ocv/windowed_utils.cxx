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

} } } // end namespace
