// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV set template class.

#ifndef KWIVER_VITAL_KLV_KLV_SET_H_
#define KWIVER_VITAL_KLV_KLV_SET_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_tag_traits.h>
#include <arrows/klv/klv_value.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <boost/range.hpp>

#include <map>

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
class KWIVER_ALGO_KLV_EXPORT klv_set
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
  std::vector< const_iterator >
  fully_sorted() const
  {
    std::vector< const_iterator > result;
    for( auto it = cbegin(); it != cend(); ++it )
    {
      result.push_back( it );
    }
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
                       []( const_iterator lhs_value, const_iterator rhs_value ){
                         return *lhs_value == *rhs_value;
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
      os << pair->first << ": " << pair->second;
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

namespace klv_detail {

// ----------------------------------------------------------------------------
// Helper class for klv_set_format allowing compile-time lookup of functions
// pertaining to a KLV key type.
template < class Key > class KWIVER_ALGO_KLV_EXPORT key_traits;

// ----------------------------------------------------------------------------
template <>
class KWIVER_ALGO_KLV_EXPORT key_traits< klv_lds_key >
{
public:
  static klv_lds_key
  read_key( klv_read_iter_t& data, size_t max_length )
  {
    return klv_read_lds_key( data, max_length );
  }

  static void
  write_key( klv_lds_key const& key,
             klv_write_iter_t& data, size_t max_length )
  {
    klv_write_lds_key( key, data, max_length );
  }

  static size_t
  length_of_key( klv_lds_key const& key )
  {
    return klv_lds_key_length( key );
  }

  static klv_tag_traits const&
  tag_traits_from_key( klv_tag_traits_lookup const& lookup,
                       klv_lds_key const& key )
  {
    return lookup.by_tag( key );
  }

  static klv_lds_key
  key_from_tag_traits( klv_tag_traits const& trait )
  {
    return trait.tag();
  }
};

// ----------------------------------------------------------------------------
template <>
class KWIVER_ALGO_KLV_EXPORT key_traits< klv_uds_key >
{
public:
  static klv_uds_key
  read_key( klv_read_iter_t& data, size_t max_length )
  {
    return klv_read_uds_key( data, max_length );
  }

  static void
  write_key( klv_uds_key const& key,
             klv_write_iter_t& data, size_t max_length )
  {
    klv_write_uds_key( key, data, max_length );
  }

  static size_t
  length_of_key( klv_uds_key const& key )
  {
    return klv_uds_key_length( key );
  }

  static klv_tag_traits const&
  tag_traits_from_key( klv_tag_traits_lookup const& lookup,
                       klv_uds_key const& key )
  {
    return lookup.by_uds_key( key );
  }

  static klv_uds_key
  key_from_tag_traits( klv_tag_traits const& trait )
  {
    return trait.uds_key();
  }
};

} // namespace klv_detail

// ----------------------------------------------------------------------------
/// Interprets data as a local or universal set.
template < class Key >
class KWIVER_ALGO_KLV_EXPORT klv_set_format
  : public klv_data_format_< klv_set< Key > >
{
public:
  explicit
  klv_set_format( klv_tag_traits_lookup const& traits )
    : klv_data_format_< klv_set< Key > >{ 0 }, m_traits{ traits } {}

  virtual
  ~klv_set_format() = default;

protected:
  using key_traits = klv_detail::key_traits< Key >;

  klv_set< Key >
  read_typed( klv_read_iter_t& data, size_t length ) const override final
  {
    // These help us keep track of how many bytes we have read
    auto const begin = data;
    auto const remaining_length =
      [ & ]() -> size_t {
        return length - std::distance( begin, data );
      };

    klv_set< Key > result;
    while( remaining_length() )
    {
      // Key
      auto const key = key_traits::read_key( data, remaining_length() );

      // Length
      auto const length_of_value =
        klv_read_ber< size_t >( data, remaining_length() );

      // Value
      auto const& traits = key_traits::tag_traits_from_key( m_traits, key );
      auto value = traits.format().read( data, length_of_value );

      result.add( key, std::move( value ) );
    }
    check_tag_counts( result );

    return result;
  }

  void
  write_typed( klv_set< Key > const& klv,
               klv_write_iter_t& data, size_t length ) const override final
  {
    // These help us keep track of how many bytes we have written
    auto const begin = data;
    auto const remaining_length =
      [ & ]() -> size_t {
        return length - std::distance( begin, data );
      };

    check_tag_counts( klv );
    for( auto const& entry : klv )
    {
      auto const& key = entry.first;
      auto const& value = entry.second;
      auto const& traits = key_traits::tag_traits_from_key( m_traits, key );

      // Key
      key_traits::write_key( key, data, remaining_length() );

      // Length
      auto const length_of_value = traits.format().length_of( value );
      klv_write_ber( length_of_value, data, remaining_length() );

      // Value
      traits.format().write( value, data, remaining_length() );
    }
  }

  size_t
  length_of_typed( klv_set< Key > const& value,
                   VITAL_UNUSED size_t length_hint ) const override final
  {
    constexpr size_t initializer = 0;
    auto accumulator =
      [ this ]( size_t total,
                typename klv_set< Key >::value_type const& entry ){
        auto const& key = entry.first;
        auto const& traits = key_traits::tag_traits_from_key( m_traits, key );

        auto const length_of_key = key_traits::length_of_key( key );
        auto const length_of_value = traits.format().length_of( entry.second );
        auto const length_of_length = klv_ber_length( length_of_value );
        return total +
               length_of_key + length_of_length + length_of_value;
      };
    return std::accumulate( value.cbegin(), value.cend(),
                            initializer, accumulator );
  }

  // Print warnings if tags appear too few or too many times in the given set.
  void
  check_tag_counts( klv_set< Key > const& klv ) const
  {
    for( auto const& trait : m_traits )
    {
      auto const count = klv.count( key_traits::key_from_tag_traits( trait ) );
      auto const range = trait.tag_count_range();
      if( !range.is_count_allowed( count ) )
      {
        LOG_WARN( kwiver::vital::get_logger( "klv" ),
                  range.error_message( count ) );
      }
    }
  }

  klv_tag_traits_lookup const& m_traits;
};

// ----------------------------------------------------------------------------
/// KLV local set. Key-value pairs of a format defined by a standard.
using klv_local_set = klv_set< klv_lds_key >;

// ----------------------------------------------------------------------------
/// Interprets data as a KLV local set.
using klv_local_set_format = klv_set_format< klv_lds_key >;

// ----------------------------------------------------------------------------
/// KLV universal set. Key-value pairs of a format defined by a standard.
using klv_universal_set = klv_set< klv_uds_key >;

// ----------------------------------------------------------------------------
/// Interprets data as a KLV universal set.
using klv_universal_set_format = klv_set_format< klv_uds_key >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
