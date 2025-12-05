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

#ifndef KWIVER_ARROWS_OCV_WINDOWED_UTILS
#define KWIVER_ARROWS_OCV_WINDOWED_UTILS


#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <opencv2/core/core.hpp>

#include <vital/algo/image_object_detector.h>
#include <vital/util/enum_converter.h>

namespace kwiver {
namespace arrows {
namespace ocv {

enum rescale_option {
  DISABLED = 0,
  MAINTAIN_AR,
  SCALE,
  CHIP,
  CHIP_AND_ORIGINAL,
  ORIGINAL_AND_RESIZED,
  ADAPTIVE
};

ENUM_CONVERTER( rescale_option_converter, rescale_option,
    { "disabled",             DISABLED },
    { "maintain_ar",          MAINTAIN_AR },
    { "scale",                SCALE },
    { "chip",                 CHIP },
    { "chip_and_original",    CHIP_AND_ORIGINAL },
    { "original_and_resized", ORIGINAL_AND_RESIZED },
    { "adaptive",             ADAPTIVE }
  )

struct window_settings
{
  window_settings();
  ~window_settings() {}

  vital::config_block_sptr config() const;
  void set_config( vital::config_block_sptr cfg );

  rescale_option mode;
  double scale;
  int chip_width;
  int chip_height;
  int chip_step_width;
  int chip_step_height;
  int chip_edge_filter;
  double chip_edge_max_prob;
  int chip_adaptive_thresh;
  int batch_size;
  int min_detection_dim;
  bool original_to_chip_size;
  bool black_pad;
};

struct windowed_region_prop
{
  explicit windowed_region_prop( cv::Rect r, double s1 )
   : original_roi( r ), edge_filter( -1 ),
     right_border( false ), bottom_border( false ),
     scale1( s1 ), shiftx( 0 ), shifty( 0 ), scale2( 1.0 )
  {}

  explicit windowed_region_prop( cv::Rect r, int ef, bool rb, bool bb,
    double s1, int sx, int sy, double s2 )
   : original_roi( r ), edge_filter( ef ),
     right_border( rb ), bottom_border( bb ),
     scale1( s1 ), shiftx( sx ), shifty( sy ), scale2( s2 )
  {}

  cv::Rect original_roi;
  int edge_filter;
  bool right_border;
  bool bottom_border;
  double scale1;
  int shiftx, shifty;
  double scale2;
};

double
scale_image_maintaining_ar( const cv::Mat& src, cv::Mat& dst,
                            int width, int height, bool pad = false );

double
format_image( const cv::Mat& src, cv::Mat& dst, rescale_option option,
              double scale_factor, int width, int height, bool pad = false );

} } }

#endif /* KWIVER_ARROWS_OCV_WINDOWED_UTILS */
