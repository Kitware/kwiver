// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1303 parser's templated functions.

#ifndef KWIVER_ARROWS_KLV_KLV_1303_HPP_
#define KWIVER_ARROWS_KLV_KLV_1303_HPP_

#include "klv_1303.h"

#include <arrows/klv/klv_util.h>

#include <vital/range/iota.h>

#include <numeric>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
DEFINE_TEMPLATE_CMP(
  klv_1303_mdap< T >,
  &klv_1303_mdap< T >::sizes,
  &klv_1303_mdap< T >::elements )

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, klv_1303_mdap< T > const& value )
{
  return os << "{ "
            << "sizes: " << value.sizes << ", "
            << "elements: " << value.elements
            << " }";
}

// ----------------------------------------------------------------------------
template < class Format >
template < class... Args >
klv_1303_mdap_format< Format >
::klv_1303_mdap_format( Args&&... args )
  : m_format{ std::forward< Args >( args )... }
{}

// ----------------------------------------------------------------------------
template < class Format >
std::string
klv_1303_mdap_format< Format >
::description() const
{
  return "MDAP/MDARRAY of " + this->m_length_constraints.description();
}

// ----------------------------------------------------------------------------
template < class Format >
klv_1303_mdap< typename Format::data_type >
klv_1303_mdap_format< Format >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  mdap_t result;

  // Number of dimensions
  result.sizes.resize(
      klv_read_ber_oid< size_t >( data, tracker.remaining() ) );
  if( !result.sizes.size() )
  {
    VITAL_THROW( kv::metadata_exception,
                 "MDAP: number of dimensions must be positive." );
  }

  // Dimension sizes
  for( auto& size : result.sizes )
  {
    size = klv_read_ber_oid< size_t >( data, tracker.remaining() );
    if( !size )
    {
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: each dimension's size must be positive" );
    }
  }

  // Element size
  result.element_size =
    klv_read_ber_oid< size_t >( data, tracker.remaining() );

  // Array processing algorithm
  result.apa = klv_1303_apa_format{}.read_( data, tracker.verify( 1 ) );

  // Array processing algorithm parameters
  std::unique_ptr< klv_data_format > format;
  auto const length_of_array =
    std::accumulate( result.sizes.begin(), result.sizes.end(),
                     size_t{ 1 }, std::multiplies< size_t >{} );
  switch( result.apa )
  {
    case KLV_1303_APA_IMAP:
    {
      result.apa_params_length =
        tracker.remaining() - length_of_array * result.element_size;

      auto const param_length = result.apa_params_length / 2;
      auto const minimum =
        klv_read_float( data, tracker.verify( param_length ) );
      auto const maximum =
        klv_read_float( data, tracker.verify( param_length ) );
      result.imap_params = kv::interval< double >{ minimum, maximum };

      using format_t = klv_lengthless_format< klv_imap_format >;
      format.reset( new format_t{ *result.imap_params, result.element_size } );

      break;
    }
    case KLV_1303_APA_NATURAL:
      result.apa_params_length = 0;
      format.reset( new Format{ m_format } );
      format->set_length_constraints( result.element_size );
      break;
    case KLV_1303_APA_BOOLEAN:
    case KLV_1303_APA_UINT:
    case KLV_1303_APA_RLE:
    default:
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: ADA value not supported" );
      break;
  }

  // Array data elements
  result.elements.reserve( length_of_array );
  for( size_t i = 0; i < length_of_array; ++i )
  {
    auto value = format->read( data, tracker.verify( result.element_size ) );
    result.elements.emplace_back(
        std::move( value.template get< element_t >() ) );
  }

  return result;
}

// ----------------------------------------------------------------------------
template < class Format >
void
klv_1303_mdap_format< Format >
::write_typed( mdap_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Number of dimensions
  auto const dimension_count = value.sizes.size();
  if( !dimension_count )
  {
    VITAL_THROW( kv::metadata_exception,
                 "MDAP: number of dimensions must be positive." );
  }
  klv_write_ber_oid( dimension_count, data, tracker.remaining() );

  // Dimension sizes
  for( auto const size : value.sizes )
  {
    if( !size )
    {
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: each dimension's size must be positive" );
    }
    klv_write_ber_oid( size, data, tracker.remaining() );
  }

  // Element size
  klv_write_ber_oid( value.element_size, data, tracker.remaining() );

  // Array processing algorithm
  klv_1303_apa_format{}.write_( value.apa, data, tracker.remaining() );

  // Array processing algorithm parameters
  std::unique_ptr< klv_data_format > format;
  switch( value.apa )
  {
    case KLV_1303_APA_IMAP:
    {
      auto const param_length = value.apa_params_length / 2;
      auto const minimum = value.imap_params->lower();
      auto const maximum = value.imap_params->upper();
      klv_write_float( minimum, data, tracker.verify( param_length ) );
      klv_write_float( maximum, data, tracker.verify( param_length ) );
      using format_t = klv_lengthless_format< klv_imap_format >;
      format.reset( new format_t{ *value.imap_params, value.element_size } );
      break;
    }
    case KLV_1303_APA_NATURAL:
      format.reset( new Format{ m_format } );
      format->set_length_constraints( value.element_size );
      break;
    case KLV_1303_APA_BOOLEAN:
    case KLV_1303_APA_UINT:
    case KLV_1303_APA_RLE:
    default:
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: ADA value not supported" );
      break;
  }

  // Array data elements
  for( auto const& element : value.elements )
  {
    format->write( element, data, tracker.verify( value.element_size ) );
  }
}

// ----------------------------------------------------------------------------
template < class Format >
size_t
klv_1303_mdap_format< Format >
::length_of_typed( mdap_t const& value ) const
{
  auto const length_of_dimension_count =
    klv_ber_oid_length( value.sizes.size() );
  auto const length_of_sizes =
    std::accumulate( value.sizes.begin(), value.sizes.end(), size_t{ 0 },
                     []( size_t count, size_t size ){
                       return count + klv_ber_oid_length( size );
                     } );
  auto const length_of_element_size =
    klv_ber_oid_length( value.element_size );
  auto const length_of_apa =
    klv_1303_apa_format{}.length_of_( value.apa );
  auto const length_of_apa_params = value.apa_params_length;
  size_t length_of_array = 0;
  switch( value.apa )
  {
    case KLV_1303_APA_IMAP:
    case KLV_1303_APA_NATURAL:
      // Element size times number of elements
      length_of_array =
        std::accumulate( value.sizes.begin(), value.sizes.end(),
                         value.element_size, std::multiplies< size_t >{} );
      break;
    case KLV_1303_APA_BOOLEAN:
    case KLV_1303_APA_UINT:
    case KLV_1303_APA_RLE:
    default:
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: ADA value not supported" );
      break;
  }

  return length_of_dimension_count +
         length_of_sizes +
         length_of_element_size +
         length_of_apa +
         length_of_apa_params +
         length_of_array;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
