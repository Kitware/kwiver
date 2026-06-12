// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// mesh_filter algorithm instantiation

#include "mesh_filter.h"

namespace kwiver {

namespace vital {

namespace algo {

mesh_filter
::mesh_filter()
{
  attach_logger( "algo.mesh_filter" );
}

} // namespace algo

} // namespace vital

} // namespace kwiver
