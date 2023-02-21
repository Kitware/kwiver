// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Comparison utilities.

#ifndef KWIVER_VITAL_UTIL_COMPARE_H_
#define KWIVER_VITAL_UTIL_COMPARE_H_

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Return -1 if \p lhs < \p rhs, 1 if \p lhs > \p rhs, or 0 if they are equal.
// TODO(C++20) Replace with <=>
template < class T >
int
threeway_compare( T const& lhs, T const& rhs )
{
  return ( lhs < rhs ) ? -1 : ( ( rhs < lhs ) ? 1 : 0 );
}

} // namespace vital

} // namespace kwiver

#endif
