// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of internal KLV utility functions.

#ifndef KWIVER_ARROWS_KLV_KLV_UTIL_H_
#define KWIVER_ARROWS_KLV_KLV_UTIL_H_

#include <vital/optional.h>

#include <tuple>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, kwiver::vital::optional< T > const& value )
{
  if( value )
  {
    os << *value;
  }
  else
  {
    os << "(empty)";
  }
  return os;
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_lt( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::tie( ( lhs.*args )... ) < std::tie( ( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_gt( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::tie( ( lhs.*args )... ) > std::tie( ( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_le( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::tie( ( lhs.*args )... ) <= std::tie( ( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_ge( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::tie( ( lhs.*args )... ) >= std::tie( ( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_eq( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::tie( ( lhs.*args )... ) == std::tie( ( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_ne( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::tie( ( lhs.*args )... ) != std::tie( ( rhs.*args )... );
}

// ----------------------------------------------------------------------------
#define DECLARE_CMP( T )                                      \
KWIVER_ALGO_KLV_EXPORT bool operator< ( T const&, T const& ); \
KWIVER_ALGO_KLV_EXPORT bool operator> ( T const&, T const& ); \
KWIVER_ALGO_KLV_EXPORT bool operator<=( T const&, T const& ); \
KWIVER_ALGO_KLV_EXPORT bool operator>=( T const&, T const& ); \
KWIVER_ALGO_KLV_EXPORT bool operator==( T const&, T const& ); \
KWIVER_ALGO_KLV_EXPORT bool operator!=( T const&, T const& );

// ----------------------------------------------------------------------------
#define DEFINE_STRUCT_CMP( T, ... )                                                          \
bool operator< ( T const& lhs, T const& rhs ) { return struct_lt( lhs, rhs, __VA_ARGS__ ); } \
bool operator> ( T const& lhs, T const& rhs ) { return struct_gt( lhs, rhs, __VA_ARGS__ ); } \
bool operator<=( T const& lhs, T const& rhs ) { return struct_le( lhs, rhs, __VA_ARGS__ ); } \
bool operator>=( T const& lhs, T const& rhs ) { return struct_ge( lhs, rhs, __VA_ARGS__ ); } \
bool operator==( T const& lhs, T const& rhs ) { return struct_eq( lhs, rhs, __VA_ARGS__ ); } \
bool operator!=( T const& lhs, T const& rhs ) { return struct_ne( lhs, rhs, __VA_ARGS__ ); }

// ----------------------------------------------------------------------------
#define DEFINE_STRUCT_CMP_TUPLIZE( T )                                                     \
bool operator< ( T const& lhs, T const& rhs ) { return tuplize( lhs ) <  tuplize( rhs ); } \
bool operator> ( T const& lhs, T const& rhs ) { return tuplize( lhs ) >  tuplize( rhs ); } \
bool operator<=( T const& lhs, T const& rhs ) { return tuplize( lhs ) <= tuplize( rhs ); } \
bool operator>=( T const& lhs, T const& rhs ) { return tuplize( lhs ) >= tuplize( rhs ); } \
bool operator==( T const& lhs, T const& rhs ) { return tuplize( lhs ) == tuplize( rhs ); } \
bool operator!=( T const& lhs, T const& rhs ) { return tuplize( lhs ) != tuplize( rhs ); }

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
