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
klv_series_format< Format >
::klv_series_format( Args&&... args )
  : m_format{ std::forward< Args >( args )... }
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
std::vector< typename Format::data_type >
klv_series_format< Format >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  std::vector< element_t > result;
  while( tracker.remaining() )
  {
    auto const length_of_entry =
      klv_read_ber< size_t >( data, tracker.remaining() );
    result.emplace_back(
      m_format.read( data, tracker.verify( length_of_entry ) )
      .template get< element_t >() );
  }

  return result;
}

// ----------------------------------------------------------------------------
template < class Format >
void
klv_series_format< Format >
::write_typed( std::vector< element_t > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  for( auto const& entry : value )
  {
    klv_write_ber( m_format.length_of( entry ), data, tracker.remaining() );
    m_format.write( entry, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
template < class Format >
size_t
klv_series_format< Format >
::length_of_typed( std::vector< element_t > const& value ) const
{
  size_t result = 0;
  for( auto const& entry : value )
  {
    auto const length_of_entry = m_format.length_of( entry );
    auto const length_of_length_of_entry = klv_ber_length( length_of_entry );
    result += length_of_entry + length_of_length_of_entry;
  }
  return result;
}

// ----------------------------------------------------------------------------
template < class Format >
std::ostream&
klv_series_format< Format >
::print_typed( std::ostream& os,
               std::vector< element_t > const& value ) const
{
  os << "{ ";
  auto first = true;
  for( auto const& item : value )
  {
    first = first ? false : ( os << ", ", false );
    m_format.print( os, item );
  }
  os << " }";
  return os;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
