// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of OCV histogram equalization image filter

#include "image_histogram_eq.h"

#include <arrows/ocv/image_container.h>

#include <opencv2/imgproc.hpp>

namespace kwiver {

namespace arrows {

namespace ocv {

// ----------------------------------------------------------------------------
vital::image_container_sptr
image_histogram_eq
::filter( vital::image_container_sptr img )
{
  cv::Mat src = image_container::vital_to_ocv(
    img->get_image(),
    image_container::OTHER_COLOR );
  cv::Mat dst;
  cv::equalizeHist( src, dst );
  return std::make_shared< image_container >(
    image_container( dst, image_container::OTHER_COLOR ) );
}

} // namespace ocv

} // namespace arrows

} // namespace kwiver
