// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Tuple handling utilities.

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
template < class T, size_t Index >
using _sized_tuple_helper2 = T;

// ----------------------------------------------------------------------------
template < class T, size_t... Indices >
using _sized_tuple_helper1 =
  std::tuple< _sized_tuple_helper2< T, Indices > ... >;

// ----------------------------------------------------------------------------
template < class T, class IndexSeq >
struct _sized_tuple_helper0 {};

// ----------------------------------------------------------------------------
template < class T, size_t... Indices >
struct _sized_tuple_helper0< T, std::integer_sequence< size_t, Indices... > >
{
  using type = _sized_tuple_helper1< T, Indices... >;
};

// ----------------------------------------------------------------------------
/// A tuple with \p N elements, all of type \p T.
template < class T, size_t N >
using sized_tuple =
  typename _sized_tuple_helper0< T, std::make_index_sequence< N > >::type;

// ----------------------------------------------------------------------------
template < class T, size_t... Indices >
inline
auto
_tie_n( T* ptr, std::index_sequence< Indices... > const& )
{
  return std::tie( ptr[ Indices ] ... );
}

// ----------------------------------------------------------------------------
/// Returns a tuple referencing \p N contiguous objects starting at \p ptr.
template < size_t N, class T >
inline
auto
tie_n( T* ptr )
{
  return _tie_n( ptr, std::make_index_sequence< N >{} );
}

// ----------------------------------------------------------------------------
template < class T, size_t... Indices >
inline
auto
_tie_stepped_n(
  T* ptr, ptrdiff_t step, std::index_sequence< Indices... > const& )
{
  return std::tie( ptr[ step * Indices ] ... );
}

// ----------------------------------------------------------------------------
/// Returns a tuple referencing \p N objects, separated by \p step bytes,
/// starting at \p ptr.
template < size_t N, class T >
inline
auto
tie_stepped_n( T* ptr, ptrdiff_t step )
{
  return _tie_stepped_n( ptr, step, std::make_index_sequence< N >{} );
}

// ----------------------------------------------------------------------------
template < class T, size_t... Indices >
inline
auto
_tie_tuple( T& x, std::integer_sequence< size_t, Indices... > )
{
  return std::tie( std::get< Indices >( x )... );
}

// ----------------------------------------------------------------------------
/// Returns a tuple of references to each element of \p x.
template < class... T >
inline
auto
tie_tuple( std::tuple< T... >& x )
{
  return _tie_tuple( x, std::index_sequence_for< T... >{} );
}

} // namespace vital

} // namespace kwiver
