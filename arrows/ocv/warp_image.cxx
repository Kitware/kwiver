/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * \brief Implementation of ocv::warp_image
 */

#include "warp_image.h"

#include <vital/exceptions.h>

#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/eigen.hpp>

namespace kwiver {
namespace arrows {
namespace ocv {

using namespace kwiver::vital;


/// Private implementation class
class warp_image::priv
{
public:
  /// Constructor
  priv()
    : auto_size_output(false)
  {
  }

  /// number of feature matches required for acceptance
  bool auto_size_output;
};


/// Constructor
warp_image
::warp_image()
: d_(new priv)
{
  attach_logger( "arrows.ocv.warp_image" );
}


/// Destructor
warp_image
::~warp_image() VITAL_NOTHROW
{
}


/// Get this alg's \link vital::config_block configuration block \endlink
vital::config_block_sptr
warp_image
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value("auto_size_output", d_->auto_size_output,
                    "If an output image is allocated, resize to contain all input pixels");
  return config;
}


/// Set this algo's properties via a config block
void
warp_image
::set_configuration(vital::config_block_sptr in_config)
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(in_config);

  d_->auto_size_output = config->get_value<bool>("auto_size_output");
}


bool
warp_image
::check_configuration(vital::config_block_sptr config) const
{
  return true;
}


/// Warp an input image with a homography
void
warp_image
::warp( image_container_sptr image_src,
        image_container_sptr& image_dest,
        homography_sptr homog) const
{
  if ( !image_src || !homog )
  {
    throw vital::invalid_data("Inputs to ocv::warp_image are null");
  }

  cv::Mat cv_src = ocv::image_container::vital_to_ocv(image_src->get_image());

  cv::Mat cv_H;
  eigen2cv(homog->matrix(), cv_H);

  cv::Mat cv_dest;
  if ( image_dest )
  {
    cv_dest = ocv::image_container::vital_to_ocv(image_dest->get_image());
  }

  cv::Size cv_dsize = cv_dest.size();
  if ( cv_dsize.width < 1 || cv_dsize.height < 1 )
  {
    cv_dsize = cv_src.size();
  }

  cv::warpPerspective(cv_src, cv_dest, cv_H, cv_dsize);

  image_dest = std::make_shared<ocv::image_container>(cv_dest);
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
