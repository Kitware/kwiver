// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

///
/// \file
/// \brief match_features algorithm instantiation
///

#include <vital/algo/algorithm.txx>
#include <vital/algo/match_features.h>

namespace kwiver {

namespace vital {

namespace algo {

match_features
::match_features()
{
  attach_logger( "algo.match_features" );
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::match_features );
/// \endcond
