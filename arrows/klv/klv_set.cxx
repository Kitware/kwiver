// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Instantiations of \c klv_set and \c klv_set_format.

#include "klv_set.h"

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

// ----------------------------------------------------------------------------
template < class Key >
typename klv_set< Key >::const_iterator

klv_set< Key >
::find( Key const& key ) const
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

// ----------------------------------------------------------------------------
template < class Key >
klv_value&
klv_set< Key >
::at( Key const& key )
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

// ----------------------------------------------------------------------------
template < class Key >
klv_value const&
klv_set< Key >
::at( Key const& key ) const
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
#define KLV_INSTANTIATE( Key )                                 \
  template class KWIVER_ALGO_KLV_EXPORT klv_set< Key >;        \
  template class KWIVER_ALGO_KLV_EXPORT klv_set_format< Key >; \
  template bool operator== < Key >( klv_set< Key > const&,     \
                                    klv_set< Key > const& );   \
  template bool operator< < Key >( klv_set< Key > const&,      \
                                   klv_set< Key > const& );    \
  template std::ostream& operator<< < Key >( std::ostream&,    \
                                             klv_set< Key > const& )

KLV_INSTANTIATE( klv_lds_key );
KLV_INSTANTIATE( klv_uds_key );

} // namespace klv

} // namespace arrows

} // namespace kwiver
