// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV series format's templated functions.

#include "klv_series.h"

#ifndef KWIVER_ARROWS_KLV_KLV_SERIES_HPP_
# define KWIVER_ARROWS_KLV_KLV_SERIES_HPP_

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Format >
template < class... Args >
klv_series< Format >

::klv_series( container_t const& elements, Args&&... args )
  : m_format{ std::forward< Args >( args )... },
    m_elements( elements )
{}

// ----------------------------------------------------------------------------
template < class Format >
typename klv_series< Format >::container_t&
klv_series< Format >
::operator*()
{
  return m_elements;
}

// ----------------------------------------------------------------------------
template < class Format >
typename klv_series< Format >::container_t const&
klv_series< Format >
::operator*() const
{
  return m_elements;
}

// ----------------------------------------------------------------------------
template < class Format >
typename klv_series< Format >::container_t*
klv_series< Format >
::operator->()
{
  return &m_elements;
}

// ----------------------------------------------------------------------------
template < class Format >
typename klv_series< Format >::container_t const*
klv_series< Format >
::operator->() const
{
  return &m_elements;
}

// ----------------------------------------------------------------------------
template < class Format >
Format const&
klv_series< Format >
::format() const
{
  return m_format;
}

// ----------------------------------------------------------------------------
template < class Format >
std::ostream&
operator<<( std::ostream& os, klv_series< Format > const& value )
{
  os << "{ ";

  auto first = true;
  for( auto const& entry : *value )
  {
    first = first ? false : ( os << ", ", false );
    value.format().print( os, entry );
  }
  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
template < class Format >
bool
operator==( klv_series< Format > const& lhs, klv_series< Format > const& rhs )
{
  return *lhs == *rhs;
}

// ----------------------------------------------------------------------------
template < class Format >
bool
operator<( klv_series< Format > const& lhs, klv_series< Format > const& rhs )
{
  return *lhs < *rhs;
}

// ----------------------------------------------------------------------------
template < class Format >
template < class... Args >
klv_series_format< Format >

::klv_series_format( Args&&... args )
  : klv_data_format_< klv_series_t >{ 0 },
    m_format{ std::forward< Args >( args )... }
{}

// ----------------------------------------------------------------------------
template < class Format >
std::string
klv_series_format< Format >
::description() const
{
  return "series of " + m_format.description();
}

// ----------------------------------------------------------------------------
template < class Format >
klv_series< Format >
klv_series_format< Format >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  typename klv_series< Format >::container_t result;
  while( tracker.remaining() )
  {
    auto const length_of_entry =
      klv_read_ber< size_t >( data, tracker.remaining() );
    result.emplace_back(
      m_format.read( data, tracker.verify( length_of_entry ) ) );
  }

  return klv_series< Format >{ result, m_format };
}

// ----------------------------------------------------------------------------
template < class Format >
void
klv_series_format< Format >
::write_typed( klv_series_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  for( auto const& entry : *value )
  {
    klv_write_ber( m_format.length_of( entry ), data, tracker.remaining() );
    m_format.write( entry, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
template < class Format >
size_t
klv_series_format< Format >
::length_of_typed( klv_series_t const& value ) const
{
  size_t result = 0;
  for( auto const& entry : *value )
  {
    auto const length_of_entry = value.format().length_of( entry );
    auto const length_of_length_of_entry = klv_ber_length( length_of_entry );
    result += length_of_entry + length_of_length_of_entry;
  }
  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
