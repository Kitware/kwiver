// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef VITAL_NONCOPYABLE_H
#define VITAL_NONCOPYABLE_H

#include <vital/vital_config.h>

namespace kwiver::vital {

/**
 * Mixin class for making a class non-copyable.
 */
class noncopyable
{
protected:
  noncopyable() = default;
  virtual ~noncopyable() = default;

  noncopyable( const noncopyable& ) = delete;
  noncopyable& operator=( const noncopyable& ) = delete;
};

} // namespace kwiver::vital

#endif /* VITAL_NONCOPYABLE_H */
