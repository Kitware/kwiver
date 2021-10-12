/*ckwg +29
 * Copyright 2013-2016, 2019 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

/**
 * \file
 * \brief OCV detect_features algorithm impl interface
 */

#ifndef KWIVER_ARROWS_OCV_DETECT_FEATURES_H_
#define KWIVER_ARROWS_OCV_DETECT_FEATURES_H_

#include <vital/algo/detect_features.h>

#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <opencv2/features2d/features2d.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

/// OCV Specific base definition for algorithms that detect feature points
/**
 * This extended algorithm_def provides a common implementation for the detect
 * method.
 */
class KWIVER_ALGO_OCV_EXPORT detect_features
  : public kwiver::vital::algo::detect_features
{
public:
  /// Extract a set of image features from the provided image
  /**
   * A given mask image should be one-channel (mask->depth() == 1). If the
   * given mask image has more than one channel, only the first will be
   * considered.
   *
   * \param image_data contains the image data to process
   * \param mask Mask image where regions of positive values (boolean true)
   *             indicate regions to consider. Only the first channel will be
   *             considered.
   * \returns a set of image features
   */
  virtual vital::feature_set_sptr
  detect(vital::image_container_sptr image_data,
         vital::image_container_sptr mask = vital::image_container_sptr()) const;

protected:
  /// the feature detector algorithm
  cv::Ptr<cv::FeatureDetector> detector;
};

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver

#endif
