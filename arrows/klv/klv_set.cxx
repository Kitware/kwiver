// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Instantiations of \c klv_set and \c klv_set_format.

#include "klv_set.h"

#include <arrows/klv/klv_1010.h>
#include <arrows/klv/klv_key_traits.h>
#include <arrows/klv/klv_util.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Key >
klv_set< Key >

::klv_set() {}

// ----------------------------------------------------------------------------
template < class Key >
klv_set< Key >

::klv_set( container const& items ) : m_items{ items } {}

// ----------------------------------------------------------------------------
template < class Key >
klv_set< Key >

::klv_set( std::initializer_list< value_type > const& items )
  : m_items{ items } {}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::iterator

klv_set< Key >
::begin()
{
  return m_items.begin();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_iterator

klv_set< Key >
::begin() const
{
  return m_items.cbegin();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_iterator

klv_set< Key >
::cbegin() const
{
  return m_items.cbegin();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::iterator

klv_set< Key >
::end()
{
  return m_items.end();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_iterator

klv_set< Key >
::end() const
{
  return m_items.cend();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_iterator

klv_set< Key >
::cend() const
{
  return m_items.cend();
}

// ----------------------------------------------------------------------------
template < class Key >
bool
klv_set< Key >
::empty() const
{
  return m_items.empty();
}

// ----------------------------------------------------------------------------
template < class Key >
size_t
klv_set< Key >
::size() const
{
  return m_items.size();
}

// ----------------------------------------------------------------------------
template < class Key >
size_t
klv_set< Key >
::count( Key const& key ) const
{
  return m_items.count( key );
}

// ----------------------------------------------------------------------------
template < class Key >
bool
klv_set< Key >
::has( Key const& key ) const
{
  return m_items.count( key );
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set< Key >
::add( Key const& key, klv_value const& datum )
{
  m_items.emplace( key, datum );
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set< Key >
::erase( const_iterator it )
{
  m_items.erase( it );
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set< Key >
::erase( Key const& key )
{
  m_items.erase( key );
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set< Key >
::clear()
{
  m_items.clear();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::iterator

klv_set< Key >
::find( Key const& key )
{
  auto const it = m_items.lower_bound( key );
  if( it != m_items.end() && it->first == key )
  {
    auto const next_it = std::next( it );
    if( next_it == m_items.end() || next_it->first != key )
    {
      return it;
    }
  }
  return m_items.end();
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_iterator

klv_set< Key >
::find( Key const& key ) const
{
  auto const it = m_items.lower_bound( key );
  if( it != m_items.end() && it->first == key )
  {
    auto const next_it = std::next( it );
    if( next_it == m_items.end() || next_it->first != key )
    {
      return it;
    }
  }
  return m_items.cend();
}

// ----------------------------------------------------------------------------
template < class Key >
klv_value&
klv_set< Key >
::at( Key const& key )
{
  auto const it = m_items.lower_bound( key );
  if( it != m_items.end() && it->first == key )
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

// ----------------------------------------------------------------------------
template < class Key >
klv_value const&
klv_set< Key >
::at( Key const& key ) const
{
  auto const it = m_items.lower_bound( key );
  if( it != m_items.cend() && it->first == key )
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

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::range

klv_set< Key >
::all_at( Key const& key )
{
  auto const equal_range = m_items.equal_range( key );
  return { equal_range.first, equal_range.second };
}

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_range

klv_set< Key >
::all_at( Key const& key ) const
{
  auto const equal_range = m_items.equal_range( key );
  return { equal_range.first, equal_range.second };
}

// ----------------------------------------------------------------------------
template < class Key >
std::vector< typename klv_set< Key >::const_iterator >
klv_set< Key >
::fully_sorted() const
{
  std::vector< const_iterator > result;
  for( auto it = cbegin(); it != cend(); ++it )
  {
    result.push_back( it );
  }
  std::sort( result.begin(), result.end(), value_compare );
  return result;
}

// ----------------------------------------------------------------------------
template < class Key >
bool
operator==( klv_set< Key > const& lhs, klv_set< Key > const& rhs )
{
  using const_iterator = typename klv_set< Key >::const_iterator;
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

// ----------------------------------------------------------------------------
template < class Key >
bool
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
      rhs_values.cbegin(), rhs_values.cend(),
      klv_set< Key >::value_compare );
  }
  return false;
}

// ----------------------------------------------------------------------------
template < class Key >
std::ostream&
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

// ----------------------------------------------------------------------------
template < class Key >
bool
klv_set< Key >
::value_compare( const_iterator lhs, const_iterator rhs )
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

// ----------------------------------------------------------------------------
template < class Key >
klv_set_format< Key >
::klv_set_format( klv_tag_traits_lookup const& traits )
  : m_traits( traits )
{}

// ----------------------------------------------------------------------------
template < class Key >
klv_set_format< Key >
::~klv_set_format()
{}

// ----------------------------------------------------------------------------
template < class Key >
klv_set< Key >
klv_set_format< Key >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  using kt = key_traits< Key >;

  auto const tracker = track_it( data, length );

  klv_set< Key > result;
  std::vector< klv_lds_key > history;
  while( tracker.remaining() )
  {
    // Key
    auto const key = kt::read_key( data, tracker.remaining() );

    // Length
    auto const length_of_value =
      klv_read_ber< size_t >( data, tracker.remaining() );

    // Value
    klv_value value;
    auto const& traits = kt::tag_traits_from_key( m_traits, key );

    // Record entries before this one in the SDCC-FLP
    auto const sdcc_format =
      dynamic_cast< klv_1010_sdcc_flp_format const* >( &traits.format() );
    if( sdcc_format )
    {
      auto format = *sdcc_format;
      format.set_preceding( history );
      value = format.read( data, tracker.verify( length_of_value ) );
    }
    else
    {
      value = traits.format().read( data, tracker.verify( length_of_value ) );
    }

    result.add( key, std::move( value ) );

    history.push_back( traits.tag() );
  }
  check_tag_counts( result );

  return result;
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set_format< Key >
::write_typed( klv_set< Key > const& klv,
               klv_write_iter_t& data, size_t length ) const
{
  using kt = key_traits< Key >;

  auto const tracker = track_it( data, length );

  check_tag_counts( klv );

  // Identify which entries need to be before other entries
  std::set< klv_lds_key > held_keys;
  for( auto const& entry : klv )
  {
    if( entry.second.type() == typeid( klv_1010_sdcc_flp ) )
    {
      auto const& sdcc = entry.second.template get< klv_1010_sdcc_flp >();
      for( auto const key : sdcc.members )
      {
        if( held_keys.count( key ) )
        {
          VITAL_THROW( kv::metadata_exception,
                       "Two SDCC-FLPs concern the same item" );
        }
        held_keys.emplace( key );
      }
    }
  }

  // Assemble the proper order of entries
  std::vector< typename klv_set< Key >::const_iterator > entries;
  for( auto it = klv.cbegin(); it != klv.cend(); ++it )
  {
    auto const& trait = kt::tag_traits_from_key( m_traits, it->first );
    if( held_keys.count( trait.tag() ) )
    {
      continue;
    }
    if( it->second.type() == typeid( klv_1010_sdcc_flp ) )
    {
      auto const& sdcc = it->second.template get< klv_1010_sdcc_flp >();
      for( auto const key : sdcc.members )
      {
        auto const jt =
          klv.find( kt::key_from_tag_traits( m_traits.by_tag( key ) ) );
        if( jt == klv.end() ||
            !kt::tag_traits_from_key( m_traits, jt->first ).tag() )
        {
          VITAL_THROW( kv::metadata_exception,
                       "SDCC-FLP concerns non-existent item" );
        }
        entries.emplace_back( jt );
      }
    }
    entries.emplace_back( it );
  }

  // Actually write each entry
  std::vector< klv_lds_key > history;
  for( auto it = entries.begin(); it != entries.end(); ++it )
  {
    auto const& entry = *it;
    auto const& key = entry->first;
    auto const& value = entry->second;
    auto const& traits = kt::tag_traits_from_key( m_traits, key );

    // Key
    kt::write_key( key, data, tracker.remaining() );

    // Length
    auto const length_of_value = traits.format().length_of( value );
    klv_write_ber( length_of_value, data, tracker.remaining() );

    // Value
    auto const sdcc_format =
      dynamic_cast< klv_1010_sdcc_flp_format const* >( &traits.format() );
    if( sdcc_format )
    {
      auto format = *sdcc_format;
      format.set_preceding( history );
      format.write( value, data, tracker.remaining() );
    }
    else
    {
      traits.format().write( value, data, tracker.remaining() );
    }
    history.emplace_back( traits.tag() );
  }
}

// ----------------------------------------------------------------------------
template < class Key >
size_t
klv_set_format< Key >
::length_of_typed( klv_set< Key > const& value ) const
{
  using kt = key_traits< Key >;

  constexpr size_t initializer = 0;
  auto accumulator =
    [ this ]( size_t total, typename klv_set< Key >::value_type const& entry ){
      auto const& key = entry.first;
      auto const& traits = kt::tag_traits_from_key( m_traits, key );

      auto const length_of_key = kt::length_of_key( key );
      auto const length_of_value = traits.format().length_of( entry.second );
      auto const length_of_length = klv_ber_length( length_of_value );
      return total + length_of_key + length_of_length + length_of_value;
    };
  return std::accumulate( value.cbegin(), value.cend(),
                          initializer, accumulator );
}

// ----------------------------------------------------------------------------
template < class Key >
std::ostream&
klv_set_format< Key >
::print_typed( std::ostream& os, klv_set< Key > const& value ) const
{
  using kt = key_traits< Key >;

  auto const values = value.fully_sorted();
  os << "{ ";

  bool first = true;
  for( auto const pair : values )
  {
    auto const& trait = kt::tag_traits_from_key( m_traits, pair->first );
    first = first ? false : ( os << ", ", false );
    os << trait.name() << ": ";
    trait.format().print( os, pair->second );
  }
  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set_format< Key >
::check_tag_counts( klv_set< Key > const& klv ) const
{
  using kt = key_traits< Key >;
  for( auto const& trait : m_traits )
  {
    auto const count = klv.count( kt::key_from_tag_traits( trait ) );
    auto const range = trait.tag_count_range();
    if( !range.is_count_allowed( count ) )
    {
      LOG_WARN(
        kv::get_logger( "klv" ),
        this->description() << ": "
        << "tag `" << trait.name() << "` "
        << "appears " << count << " times; "
        << "expected " << range.description() );
    }
  }
}

// ----------------------------------------------------------------------------
template < class Key >
void
klv_set_format< Key >
::check_set( klv_set< Key > const& klv ) const
{
  check_tag_counts( klv );
}

// ----------------------------------------------------------------------------
#define KLV_INSTANTIATE( Key )                                       \
  template class KWIVER_ALGO_KLV_EXPORT klv_set< Key >;              \
  template class KWIVER_ALGO_KLV_EXPORT klv_set_format< Key >;       \
  template KWIVER_ALGO_KLV_EXPORT bool                               \
  operator==< Key >( klv_set< Key > const&, klv_set< Key > const& ); \
  template KWIVER_ALGO_KLV_EXPORT bool                               \
  operator< < Key >( klv_set< Key > const&, klv_set< Key > const& ); \
  template KWIVER_ALGO_KLV_EXPORT std::ostream&                      \
  operator<< < Key >( std::ostream&, klv_set< Key > const& )

KLV_INSTANTIATE( klv_lds_key );
KLV_INSTANTIATE( klv_uds_key );

} // namespace klv

} // namespace arrows

} // namespace kwiver
