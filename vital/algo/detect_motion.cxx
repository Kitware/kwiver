// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief detect_motion algorithm instantiation
 */

#include <vital/algo/detect_motion.h>

#include <vital/algo/algorithm.txx>

namespace kwiver {

namespace vital {

namespace algo {

detect_motion
::detect_motion()
{
  attach_logger( "detect_motion" ); // specify a logger
}

} // namespace algo

} // namespace vital

} // namespace kwiver

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF( kwiver::vital::algo::detect_motion );
/// \endcond
