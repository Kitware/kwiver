// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef VITAL_ITERATOR_RANGE_H
#define VITAL_ITERATOR_RANGE_H

/// \file Class to wrap two iterators in a range-friendly way.

#include <iterator>
#include <utility>

namespace kwiver {

namespace vital {

namespace range {

// ----------------------------------------------------------------------------
/// Wraps two iterators into a range.
template < class Iterator >
class iterator_range
{
public:
  iterator_range( Iterator const& begin, Iterator const& end )
    : m_begin{ begin }, m_end{ end } {}
  iterator_range( Iterator&& begin, Iterator&& end )
    : m_begin{ std::move( begin ) }, m_end{ std::move( end ) } {}

  Iterator
  begin() const { return m_begin; }
  Iterator
  end() const { return m_end; }

  bool
  empty() const { return m_begin == m_end; }

  typename std::iterator_traits< Iterator >::difference_type
  size() const { return std::distance( m_begin, m_end ); }

protected:
  Iterator m_begin;
  Iterator m_end;
};

} // namespace range

} // namespace vital

} // namespace kwiver

#endif
