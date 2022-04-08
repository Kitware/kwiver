// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV lengthy template class.

#include "klv_lengthy.h"

#include <arrows/klv/klv_util.h>

#include <vital/optional.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class T >
klv_lengthy< T >

::klv_lengthy( T const& value )
  : value{ value }, length{ 0 }
{}

// ----------------------------------------------------------------------------
template < class T >
klv_lengthy< T >

::klv_lengthy( T&& value )
  : value{ std::move( value ) }, length{ 0 }
{}

// ----------------------------------------------------------------------------
template < class T >
klv_lengthy< T >

::klv_lengthy( T const& value, size_t length )
  : value{ value }, length{ length }
{}

// ----------------------------------------------------------------------------
template < class T >
klv_lengthy< T >

::klv_lengthy( T&& value, size_t length )
  : value{ std::move( value ) }, length{ length }
{}

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, klv_lengthy< T > const& value )
{
  return os << value.value;
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator<( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs )
{
  return lhs.value < rhs.value;
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator>( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs )
{
  return lhs.value > rhs.value;
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator<=( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs )
{
  return lhs.value <= rhs.value;
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator>=( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs )
{
  return lhs.value >= rhs.value;
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator==( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs )
{
  return lhs.value == rhs.value;
}

// ----------------------------------------------------------------------------
template < class T >
bool
operator!=( klv_lengthy< T > const& lhs, klv_lengthy< T > const& rhs )
{
  return lhs.value != rhs.value;
}

// ----------------------------------------------------------------------------
#define KLV_INSTANTIATE( T )                                                    \
  template class klv_lengthy< T >;                                              \
  template std::ostream& operator<<( std::ostream&, klv_lengthy< T > const& );  \
  template bool operator>( klv_lengthy< T > const&, klv_lengthy< T > const& );  \
  template bool operator<( klv_lengthy< T > const&, klv_lengthy< T > const& );  \
  template bool operator<=( klv_lengthy< T > const&, klv_lengthy< T > const& ); \
  template bool operator>=( klv_lengthy< T > const&, klv_lengthy< T > const& ); \
  template bool operator==( klv_lengthy< T > const&, klv_lengthy< T > const& ); \
  template bool operator!=( klv_lengthy< T > const&, klv_lengthy< T > const& )

KLV_INSTANTIATE( double );
KLV_INSTANTIATE( kwiver::vital::optional< double > );

} // namespace klv

} // namespace arrows

} // namespace kwiver
