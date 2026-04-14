// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining the image_filter algorithm that acts as a bypass

#ifndef KWIVER_ARROWS_CORE_IMAGE_FILTER_BYPASS_H_
#define KWIVER_ARROWS_CORE_IMAGE_FILTER_BYPASS_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace core {

/// A class for bypassing image filtering
class KWIVER_ALGO_CORE_EXPORT image_filter_bypass
  : public vital::algo::image_filter
{
public:
  PLUGGABLE_IMPL(
    image_filter_bypass,
    "Performs no filtering and returns the given image container."
  )

  bool
  check_configuration( vital::config_block_sptr ) const override
  {
    return true;
  }

  /// Default image filter ( does nothing )
  ///
  /// \param [in] image_data image container
  /// \returns the input image container unchanged
  virtual vital::image_container_sptr filter(
    vital::image_container_sptr image_data ) override;
};

} // end namespace core

} // end namespace arrows

} // end namespace kwiver

#endif
