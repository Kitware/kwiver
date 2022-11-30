// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV list format.

#ifndef KWIVER_ARROWS_KLV_KLV_LIST_HPP_
#define KWIVER_ARROWS_KLV_KLV_LIST_HPP_

#include "klv_list.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Format >
template < class... Args >
klv_list_format< Format >
::klv_list_format( Args&&... args )
  : m_format{ std::forward< Args >( args )... }
{}

// ----------------------------------------------------------------------------
template < class Format >
std::string
klv_list_format< Format >
::description() const
{
  return "list of " + m_format.description();
}

// ----------------------------------------------------------------------------
template < class Format >
std::vector< typename Format::data_type >
klv_list_format< Format >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  std::vector< element_t > result;
  while( tracker.remaining() )
  {
    auto element = m_format.read( data, tracker.remaining() );
    result.emplace_back( std::move( element.template get< element_t >() ) );
  }
  return result;
}

// ----------------------------------------------------------------------------
template < class Format >
void
klv_list_format< Format >
::write_typed( std::vector< element_t > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  for( auto const& element : value )
  {
    m_format.write( element, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
template < class Format >
size_t
klv_list_format< Format >
::length_of_typed( std::vector< element_t > const& value ) const
{
  auto const accumulator =
    [ this ]( size_t total, element_t const& element ){
      return total + m_format.length_of( element );
    };
  return std::accumulate(
    value.cbegin(), value.cend(), size_t{ 0 }, accumulator );
}

// ----------------------------------------------------------------------------
template < class Format >
std::ostream&
klv_list_format< Format >
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
