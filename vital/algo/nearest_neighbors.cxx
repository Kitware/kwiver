// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief nearest_neighbors algorithm definition instantiation

#include "nearest_neighbors.h"
#include <vital/algo/algorithm.txx>

namespace kwiver {

namespace vital {

namespace algo {

nearest_neighbors
::nearest_neighbors()
{
  attach_logger( "algo.nearest_neighbors" );
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::nearest_neighbors );
/// \endcond
