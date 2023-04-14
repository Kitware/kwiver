// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utility to provide an instance of a default-constructed class.

#ifndef KWIVER_VITAL_UTIL_SINGLETON_H_
#define KWIVER_VITAL_UTIL_SINGLETON_H_

namespace kwiver {

namespace vital {

template < class T >
class singleton
{
public:
  static T const&
  instance()
  {
    static T const value;
    return value;
  }
};

} // namespace vital

} // namespace kwiver

#endif
