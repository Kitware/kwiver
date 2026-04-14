// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV histogram equalization image filter wrapper

#ifndef KWIVER_ARROWS_OCV_IMAGE_HISTOGRAM_EQ_H_
#define KWIVER_ARROWS_OCV_IMAGE_HISTOGRAM_EQ_H_

#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace ocv {

/// Filter images using OpenCV histogram equalization.
///
/// This can be used to globally enhance image contrast.
class KWIVER_ALGO_OCV_EXPORT image_histogram_eq
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    image_histogram_eq,
    "Equalize image histogram using OpenCV."
  )

  bool
  check_configuration( vital::config_block_sptr ) const override
  { return true; }

  vital::image_container_sptr filter(
    vital::image_container_sptr img ) override;
};

} // namespace ocv

} // namespace arrows

} // namespace kwiver

#endif
