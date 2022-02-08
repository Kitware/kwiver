// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1010 parser.

#include "klv_1010.h"

#include "klv_util.h"

#include "vital/range/iota.h"

#include <numeric>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1010_sdcc_flp const& value )
{
  os << "{ "
     << "members: { ";

  auto first = true;
  for( auto const member : value.members )
  {
    first = first ? false : ( os << ", ", false );
    os << member;
  }
  os << " }, "
     << "sigma: { ";
  first = true;
  for( auto const sigma_value : value.sigma )
  {
    first = first ? false : ( os << ", ", false );
    os << sigma_value;
  }
  os << " }, "
     << "rho: { ";
  first = true;
  for( auto const rho_value : value.rho )
  {
    first = first ? false : ( os << ", ", false );
    os << rho_value;
  }
  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_1010_sdcc_flp,
  &klv_1010_sdcc_flp::members,
  &klv_1010_sdcc_flp::sigma,
  &klv_1010_sdcc_flp::rho
  )

// ----------------------------------------------------------------------------
klv_1010_sdcc_flp_format
::klv_1010_sdcc_flp_format()
  : klv_data_format_< klv_1010_sdcc_flp >{ 0 },
    m_sigma_uses_imap{ false }
{}

// ----------------------------------------------------------------------------
klv_1010_sdcc_flp_format
::klv_1010_sdcc_flp_format( double sigma_minimum, double sigma_maximum )
  : klv_data_format_< klv_1010_sdcc_flp >{ 0 },
    m_sigma_uses_imap{ true },
    m_sigma_minimum{ sigma_minimum },
    m_sigma_maximum{ sigma_maximum }
{}

// ----------------------------------------------------------------------------
std::string
klv_1010_sdcc_flp_format
::description() const
{
  return "SDCC-FLP of " + length_description();
}

// ----------------------------------------------------------------------------
klv_1010_sdcc_flp
klv_1010_sdcc_flp_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto tracker = track_it( data, length );
  klv_1010_sdcc_flp result;

  // Read matrix size
  auto const matrix_size =
    klv_read_ber_oid< size_t >( data, tracker.remaining() );
  result.members.resize( matrix_size, 0 );

  // Read parse control bytes
  auto const parse_control_begin = data;
  auto parse_control =
    klv_read_ber_oid< uint16_t >( data, tracker.verify( 2 ) );
  size_t sigma_length, rho_length;
  auto sigma_uses_imap = m_sigma_uses_imap;
  auto rho_uses_imap = true;
  if( ( result.long_parse_control =
          std::distance( parse_control_begin, data ) > 1 ) )
  {
    sigma_length = parse_control & 0xF;
    parse_control >>= 4;
    sigma_uses_imap = parse_control & 0x1;
    parse_control >>= 3;
    rho_length = parse_control & 0xF;
    parse_control >>= 4;
    rho_uses_imap = parse_control & 0x1;
    parse_control >>= 1;
    result.sparse = parse_control & 0x1;
  }
  else
  {
    rho_length = parse_control & 0x7;
    parse_control >>= 3;
    result.sparse = parse_control & 0x1;
    parse_control >>= 1;
    sigma_length = parse_control & 0x7;
  }

  // Report data formats
  if( sigma_uses_imap )
  {
    result.sigma_format =
      std::make_shared< klv_imap_format >(
        m_sigma_minimum, m_sigma_maximum, sigma_length );
  }
  else
  {
    result.sigma_format = std::make_shared< klv_float_format >( sigma_length );
  }

  if( rho_uses_imap )
  {
    result.rho_format =
      std::make_shared< klv_imap_format >( -1.0, 1.0, rho_length );
  }
  else
  {
    result.rho_format = std::make_shared< klv_float_format >( rho_length );
  }

  // Read sparse bit vector
  auto const rho_count = matrix_size * ( matrix_size - 1 ) / 2;
  auto const bitset_length = ( rho_count + 7 ) / 8;
  std::vector< uint8_t > bitset;
  if( result.sparse )
  {
    bitset.resize( bitset_length );
    std::copy_n( data, bitset_length, bitset.begin() );
    data += bitset_length;
  }

  // Read standard deviations
  if( sigma_length )
  {
    for( size_t i = 0; i < matrix_size; ++i )
    {
      result.sigma.push_back(
          result.sigma_format->read( data, tracker.verify( sigma_length ) )
          .get< double >() );
    }
  }

  // Read correlations
  if( rho_length )
  {
    for( auto const i : kvr::iota< size_t >( rho_count ) )
    {
      if( result.sparse && !( bitset.at( i / 8 ) & ( 0x80 >> ( i % 8 ) ) ) )
      {
        result.rho.push_back( 0.0 );
      }
      else
      {
        result.rho.push_back(
            result.rho_format->read( data, tracker.verify( rho_length ) )
            .get< double >() );
      }
    }
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1010_sdcc_flp_format
::write_typed( klv_1010_sdcc_flp const& value_source,
               klv_write_iter_t& data, size_t length ) const
{
  auto tracker = track_it( data, length );
  auto value = value_source;

  // Write matrix size
  auto const matrix_size = value.members.size();
  if( !matrix_size )
  {
    throw std::invalid_argument( "SDCC-FLP: members cannot be empty" );
  }
  klv_write_ber_oid( matrix_size, data, tracker.remaining() );

  // Write parse control bytes
  auto const sigma_length =
    value.sigma_format ? value.sigma_format->fixed_length() : 0;
  auto const rho_length =
    value.rho_format ? value.rho_format->fixed_length() : 0;
  auto const sigma_uses_imap =
    dynamic_cast< klv_imap_format* >( value.sigma_format.get() ) != nullptr;
  auto const rho_uses_imap =
    dynamic_cast< klv_imap_format* >( value.rho_format.get() ) != nullptr;

  if( value.long_parse_control )
  {
    auto parse_control = static_cast< uint16_t >( value.sparse );
    parse_control <<= 1;
    parse_control |= rho_uses_imap;
    parse_control <<= 4;
    parse_control |= rho_length;
    parse_control <<= 3;
    parse_control |= sigma_uses_imap;
    parse_control <<= 4;
    parse_control |= sigma_length;
    klv_write_ber_oid( parse_control, data, tracker.verify( 2 ) );
  }
  else
  {
    auto parse_control = static_cast< uint16_t >( sigma_length );
    parse_control <<= 1;
    parse_control |= value.sparse;
    parse_control <<= 3;
    parse_control |= rho_length;
    klv_write_ber_oid( parse_control, data, tracker.verify( 1 ) );
  }

  // Write sparse bit vector
  auto const rho_count = matrix_size * ( matrix_size - 1 ) / 2;
  if( value.sparse && rho_length )
  {
    auto const bitset_length = ( rho_count + 7 ) / 8;
    std::vector< uint8_t > bitset( bitset_length );
    for( auto const i : kvr::iota< size_t >( rho_count ) )
    {
      bitset.at( i / 8 ) |= value.rho.at( i ) ? ( 0x80 >> ( i % 8 ) ) : 0;
    }

    data = std::copy_n( bitset.begin(), bitset_length, data );
  }

  // Write standard deviations
  if( sigma_length )
  {
    for( auto const sigma_value : value.sigma )
    {
      value.sigma_format->write( sigma_value, data,
                                 tracker.verify( sigma_length ) );
    }
  }

  // Write correlations
  if( rho_length )
  {
    for( auto const rho_value : value.rho )
    {
      if( value.sparse && !rho_value )
      {
        continue;
      }
      value.rho_format->write( rho_value, data, tracker.verify( rho_length ) );
    }
  }
}

// ----------------------------------------------------------------------------
size_t
klv_1010_sdcc_flp_format
::length_of_typed( klv_1010_sdcc_flp const& value,
                   size_t length_hint ) const
{
  auto const matrix_size = value.members.size();
  auto const sigma_length =
    value.sigma_format ? value.sigma_format->fixed_length() : 0;
  auto const rho_length =
    value.rho_format ? value.rho_format->fixed_length() : 0;
  auto const rho_count =
    matrix_size * ( matrix_size - 1 ) / 2;
  auto const rho_sparse_count =
    std::accumulate( value.rho.begin(), value.rho.end(), size_t{ 0 },
                     []( size_t count, double element ){
                       return count + static_cast< bool >( element );
                     } );
  auto const length_of_matrix_size = klv_ber_oid_length( matrix_size );
  auto const length_of_parse_control = size_t{ 1 } + value.long_parse_control;
  auto const length_of_bit_vector =
    value.sparse ? ( rho_count + 7 ) / 8 : 0;
  auto const length_of_sigma = sigma_length * value.sigma.size();
  auto const length_of_rho =
    rho_length * ( value.sparse ? rho_sparse_count : rho_count );
  return length_of_matrix_size +
         length_of_parse_control +
         length_of_bit_vector +
         length_of_sigma +
         length_of_rho;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
