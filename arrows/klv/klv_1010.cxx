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
  os << " } }";
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
    m_sigma_imap{},
    m_preceding_keys{}
{}

// ----------------------------------------------------------------------------
klv_1010_sdcc_flp_format
::klv_1010_sdcc_flp_format( imap_from_key_fn sigma_imap )
  : klv_data_format_< klv_1010_sdcc_flp >{ 0 },
    m_sigma_imap{ sigma_imap },
    m_preceding_keys{}
{}

// ----------------------------------------------------------------------------
void
klv_1010_sdcc_flp_format
::set_preceding( std::vector< klv_lds_key > const& preceding_keys )
{
  m_preceding_keys = preceding_keys;
}

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

  if( m_preceding_keys.size() < matrix_size )
  {
    VITAL_THROW( kv::metadata_exception,
                 "SDCC-FLP: insufficient preceding keys" );
  }
  std::copy( m_preceding_keys.end() - matrix_size,
             m_preceding_keys.end(),
             result.members.begin() );

  // Read parse control bytes
  auto const parse_control_begin = data;
  auto parse_control =
    klv_read_ber_oid< uint16_t >( data, tracker.verify( 2 ) );
  if( ( result.long_parse_control =
          std::distance( parse_control_begin, data ) > 1 ) )
  {
    result.sigma_length = parse_control & 0xF;
    parse_control >>= 4;
    result.sigma_uses_imap = parse_control & 0x1;
    parse_control >>= 3;
    result.rho_length = parse_control & 0xF;
    parse_control >>= 4;
    result.rho_uses_imap = parse_control & 0x1;
    parse_control >>= 1;
    result.sparse = parse_control & 0x1;
  }
  else
  {
    result.sigma_uses_imap = m_sigma_imap;
    result.rho_uses_imap = true;

    result.rho_length = parse_control & 0x7;
    parse_control >>= 3;
    result.sparse = parse_control & 0x1;
    parse_control >>= 1;
    result.sigma_length = parse_control & 0x7;
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
  if( result.sigma_length )
  {
    for( size_t i = 0; i < matrix_size; ++i )
    {
      double value;
      if( result.sigma_uses_imap )
      {
        auto const format =
          m_sigma_imap( result.members.at( i ), result.sigma_length );
        value = format.read_( data, tracker.verify( result.sigma_length ) );
      }
      else
      {
        value = klv_read_float( data, tracker.verify( result.sigma_length ) );
      }
      result.sigma.push_back( value );
    }
  }

  // Read correlations
  if( result.rho_length )
  {
    for( auto const i : kvr::iota< size_t >( rho_count ) )
    {
      double value;
      if( result.sparse && !( bitset.at( i / 8 ) & ( 0x80 >> ( i % 8 ) ) ) )
      {
        value = 0.0;
      }
      else if( result.rho_uses_imap )
      {
        value = klv_read_imap( -1.0, 1.0, data,
                               tracker.verify( result.rho_length ) );
      }
      else
      {
        value = klv_read_float( data, tracker.verify( result.rho_length ) );
      }
      result.rho.push_back( value );
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
  if( value.long_parse_control )
  {
    auto parse_control = static_cast< uint16_t >( value.sparse );
    parse_control <<= 1;
    parse_control |= value.rho_uses_imap;
    parse_control <<= 4;
    parse_control |= value.rho_length;
    parse_control <<= 3;
    parse_control |= value.sigma_uses_imap;
    parse_control <<= 4;
    parse_control |= value.sigma_length;
    if( ( parse_control >> 8 ) == 0 )
    {
      *data = 0x80;
      ++data;
      klv_write_ber_oid( parse_control, data, tracker.verify( 1 ) );
    }
    else
    {
      klv_write_ber_oid( parse_control, data, tracker.verify( 2 ) );
    }
  }
  else
  {
    auto parse_control = static_cast< uint16_t >( value.sigma_length );
    parse_control <<= 1;
    parse_control |= value.sparse;
    parse_control <<= 3;
    parse_control |= value.rho_length;
    klv_write_ber_oid( parse_control, data, tracker.verify( 1 ) );
  }

  // Write sparse bit vector
  auto const rho_count = matrix_size * ( matrix_size - 1 ) / 2;
  if( value.sparse && value.rho_length )
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
  if( value.sigma_length )
  {
    auto it = value.members.begin();
    for( auto const sigma_value : value.sigma )
    {
      if( value.sigma_uses_imap )
      {
        auto const format = m_sigma_imap( *it, value.sigma_length );
        format.write_( sigma_value, data,
                       tracker.verify( value.sigma_length ) );
      }
      else
      {
        klv_write_float( sigma_value, data,
                         tracker.verify( value.sigma_length ) );
      }
      ++it;
    }
  }

  // Write correlations
  if( value.rho_length )
  {
    for( auto const rho_value : value.rho )
    {
      if( value.sparse && !rho_value )
      {
        continue;
      }
      else if( value.rho_uses_imap )
      {
        klv_write_imap( rho_value, -1.0, 1.0, data,
                        tracker.verify( value.rho_length ) );
      }
      else
      {
        klv_write_float( rho_value, data, tracker.verify( value.rho_length ) );
      }
    }
  }
}

// ----------------------------------------------------------------------------
size_t
klv_1010_sdcc_flp_format
::length_of_typed( klv_1010_sdcc_flp const& value ) const
{
  auto const matrix_size = value.members.size();
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
  auto const length_of_sigma = value.sigma_length * value.sigma.size();
  auto const length_of_rho =
    value.rho_length * ( value.sparse ? rho_sparse_count : rho_count );
  return length_of_matrix_size +
         length_of_parse_control +
         length_of_bit_vector +
         length_of_sigma +
         length_of_rho;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
