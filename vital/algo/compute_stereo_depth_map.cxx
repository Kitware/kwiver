// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief compute_stereo_depth_map algorithm definition instantiation

#include <vital/algo/algorithm.txx>
#include <vital/algo/compute_stereo_depth_map.h>

namespace kwiver {

namespace vital {

namespace algo {

compute_stereo_depth_map
::compute_stereo_depth_map()
{
  attach_logger( "algo.compute_stereo_depth_map" );
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::compute_stereo_depth_map );
/// \endcond
