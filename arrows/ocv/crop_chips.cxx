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
 * \brief Implementation of OCV split image algorithm
 */

#include "crop_chips.h"

#include <arrows/ocv/image_container.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv {

/// Constructor
crop_chips
::crop_chips()
{
}

/// Destructor
crop_chips
::~crop_chips()
{
}

/// Extract sub-chips from a parent image
kwiver::vital::image_container_set_sptr
crop_chips
::crop(kwiver::vital::image_container_sptr const img,
       std::vector<kwiver::vital::bounding_box_d> const& bboxes) const
{
  kwiver::vital::image_container_set_sptr output;

  std::vector< kwiver::vital::image_container_sptr > _chips;

  cv::Mat cv_image = ocv::image_container::vital_to_ocv( img->get_image() );
  for (auto bbox : bboxes)
  {
    cv::Mat chip = cv_image( cv::Rect( bbox.min_x(), bbox.min_y(),
                                       bbox.width(), bbox.height() ) );
    auto chip_sptr = image_container_sptr(
            new ocv::image_container( chip.clone() ) );
    _chips.push_back( chip_sptr );
  }

  output = std::make_shared<kwiver::vital::simple_image_container_set>(_chips);
  return output;
}

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
