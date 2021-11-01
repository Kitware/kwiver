// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Utility functions regarding arithmetic types.

#include <type_traits>

#include <cmath>

namespace kwiver {

namespace vital {

#ifdef _MSC_VER

// isnan() wrapper which accepts integral types, even on noncompliant MSVC.
// See https://github.com/microsoft/STL/issues/519
template < class T >
bool
isnan( T value )
{
  // TODO(C++17): make if constexpr and remove the static_cast
  if( std::is_floating_point< T >::value )
  {
    using promoted_t = typename std::common_type< T, float >::type;
    return std::isnan( static_cast< promoted_t >( value ) );
  }
  return false;
}

#else
using std::isnan;
#endif

} // namespace vital

} // namespace kwiver
