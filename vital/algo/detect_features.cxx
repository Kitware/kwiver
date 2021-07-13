// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief detect_features algorithm definition instantiation
 */

#include <vital/algo/algorithm.txx>
#include <vital/algo/detect_features.h>

namespace kwiver {

namespace vital {

namespace algo {

detect_features
::detect_features()
{
  attach_logger( "algo.detect_features" );
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::detect_features );
/// \endcond
