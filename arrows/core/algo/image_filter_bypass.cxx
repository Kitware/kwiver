// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of bypass image filter

#include "image_filter_bypass.h"

namespace kwiver {

namespace arrows {

namespace core {

/// Default image filter ( does nothing )
vital::image_container_sptr
image_filter_bypass
::filter( vital::image_container_sptr image_data )
{
  return image_data;
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
