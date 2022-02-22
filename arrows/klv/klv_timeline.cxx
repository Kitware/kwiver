// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of KLV timeline class.

#include "klv_timeline.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
bool
klv_timeline::key_t
::operator<( key_t const& other ) const
{
  if( standard < other.standard )
  {
    return true;
  }
  if( standard > other.standard )
  {
    return false;
  }
  if( tag < other.tag )
  {
    return true;
  }
  if( tag > other.tag )
  {
    return false;
  }
  return index < other.index;
}

// ----------------------------------------------------------------------------
bool
klv_timeline::key_t
::operator==( key_t const& other ) const
{
  return standard == other.standard &&
         tag == other.tag &&
         index == other.index;
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::begin()
{
  return m_map.begin();
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::end()
{
  return m_map.end();
}

// ----------------------------------------------------------------------------
klv_timeline::const_iterator
klv_timeline
::begin() const
{
  return m_map.begin();
}

// ----------------------------------------------------------------------------
klv_timeline::const_iterator
klv_timeline
::end() const
{
  return m_map.end();
}

// ----------------------------------------------------------------------------
klv_timeline::const_iterator
klv_timeline
::cbegin() const
{
  return m_map.cbegin();
}

// ----------------------------------------------------------------------------
klv_timeline::const_iterator
klv_timeline
::cend() const
{
  return m_map.cend();
}

// ----------------------------------------------------------------------------
size_t
klv_timeline
::size() const
{
  return m_map.size();
}

// ----------------------------------------------------------------------------
klv_value
klv_timeline
::at( klv_top_level_tag standard, klv_lds_key tag, uint64_t time ) const
{
  auto const matches = find_all( standard, tag );
  if( matches.begin() == matches.end() )
  {
    return {};
  }

  klv_value const* result = nullptr;
  for( auto const& match : matches )
  {
    auto const it = match.second.find( time );
    if( it != match.second.end() )
    {
      if( result )
      {
        throw std::logic_error(
                "klv_timeline.at(): more than one entry found" );
      }
      result = &it->value;
    }
  }

  return result ? *result : klv_value{};
}

// ----------------------------------------------------------------------------
klv_value
klv_timeline
::at( klv_top_level_tag standard, klv_lds_key tag, uint64_t index,
      uint64_t time ) const
{
  auto const it = find( standard, tag, index );
  if( it == end() )
  {
    return {};
  }

  auto const inner_it = it->second.find( time );
  return ( inner_it != it->second.end() ) ? inner_it->value : klv_value{};
}

// ----------------------------------------------------------------------------
std::vector< klv_value >
klv_timeline
::all_at( klv_top_level_tag standard, klv_lds_key tag, uint64_t time ) const
{
  std::vector< klv_value > results;

  for( auto const& entry : find_all( standard, tag ) )
  {
    auto const it = entry.second.find( time );
    if( it != entry.second.end() )
    {
      results.emplace_back( it->value );
    }
  }

  return results;
}

// ----------------------------------------------------------------------------
klv_timeline::range
klv_timeline
::find_all( klv_top_level_tag standard )
{
  auto const max_tag = std::numeric_limits< klv_lds_key >::max();
  return { m_map.lower_bound( { standard, 0, 0 } ),
           m_map.upper_bound( { standard, max_tag, UINT64_MAX } ) };
}

// ----------------------------------------------------------------------------
klv_timeline::const_range
klv_timeline
::find_all( klv_top_level_tag standard ) const
{
  auto const max_tag = std::numeric_limits< klv_lds_key >::max();
  return { m_map.lower_bound( { standard, 0, 0 } ),
           m_map.upper_bound( { standard, max_tag, UINT64_MAX } ) };
}

// ----------------------------------------------------------------------------
klv_timeline::range
klv_timeline
::find_all( klv_top_level_tag standard, klv_lds_key tag )
{
  return { m_map.lower_bound( { standard, tag, 0 } ),
           m_map.upper_bound( { standard, tag, UINT64_MAX } ) };
}

// ----------------------------------------------------------------------------
klv_timeline::const_range
klv_timeline
::find_all( klv_top_level_tag standard, klv_lds_key tag ) const
{
  return { m_map.lower_bound( { standard, tag, 0 } ),
           m_map.upper_bound( { standard, tag, UINT64_MAX } ) };
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::find( klv_top_level_tag standard, klv_lds_key tag, uint64_t index )
{
  return m_map.find( { standard, tag, index } );
}

// ----------------------------------------------------------------------------
klv_timeline::const_iterator
klv_timeline
::find( klv_top_level_tag standard, klv_lds_key tag, uint64_t index ) const
{
  return m_map.find( { standard, tag, index } );
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::find( klv_top_level_tag standard, klv_lds_key tag )
{
  auto const it_range = find_all( standard, tag );
  switch( std::distance( it_range.begin(), it_range.end() ) )
  {
    case 0:
      return m_map.end();
    case 1:
      return it_range.begin();
    default:
      throw std::logic_error( "klv_timeline.find(): multiple entries found" );
  }
}

// ----------------------------------------------------------------------------
klv_timeline::const_iterator
klv_timeline
::find( klv_top_level_tag standard, klv_lds_key tag ) const
{
  auto const it_range = find_all( standard, tag );
  switch( std::distance( it_range.begin(), it_range.end() ) )
  {
    case 0:
      return m_map.end();
    case 1:
      return it_range.begin();
    default:
      throw std::logic_error( "klv_timeline.find(): multiple entries found" );
  }
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::insert( klv_top_level_tag standard, klv_lds_key tag )
{
  auto const it_range = find_all( standard, tag );

  // Find unused index
  auto index = 0;
  if( it_range.begin() != it_range.end() )
  {
    auto const last_index = std::prev( it_range.end() )->first.index;
    if( last_index == UINT64_MAX )
    {
      // Integer overflow, have to find by linear iteration
      // Unlikely this will ever happen
      while( find( standard, tag, index ) != end() )
      {
        ++index;
      }
    }
    else
    {
      // Usual case - one greater than the greatest index
      index = last_index + 1;
    }
  }
  return insert_or_find( standard, tag, index );
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::insert_or_find( klv_top_level_tag standard, klv_lds_key tag )
{
  auto it_range = find_all( standard, tag );
  switch( std::distance( it_range.begin(), it_range.end() ) )
  {
    case 0:
      return insert_or_find( standard, tag, 0 );
    case 1:
      return it_range.begin();
    default:
      throw std::logic_error(
              "klv_timeline.insert_or_find(): multiple entries found" );
  }
}

// ----------------------------------------------------------------------------
klv_timeline::iterator
klv_timeline
::insert_or_find( klv_top_level_tag standard, klv_lds_key tag, uint64_t index )
{
  return m_map.emplace( key_t{ standard, tag, index },
                        interval_map_t{} ).first;
}

// ----------------------------------------------------------------------------
void
klv_timeline
::erase( const_iterator it )
{
  m_map.erase( it );
}

// ----------------------------------------------------------------------------
void
klv_timeline
::erase( const_range range )
{
  m_map.erase( range.begin(), range.end() );
}

// ----------------------------------------------------------------------------
bool
operator==( klv_timeline const& lhs, klv_timeline const& rhs )
{
  return lhs.size() == rhs.size() &&
         std::equal( lhs.begin(), lhs.end(), rhs.begin() );
}

// ----------------------------------------------------------------------------
bool
operator!=( klv_timeline const& lhs, klv_timeline const& rhs )
{
  return !( lhs == rhs );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            typename klv_timeline::key_t const& rhs )
{
  auto const& trait = klv_lookup_packet_traits().by_tag( rhs.standard );
  auto const lookup = trait.subtag_lookup();
  auto const& standard_name = trait.name();
  auto const tag_name = lookup
                        ? lookup->by_tag( rhs.tag ).name()
                        : std::to_string( rhs.tag );
  os << "{ " << "key: { " << "standard: " << standard_name << ", "
     << "tag: " << tag_name << ", index: " << rhs.index << " }";
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_timeline const& rhs )
{
  os << "{ ";

  auto first_outer = true;
  for( auto const& entry : rhs )
  {
    if( first_outer )
    {
      first_outer = false;
    }
    else
    {
      os << ", ";
    }
    os << entry.first << ", value: { ";

    auto first_inner = true;
    for( auto const& subentry : entry.second )
    {
      if( first_inner )
      {
        first_inner = false;
      }
      else
      {
        os << ", ";
      }
      os << "{ "
         << "interval: { "
         << subentry.key_interval.lower() << ", "
         << subentry.key_interval.upper() << " }, "
         << "value: " << subentry.value
         << " }";
    }
    os << " } }";
  }
  os << " }";

  return os;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
