// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief metadata_filter algorithm instantiation

#include <vital/algo/algorithm.txx>
#include <vital/algo/metadata_filter.h>

namespace kwiver {

namespace vital {

namespace algo {

metadata_filter
::metadata_filter()
{
  attach_logger( "algo.metadata_filter" ); // specify a logger
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::metadata_filter );
/// \endcond
