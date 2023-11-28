// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of KLV update tracker.

#include "klv_update_tracker.h"

#include <arrows/klv/klv_key_traits.h>
#include <arrows/klv/klv_util.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template< class Key >
bool
klv_update_tracker< Key >::key_t
::operator<( klv_update_tracker< Key >::key_t const& other ) const
{
  return std::tie( standard, tag ) < std::tie( other.standard, other.tag );
}

// ----------------------------------------------------------------------------
template< class Key >
bool
klv_update_tracker< Key >::value_t
::operator<( klv_update_tracker< Key >::value_t const& other ) const
{
  return
    std::tie( timestamp, value ) < std::tie( other.timestamp, other.value );
}

// ----------------------------------------------------------------------------
template< class Key >
klv_update_tracker< Key >
::klv_update_tracker() : m_map{}
{}

// ----------------------------------------------------------------------------
template< class Key >
typename klv_update_tracker< Key >::value_t const*
klv_update_tracker< Key >
::at( key_t const& key ) const
{
  auto const it = m_map.find( key );
  return ( it == m_map.end() ) ? nullptr : &it->second;
}

// ----------------------------------------------------------------------------
template< class Key >
bool
klv_update_tracker< Key >
::has_changed( klv_set< Key > const& set, key_t const& key ) const
{
  auto const old_value = at( key );
  if( !old_value )
  {
    return !set.count( key.tag );
  }

  std::set< klv_value > new_value;
  for( auto const entry : set.all_at( key.tag ) )
  {
    new_value.emplace( entry.second );
  }
  return new_value != old_value->value;
}

// ----------------------------------------------------------------------------
template< class Key >
bool
klv_update_tracker< Key >
::update( klv_set< Key > const& set, key_t const& key, uint64_t timestamp )
{
  auto const tag_range = set.all_at( key.tag );
  auto const it = m_map.find( key );
  if( tag_range.empty() )
  {
    if( it == m_map.end() )
    {
      return false;
    }
    else
    {
      m_map.erase( it );
      return true;
    }
  }
  else
  {
    value_t new_value = { timestamp, std::set< klv_value >{} };
    for( auto const entry : set.all_at( key.tag ) )
    {
      new_value.value.emplace( entry.second );
    }

    auto const insertion = m_map.emplace( key, new_value );
    if( insertion.second )
    {
      return true;
    }
    if( insertion.first->second.value != new_value.value )
    {
      insertion.first->second = std::move( new_value );
      return true;
    }
    return false;
  }
}

// ----------------------------------------------------------------------------
template< class Key >
void
klv_update_tracker< Key >
::prune( klv_set< Key >& set, klv_update_intervals const& intervals,
         klv_top_level_tag standard, uint64_t timestamp )
{
  using kt = key_traits< Key >;
  auto const traits =
    klv_lookup_packet_traits().by_tag( standard ).subtag_lookup();
  if( !traits )
  {
    VITAL_THROW(
      kv::invalid_value, "Standard does not have tag traits implemented" );
  }
  for( auto it = set.begin(); it != set.end();)
  {
    auto const next_it = std::next( it );
    auto const update_interval =
      intervals.at(
        { standard, kt::tag_traits_from_key( *traits, it->first ).tag() } );
    auto const last_update = at( { standard, it->first } );
    auto const next_update_time =
      last_update ? last_update->timestamp + update_interval : 0;
    if( !update( set, { standard, it->first }, timestamp ) &&
        timestamp < next_update_time )
    {
      set.erase( it );
    }
    it = next_it;
  }
}

// ----------------------------------------------------------------------------
template class klv_update_tracker< klv_lds_key >;
template class klv_update_tracker< klv_uds_key >;

} // namespace klv

} // namespace arrows

} // namespace kwiver
