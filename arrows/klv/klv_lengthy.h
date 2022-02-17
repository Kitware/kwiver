// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV lengthy template class.

#ifndef KWIVER_ARROWS_KLV_KLV_LENGTHY_H_
#define KWIVER_ARROWS_KLV_KLV_LENGTHY_H_

#include <arrows/klv/kwiver_algo_klv_export.h>
#include <arrows/klv/klv_util.h>

#include <ostream>

#include <cstddef>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class T >
struct KWIVER_ALGO_KLV_EXPORT klv_lengthy
{
  klv_lengthy( T const& value );
  klv_lengthy( T&& value );
  klv_lengthy( T const& value, size_t length );
  klv_lengthy( T&& value, size_t length );

  T value;
  size_t length;
};

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_lengthy< T > const& value );

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs );

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
bool
operator>( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs );

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
bool
operator<=( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs );

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
bool
operator>=( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs );

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs );

// ----------------------------------------------------------------------------
template < class T >
KWIVER_ALGO_KLV_EXPORT
bool
operator!=( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
