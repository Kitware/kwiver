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

} // namespace klv

} // namespace arrows

} // namespace kwiver
