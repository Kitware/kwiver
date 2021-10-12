/*ckwg +29
 * Copyright 2014-2016 by Kitware, Inc.
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

#include "detect_features.h"

#include <vector>

#include <arrows/viscl/feature_set.h>
#include <arrows/viscl/image_container.h>

#include <viscl/tasks/hessian.h>


namespace kwiver {
namespace arrows {
namespace vcl {


/// Private implementation class
class detect_features::priv
{
public:
  /// Constructor
  priv() : max_kpts(5000), thresh(0.003f), sigma(2.0f)
  {
  }

  viscl::hessian detector;
  unsigned int max_kpts;
  float thresh;
  float sigma;
};


/// Constructor
detect_features
::detect_features()
: d_(new priv)
{
}


/// Destructor
detect_features
::~detect_features()
{
}

/// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
detect_features
::get_configuration() const
{
  vital::config_block_sptr config = algorithm::get_configuration();
  config->set_value("max_keypoints", d_->max_kpts, "Maximum number of features to detect on an image.");
  config->set_value("thresh", d_->thresh, "Threshold on the determinant of Hessian for keypoint candidates.");
  config->set_value("sigma", d_->sigma, "Smoothing scale.");
  return config;
}

/// Set this algorithm's properties via a config block
void
detect_features
::set_configuration(vital::config_block_sptr config)
{
  d_->max_kpts = config->get_value<unsigned int>("max_keypoints", d_->max_kpts);
  d_->thresh = config->get_value<float>("thresh", d_->thresh);
  d_->sigma = config->get_value<float>("sigma", d_->sigma);
}

/// Check that the algorithm's configuration vital::config_block is valid
bool
detect_features
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}

/// Extract a set of image features from the provided image
/// \param image_data contains the image data to process
/// \returns a set of image features
vital::feature_set_sptr
detect_features
::detect(vital::image_container_sptr image_data, vital::image_container_sptr mask) const
{
  // TODO: Do something with the given mask

  viscl::image img = vcl::image_container_to_viscl(*image_data);
  vcl::feature_set::type feature_data;

  d_->detector.smooth_and_detect(img, feature_data.kptmap_, feature_data.features_, feature_data.numfeat_,
                                 d_->max_kpts, d_->thresh, d_->sigma);

  return vital::feature_set_sptr(new feature_set(feature_data));
}


} // end namespace vcl
} // end namespace arrows
} // end namespace kwiver
