// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of internal KLV utility functions.

#ifndef KWIVER_ARROWS_KLV_KLV_UTIL_H_
#define KWIVER_ARROWS_KLV_KLV_UTIL_H_

#include <vital/exceptions.h>
#include <vital/util/interval.h>

#include <optional>
#include <ostream>
#include <set>
#include <tuple>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
#define ENUM_AND_NAME( X ) X, #X

// ----------------------------------------------------------------------------
template< class T >
std::ostream&
operator<<( std::ostream& os, vital::interval< T > const& value )
{
  return os << "( " << value.lower() << ", " << value.upper() << " )";
}

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, std::optional< T > const& value )
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
template < class T >
std::ostream&
operator<<( std::ostream& os, std::vector< T > const& value )
{
  os << "{ ";
  auto first = true;
  for( T const& item : value )
  {
    first = first ? false : ( os << ", ", false );
    os << item;
  }
  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, std::set< T > const& value )
{
  os << "{ ";
  for( T const& item : value )
  {
    os << item;
    if( &item != &*value.cend() )
    {
      os << ", ";
    }
  }
  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
template< class T >
struct wrap_cmp_nan
{
  explicit wrap_cmp_nan( T const& value ) : value{ value } {}

  bool operator<( wrap_cmp_nan< T > const& other ) const
  {
    if constexpr( std::is_floating_point_v< T > )
    {
      return value < other.value ||
             ( std::isnan( value ) && !std::isnan( other.value ) );
    }
    else
    {
      return value < other.value;
    }
  }

  bool operator>( wrap_cmp_nan< T > const& other ) const
  {
    return other < *this;
  }

  bool operator<=( wrap_cmp_nan< T > const& other ) const
  {
    return !( other < *this );
  }

  bool operator>=( wrap_cmp_nan< T > const& other ) const
  {
    return !( *this < other );
  }

  bool operator==( wrap_cmp_nan< T > const& other ) const
  {
    if constexpr( std::is_floating_point_v< T > )
    {
      return value == other.value ||
             ( std::isnan( value ) && std::isnan( other.value ) );
    }
    else
    {
      return value == other.value;
    }
  }

  bool operator!=( wrap_cmp_nan< T > const& other ) const
  {
    return !( *this == other );
  }

  T const& value;
};

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_lt( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::make_tuple( wrap_cmp_nan( lhs.*args )... ) <
         std::make_tuple( wrap_cmp_nan( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_gt( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::make_tuple( wrap_cmp_nan( lhs.*args )... ) >
         std::make_tuple( wrap_cmp_nan( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_le( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::make_tuple( wrap_cmp_nan( lhs.*args )... ) <=
         std::make_tuple( wrap_cmp_nan( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_ge( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::make_tuple( wrap_cmp_nan( lhs.*args )... ) >=
         std::make_tuple( wrap_cmp_nan( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_eq( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::make_tuple( wrap_cmp_nan( lhs.*args )... ) ==
         std::make_tuple( wrap_cmp_nan( rhs.*args )... );
}

// ----------------------------------------------------------------------------
template< class T, class... Args >
bool
struct_ne( T const& lhs, T const& rhs, Args T::*... args )
{
  return std::make_tuple( wrap_cmp_nan( lhs.*args )... ) !=
         std::make_tuple( wrap_cmp_nan( rhs.*args )... );
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
#define DECLARE_TEMPLATE_CMP( TYPE )                                                    \
template< class T > KWIVER_ALGO_KLV_EXPORT bool operator< ( TYPE const&, TYPE const& ); \
template< class T > KWIVER_ALGO_KLV_EXPORT bool operator> ( TYPE const&, TYPE const& ); \
template< class T > KWIVER_ALGO_KLV_EXPORT bool operator<=( TYPE const&, TYPE const& ); \
template< class T > KWIVER_ALGO_KLV_EXPORT bool operator>=( TYPE const&, TYPE const& ); \
template< class T > KWIVER_ALGO_KLV_EXPORT bool operator==( TYPE const&, TYPE const& ); \
template< class T > KWIVER_ALGO_KLV_EXPORT bool operator!=( TYPE const&, TYPE const& );

// ----------------------------------------------------------------------------
#define DEFINE_STRUCT_CMP( T, ... )                                                          \
bool operator< ( T const& lhs, T const& rhs ) { return struct_lt( lhs, rhs, __VA_ARGS__ ); } \
bool operator> ( T const& lhs, T const& rhs ) { return struct_gt( lhs, rhs, __VA_ARGS__ ); } \
bool operator<=( T const& lhs, T const& rhs ) { return struct_le( lhs, rhs, __VA_ARGS__ ); } \
bool operator>=( T const& lhs, T const& rhs ) { return struct_ge( lhs, rhs, __VA_ARGS__ ); } \
bool operator==( T const& lhs, T const& rhs ) { return struct_eq( lhs, rhs, __VA_ARGS__ ); } \
bool operator!=( T const& lhs, T const& rhs ) { return struct_ne( lhs, rhs, __VA_ARGS__ ); }

// ----------------------------------------------------------------------------
#define DEFINE_TEMPLATE_CMP( TYPE, ... )                                                                               \
template< class T > bool operator< ( TYPE const& lhs, TYPE const& rhs ) { return struct_lt( lhs, rhs, __VA_ARGS__ ); } \
template< class T > bool operator> ( TYPE const& lhs, TYPE const& rhs ) { return struct_gt( lhs, rhs, __VA_ARGS__ ); } \
template< class T > bool operator<=( TYPE const& lhs, TYPE const& rhs ) { return struct_le( lhs, rhs, __VA_ARGS__ ); } \
template< class T > bool operator>=( TYPE const& lhs, TYPE const& rhs ) { return struct_ge( lhs, rhs, __VA_ARGS__ ); } \
template< class T > bool operator==( TYPE const& lhs, TYPE const& rhs ) { return struct_eq( lhs, rhs, __VA_ARGS__ ); } \
template< class T > bool operator!=( TYPE const& lhs, TYPE const& rhs ) { return struct_ne( lhs, rhs, __VA_ARGS__ ); }

// ----------------------------------------------------------------------------
#define DEFINE_STRUCT_CMP_TUPLIZE( T )                                                     \
bool operator< ( T const& lhs, T const& rhs ) { return tuplize( lhs ) <  tuplize( rhs ); } \
bool operator> ( T const& lhs, T const& rhs ) { return tuplize( lhs ) >  tuplize( rhs ); } \
bool operator<=( T const& lhs, T const& rhs ) { return tuplize( lhs ) <= tuplize( rhs ); } \
bool operator>=( T const& lhs, T const& rhs ) { return tuplize( lhs ) >= tuplize( rhs ); } \
bool operator==( T const& lhs, T const& rhs ) { return tuplize( lhs ) == tuplize( rhs ); } \
bool operator!=( T const& lhs, T const& rhs ) { return tuplize( lhs ) != tuplize( rhs ); }

// ----------------------------------------------------------------------------
template< class T >
class iterator_tracker {
public:
  iterator_tracker( T& it, size_t length )
    : m_begin( it ), m_length{ length }, m_it( it )
  {
    static_assert(
      std::is_same_v< std::decay_t< decltype( *it ) >, uint8_t >,
      "iterator must point to uint8_t" );
  }

  template<
    class Int,
    std::enable_if_t< std::is_unsigned_v< std::decay_t< Int > >, bool > = true >
  size_t verify( Int count ) const
  {
    if( count > remaining() )
    {
      m_it = m_begin;
      VITAL_THROW( vital::metadata_buffer_overflow,
                   "tried to read or write past end of data buffer" );
    }
    return count;
  }

  template<
    class Int,
    std::enable_if_t< std::is_signed_v< std::decay_t< Int > >, bool > = true >
  size_t verify( Int count ) const
  {
    if( count < 0 )
    {
      m_it = m_begin;
      VITAL_THROW( vital::metadata_buffer_overflow,
                   "tried to read or write a value of negative length" );
    }

    return verify( static_cast< size_t >( count ) );
  }

  size_t traversed() const
  {
    auto const distance = std::distance( m_begin, m_it );

    if( distance < 0 )
    {
      m_it = m_begin;
      VITAL_THROW( vital::metadata_buffer_overflow,
                   "read or written before beginning of data buffer" );
    }

    if( static_cast< size_t >( distance ) > m_length )
    {
      m_it = m_begin;
      VITAL_THROW( vital::metadata_buffer_overflow,
                   "read or written past end of data buffer" );
    }

    return distance;
  }

  size_t remaining() const
  {
    return m_length - traversed();
  }

  T begin() const
  {
    return m_begin;
  }

  T end() const
  {
    return m_begin + m_length;
  }

  T& it() const
  {
    return m_it;
  }

private:
  T m_begin;
  size_t m_length;
  T& m_it;
};

// ----------------------------------------------------------------------------
template< class T >
iterator_tracker< T > track_it( T& it, size_t length )
{
  return { it, length };
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
