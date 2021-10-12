// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV set template class.

#ifndef KWIVER_VITAL_KLV_KLV_SET_H_
#define KWIVER_VITAL_KLV_KLV_SET_H_

#include <arrows/klv/klv_value.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <boost/range.hpp>

namespace kwiver {

namespace arrows {

namespace klv {

// ---------------------------------------------------------------------------
/// Base class for KLV universal and local sets.
///
/// This class is mostly a wrapper around a std::multimap, but with a slightly
/// modified interface. Usually, there will be only one entry for a key, so we
/// include the at() and find() methods here - otherwise, extracting a single
/// item from a multimap and verifying its singularity is awkward. Another
/// issue addressed here is that multimaps only sort by key, not value.
/// Therefore, multimaps { A : 1, A : 2 } and { A : 2, A : 1 } evaluate as not
/// equal. This class treats those sets as equal.
template < class Key >
class klv_set
{
public:
  using container = std::multimap< Key, klv_value >;
  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;
  using value_type = typename container::value_type;
  using range = boost::iterator_range< iterator >;
  using const_range = boost::iterator_range< const_iterator >;

  klv_set() {}

  klv_set( container const& items ) : m_items{ items } {}

  klv_set( std::initializer_list< value_type > const& items )
    : m_items{ items } {}

  iterator
  begin()
  {
    return m_items.begin();
  }

  const_iterator
  begin() const
  {
    return m_items.cbegin();
  }

  const_iterator
  cbegin() const
  {
    return m_items.cbegin();
  }

  iterator
  end()
  {
    return m_items.end();
  }

  const_iterator
  end() const
  {
    return m_items.cend();
  }

  const_iterator
  cend() const
  {
    return m_items.cend();
  }

  size_t
  size() const
  {
    return m_items.size();
  }

  size_t
  count( Key const& key ) const
  {
    return m_items.count( key );
  }

  bool
  has( Key const& key ) const
  {
    return m_items.count( key );
  }

  void
  add( Key const& key, klv_value const& datum )
  {
    m_items.emplace( key, datum );
  }

  void
  erase( const_iterator it )
  {
    m_items.erase( it );
  }

  void
  erase( Key const& key )
  {
    m_items.erase( key );
  }

  void
  clear()
  {
    m_items.clear();
  }

  /// Return single entry corresponding to \p key, or end iterator on failure.
  iterator
  find( Key const& key )
  {
    auto const it = m_items.lower_bound( key );
    if( it != m_items.end() )
    {
      auto const next_it = std::next( it );
      if( next_it == m_items.end() || next_it->first != key )
      {
        return it;
      }
    }
    return m_items.end();
  }

  /// \copydoc iterator find( Key const& key )
  const_iterator
  find( Key const& key ) const
  {
    auto const it = m_items.lower_bound( key );
    if( it != m_items.end() )
    {
      auto const next_it = std::next( it );
      if( next_it == m_items.end() || next_it->first != key )
      {
        return it;
      }
    }
    return m_items.cend();
  }

  /// Return single value corresponding to \p key.
  ///
  /// \throws out_of_range If no \p key entry is present.
  /// \throws logic_error If more than one \p key entry is present.
  klv_value&
  at( Key const& key )
  {
    auto const it = m_items.lower_bound( key );
    if( it != m_items.end() )
    {
      auto const next_it = std::next( it );
      if( next_it == m_items.end() || next_it->first != key )
      {
        return it->second;
      }
      throw std::logic_error( "more than one instance of key found" );
    }
    throw std::out_of_range( "key not found" );
  }

  /// \copydoc klv_value& at( Key const& key )
  klv_value const&
  at( Key const& key ) const
  {
    auto const it = m_items.lower_bound( key );
    if( it != m_items.cend() )
    {
      auto const next_it = std::next( it );
      if( next_it == m_items.cend() || next_it->first != key )
      {
        return it->second;
      }
      throw std::runtime_error( "more than one instance of key found" );
    }
    throw std::out_of_range( "key not found" );
  }

  /// Return the range of entries corresponding to \p key.
  ///
  /// \note Order of entries returned is not defined.
  range
  all_at( Key const& key )
  {
    auto const equal_range = m_items.equal_range( key );
    return { equal_range.first, equal_range.second };
  }

  /// \copydoc range all_at( Key const& key )
  const_range
  all_at( Key const& key ) const
  {
    auto const equal_range = m_items.equal_range( key );
    return { equal_range.first, equal_range.second };
  }

  /// Returns iterators to all entries, sorted by key, then by value.
  std::vector< iterator >
  fully_sorted() const
  {
    std::vector< iterator > result{ cbegin(), cend() };
    std::sort( result.begin(), result.end(), value_compare );
    return result;
  }

  friend bool
  operator==( klv_set< Key > const& lhs, klv_set< Key > const& rhs )
  {
    if( lhs.size() != rhs.size() )
    {
      return false;
    }

    auto const lhs_values = lhs.fully_sorted();
    auto const rhs_values = rhs.fully_sorted();

    return std::equal( lhs_values.cbegin(), lhs_values.cend(),
                       rhs_values.cbegin(),
                       []( const_iterator lhs, const_iterator rhs ){
                         return *lhs == *rhs;
                       } );
  }

  friend bool
  operator<( klv_set< Key > const& lhs, klv_set< Key > const& rhs )
  {
    if( lhs.size() < rhs.size() )
    {
      return true;
    }
    else if( lhs.size() == rhs.size() )
    {
      auto const lhs_values = lhs.fully_sorted();
      auto const rhs_values = rhs.fully_sorted();
      return std::lexicographical_compare(
        lhs_values.cbegin(), lhs_values.cend(),
        rhs_values.cbegin(), rhs_values.cend(), value_compare );
    }
    return false;
  }

  friend std::ostream&
  operator<<( std::ostream& os, klv_set< Key > const& rhs )
  {
    auto const values = rhs.fully_sorted();
    os << "{ ";

    bool first = true;
    for( auto const pair : values )
    {
      first = first ? false : ( os << ", ", false );
      os << pair.get().first << ": " << pair.get().second;
    }
    os << " }";
    return os;
  }

private:
  // Sort by key, then value.
  static bool
  value_compare( const_iterator lhs, const_iterator rhs )
  {
    if( lhs->first < rhs->first )
    {
      return true;
    }
    else if( lhs->first == rhs->first )
    {
      return lhs->second < rhs->second;
    }
    return false;
  }

  std::multimap< Key, klv_value > m_items;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
