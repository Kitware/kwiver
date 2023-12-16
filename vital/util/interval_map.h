// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Definition of interval_map template class.

#ifndef KWIVER_VITAL_UTIL_INTERVAL_MAP_H_
#define KWIVER_VITAL_UTIL_INTERVAL_MAP_H_

#include <vital/range/iterator_range.h>
#include <vital/util/interval.h>

#include <initializer_list>
#include <iterator>
#include <map>
#include <optional>
#include <vector>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
template < class KeyT, class ValueT >
struct interval_map_entry_t
{
  interval< KeyT > const key_interval;
  ValueT value;
};

// ----------------------------------------------------------------------------
/// A set of sorted, disjoint, half-open intervals, each mapped to a value.
///
/// Provides lookup, insertion, and deletion in sub-linear time. Maintains the
/// simplest representation of the data, with no entries of zero length.
/// Multiple contiguous entries with the same value are always combined into
/// a single entry.
template < class KeyT, class ValueT >
class interval_map
{
public:
  using interval_t = interval< KeyT >;
  using interval_map_t = interval_map< KeyT, ValueT >;
  using entry_t = interval_map_entry_t< KeyT, ValueT >;
  using container_t = std::map< KeyT, entry_t >;
  using container_iterator = typename container_t::iterator;
  using container_const_iterator = typename container_t::const_iterator;

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = entry_t;
    using pointer = entry_t*;
    using reference = entry_t&;
    using iterator_category = std::bidirectional_iterator_tag;

    reference
    operator*() const { return m_it->second; }
    pointer
    operator->() const { return &m_it->second; }

    iterator&
    operator++() { ++m_it; return *this; }
    iterator&
    operator--() { --m_it; return *this; }

    bool
    operator==( iterator const& other ) const
    {
      return m_it == other.m_it;
    }

    bool
    operator!=( iterator const& other ) const
    {
      return m_it != other.m_it;
    }

  private:
    friend class interval_map;
    friend class const_iterator;

    iterator( container_iterator const& it ) : m_it{ it } {}

    container_iterator m_it;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = entry_t const;
    using pointer = entry_t const*;
    using reference = entry_t const&;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator( iterator const& other ) : m_it{ other.m_it } {}

    reference
    operator*() const { return m_it->second; }
    pointer
    operator->() const { return &m_it->second; }

    const_iterator&
    operator++() { ++m_it; return *this; }
    const_iterator&
    operator--() { --m_it; return *this; }

    bool
    operator==( const_iterator const& other ) const
    {
      return m_it == other.m_it;
    }

    bool
    operator!=( const_iterator const& other ) const
    {
      return m_it != other.m_it;
    }

  private:
    friend class interval_map;

    const_iterator( container_const_iterator const& it ) : m_it{ it } {}

    container_const_iterator m_it;
  };

  using range_t = range::iterator_range< iterator >;
  using const_range_t = range::iterator_range< const_iterator >;

  interval_map() : m_map{} {}

  template < class Iterator >
  interval_map( Iterator begin, Iterator end ) : m_map{}
  {
    for( auto it = begin; it != end; ++it )
    {
      if( !empty( it->key_interval ) )
      {
        throw std::invalid_argument(
                "interval_map: cannot construct from overlapping intervals" );
      }
      set( *it );
    }
  }

  interval_map( std::initializer_list< entry_t > entries ) : m_map{}
  {
    for( auto const& entry : entries )
    {
      if( !empty( entry.key_interval ) )
      {
        throw std::invalid_argument(
                "interval_map: cannot construct from overlapping intervals" );
      }
      set( entry );
    }
  }

  iterator begin() { return m_map.begin(); }
  iterator end() { return m_map.end(); }

  const_iterator
  begin() const { return m_map.cbegin(); }
  const_iterator
  end() const { return m_map.cend(); }

  const_iterator
  cbegin() const { return m_map.cbegin(); }
  const_iterator
  cend() const { return m_map.cend(); }

  /// Return true if this map has no entries.
  bool
  empty() const
  {
    return m_map.empty();
  }

  /// Return true if there is no entry at any point in the given interval.
  bool
  empty( interval_t const& key_interval ) const
  {
    auto const result = find( key_interval );
    return result.begin() == result.end();
  }

  /// Return the number of entries in this map.
  size_t
  size() const
  {
    return m_map.size();
  }

  /// Return an iterator to the entry containing \p key, or the end iterator on
  /// failure.
  ///
  /// Execution time is O(log n).
  iterator
  find( KeyT key )
  {
    assert_not_nan( key );

    auto const it = upper_upper_bound( key );
    return ( it != m_map.end() && it->second.key_interval.lower() <= key )
           ? it
           : m_map.end();
  }

  /// \copydoc iterator find( KeyT )
  const_iterator
  find( KeyT key ) const
  {
    assert_not_nan( key );

    auto const it = upper_upper_bound( key );
    return ( it != m_map.end() && it->second.key_interval.lower() <= key )
           ? it
           : m_map.end();
  }

  /// Return the range of all entries intersecting \p key_interval.
  ///
  /// Execution time is O(log n).
  range_t
  find( interval_t key_interval )
  {
    auto const begin_it = upper_upper_bound( key_interval.lower() );
    auto const end_it = lower_lower_bound( key_interval.upper() );

    return { begin_it, end_it };
  }

  /// \copydoc range_t find( interval_t )
  const_range_t
  find( interval_t key_interval ) const
  {
    auto const begin_it = upper_upper_bound( key_interval.lower() );
    auto const end_it = lower_lower_bound( key_interval.upper() );

    return { begin_it, end_it };
  }

  /// Return the value, if any, mapped to the given point in key.
  ///
  /// Execution time is O(log n).
  std::optional< ValueT >
  at( KeyT key ) const
  {
    auto const it = find( key );
    return ( it !=
             cend() ) ? std::optional< ValueT >{ it->value } : std::nullopt;
  }

  /// Set the value of the given interval to the given value.
  ///
  /// Only the given interval is affected, though existing entries may be
  /// modified to produce the simplest representation of the result of the
  /// operation. Execution time is O(k + log n), where k is the number of
  /// existing entries inside the given entry.
  void
  set( interval_t const& key_interval, ValueT const& value )
  {
    set( { key_interval, value } );
  }

  /// \copydoc void set( interval_t const&, ValueT const& )
  void
  set( entry_t const& entry )
  {
    // Quick exit
    if( entry.key_interval.lower() == entry.key_interval.upper() )
    {
      return;
    }

    // Remove existing values
    erase( entry.key_interval );

    // Find adjacent entries
    auto const lower_it = upper_find( entry.key_interval.lower() );
    auto const upper_it = lower_find( entry.key_interval.upper() );

    // Merge entries that are adjacent and equivalent
    auto key_interval = entry.key_interval;
    if( lower_it != m_map.end() && entry.value == lower_it->second.value )
    {
      key_interval.encompass( lower_it->second.key_interval.lower() );
      erase( lower_it->second );
    }
    if( upper_it != m_map.end() && entry.value == upper_it->second.value )
    {
      key_interval.encompass( upper_it->second.key_interval.upper() );
      erase( upper_it->second );
    }

    // Insert possibly-merged superentry
    insert( { key_interval, entry.value } );
  }

  /// Set the value of the given interval to the given value, but do not
  /// override any existing values.
  ///
  /// \see void set( interval_t const&, ValueT const& )
  void
  weak_set( interval_t const& key_interval, ValueT const& value )
  {
    weak_set( { key_interval, value } );
  }

  /// \copydoc void weak_set( interval_t const&, ValueT const& )
  void
  weak_set( entry_t const& entry )
  {
    // Identify the existing entries we have to weave between
    auto const existing_entries = find( entry.key_interval );

    // Simple case - nothing to weave between
    if( existing_entries.empty() )
    {
      set( entry );
      return;
    }

    // We want to collect all the intervals we need, in order to avoid dealing
    // with invalidating iterators during the actual value-setting phase. This
    // is a small sacrifice of performance for the sake of cleanliness, since
    // it possible to being the complexity down from O( k log n ) to O( k + log
    // n ) if we're willing to write a bit messier code - just a note in the
    // very unlikely event this becomes a bottleneck in the future.
    std::vector< interval_t > intervals;

    // Find the first interval
    intervals.emplace_back(
        entry.key_interval.lower(),
        std::max( existing_entries.begin()->key_interval.lower(),
                  entry.key_interval.lower() ) );

    // Find the rest of the intervals
    for( auto it = existing_entries.begin();
         it != existing_entries.end(); ++it )
    {
      auto next_key = entry.key_interval.upper();
      if( std::next( it ) != m_map.end() )
      {
        next_key =
          std::min( next_key, std::next( it )->key_interval.lower() );
      }

      auto const curr_key = std::min( next_key, it->key_interval.upper() );
      intervals.emplace_back( curr_key, next_key );
    }

    // Actually set all the intervals to the given value
    for( auto const& interval : intervals )
    {
      set( interval, entry.value );
    }
  }

  /// Remove the entry pointed to by \p it.
  ///
  /// Execution time is O(log n).
  void
  erase( const_iterator const& it )
  {
    erase( *it );
  }

  /// Remove the entries between \p begin_it and \p end_it.
  ///
  /// Execution time is O(k + log n), where k is the number of entries erased.
  void
  erase( const_iterator const& begin_it, const_iterator const& end_it )
  {
    m_map.erase( begin_it.m_it, end_it.m_it );
  }

  /// Remove all entries from the given interval.
  ///
  /// Only the given interval is affected, though existing entries may be
  /// modified to produce the simplest representation of the result of the
  /// operation. Execution time is O(k + log n), where k is the number of
  /// existing entries inside the given entry.
  void
  erase( interval_t const& key_interval )
  {
    auto const find_result = find( key_interval );

    // Quick exit to guarantee edge iterator safety
    if( find_result.begin() == find_result.end() )
    {
      return;
    }

    // Copy edge entries
    auto lower_entry = *find_result.begin();
    auto upper_entry = *std::prev( find_result.end() );

    // Remove every entry inside key_interval
    erase( find_result.begin(), find_result.end() );

    // Re-insert what is left of edge entries
    if( lower_entry.key_interval.lower() < key_interval.lower() )
    {
      insert( { { lower_entry.key_interval.lower(),
                key_interval.lower() }, lower_entry.value } );
    }
    if( key_interval.upper() < upper_entry.key_interval.upper() )
    {
      insert( { { key_interval.upper(),
                upper_entry.key_interval.upper() }, upper_entry.value } );
    }
  }

  /// Remove all entries from this map.
  void
  clear()
  {
    m_map.clear();
  }

private:
  // Performs no overlap checks
  void
  insert( entry_t const& entry )
  {
    if( entry.key_interval.lower() == entry.key_interval.upper() )
    {
      return;
    }
    if( !m_map.emplace( entry.key_interval.lower(), entry ).second )
    {
      throw std::logic_error(
              "interval_map.insert(): inserting interval failed" );
    }
  }

  void
  erase( entry_t const& entry )
  {
    if( !m_map.erase( entry.key_interval.lower() ) )
    {
      throw std::logic_error(
              "interval_map.erase(): erasing interval failed" );
    }
  }

  static void
  assert_not_nan( KeyT value )
  {
    if( kwiver::vital::isnan( value ) )
    {
      throw std::invalid_argument( "interval_map: cannot accept NaN value" );
    }
  }

  // Nomenclature: upper_lower_bound is the lower_bound evaluated on the upper
  // edges of the intervals

  container_iterator
  lower_lower_bound( KeyT value )
  {
    return m_map.lower_bound( value );
  }

  container_const_iterator
  lower_lower_bound( KeyT value ) const
  {
    return m_map.lower_bound( value );
  }

  container_iterator
  lower_upper_bound( KeyT value )
  {
    return m_map.upper_bound( value );
  }

  container_const_iterator
  lower_upper_bound( KeyT value ) const
  {
    return m_map.upper_bound( value );
  }

  container_iterator
  upper_lower_bound( KeyT value )
  {
    auto const it = lower_lower_bound( value );
    return ( it != m_map.begin() &&
             std::prev( it )->second.key_interval.upper() >= value )
           ? std::prev( it )
           : it;
  }

  container_const_iterator
  upper_lower_bound( KeyT value ) const
  {
    auto const it = lower_lower_bound( value );
    return ( it != m_map.begin() &&
             std::prev( it )->second.key_interval.upper() >= value )
           ? std::prev( it )
           : it;
  }

  container_iterator
  upper_upper_bound( KeyT value )
  {
    auto const it = upper_lower_bound( value );
    return ( it != m_map.end() && it->second.key_interval.upper() == value )
           ? std::next( it )
           : it;
  }

  container_const_iterator
  upper_upper_bound( KeyT value ) const
  {
    auto const it = upper_lower_bound( value );
    return ( it != m_map.end() && it->second.key_interval.upper() == value )
           ? std::next( it )
           : it;
  }

  container_iterator
  lower_find( KeyT value )
  {
    return m_map.find( value );
  }

  container_const_iterator
  lower_find( KeyT value ) const
  {
    return m_map.find( value );
  }

  container_iterator
  upper_find( KeyT value )
  {
    auto const it = m_map.lower_bound( value );
    return ( it != m_map.begin() &&
             std::prev( it )->second.key_interval.upper() == value )
           ? std::prev( it )
           : m_map.end();
  }

  container_const_iterator
  upper_find( KeyT value ) const
  {
    auto const it = m_map.lower_bound( value );
    return ( it != m_map.begin() &&
             std::prev( it )->second.key_interval.upper() == value )
           ? std::prev( it )
           : m_map.end();
  }

  // Maps lower end of interval to entry
  std::map< KeyT, entry_t > m_map;
};

// ----------------------------------------------------------------------------
template < class KeyT, class ValueT >
bool
operator==( interval_map_entry_t< KeyT, ValueT > const& lhs,
            interval_map_entry_t< KeyT, ValueT > const& rhs )
{
  return lhs.key_interval == rhs.key_interval && lhs.value == rhs.value;
}

// ----------------------------------------------------------------------------
template < class KeyT, class ValueT >
bool
operator!=( interval_map_entry_t< KeyT, ValueT > const& lhs,
            interval_map_entry_t< KeyT, ValueT > const& rhs )
{
  return !( lhs == rhs );
}

// ----------------------------------------------------------------------------
template < class KeyT, class ValueT >
bool
operator==( interval_map< KeyT, ValueT > const& lhs,
            interval_map< KeyT, ValueT > const& rhs )
{
  return lhs.size() == rhs.size() &&
         std::equal( lhs.cbegin(), lhs.cend(), rhs.cbegin() );
}

// ----------------------------------------------------------------------------
template < class KeyT, class ValueT >
bool
operator!=( interval_map< KeyT, ValueT > const& lhs,
            interval_map< KeyT, ValueT > const& rhs )
{
  return !( lhs == rhs );
}

} // namespace vital

} // namespace kwiver

#endif
