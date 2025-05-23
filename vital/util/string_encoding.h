// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities for dealing with string encoding methods.

#include <vital/util/vital_util_export.h>

#include <string>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Return the number of Unicode code points in the given UTF-8 string.
///
/// \param begin Pointer to first character of string.
/// \param end Pointer one past last character of string.
///
/// \throw std::runtime_error If the string is not valid UTF-8.
VITAL_UTIL_EXPORT
size_t utf8_code_point_count( char const* begin, char const* end );

// ----------------------------------------------------------------------------
/// Return the number of Unicode code points in the given UTF-8 string.
///
/// \param s String to analyze,
///
/// \throw std::runtime_error If \p s is not valid UTF-8.
VITAL_UTIL_EXPORT
size_t utf8_code_point_count( std::string const& s );

} // namespace vital

} // namespace kwiver
