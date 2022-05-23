// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV checksum functions.

#include "klv_checksum.h"

#include <iomanip>
#include <numeric>
#include <vector>

#include <cstddef>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Iterator >
uint16_t
klv_running_sum_16( Iterator data_begin, Iterator data_end,
                    uint16_t initial_value, bool parity )
{
  // Counter to check even / odd byte
  size_t i = parity;
  auto accumulator =
    [ &i ]( uint16_t sum, uint8_t byte ) -> uint16_t {
      uint16_t const new_value = ( ++i % 2 ) ? ( byte << 8 ) : byte;
      return sum + new_value;
    };

  return std::accumulate( data_begin, data_end, initial_value, accumulator );
}

// ----------------------------------------------------------------------------
template < class Iterator >
uint16_t
klv_crc_16_ccitt( Iterator data_begin, Iterator data_end,
                  uint16_t initial_value )
{
  // Based on http://srecord.sourceforge.net/crc16-ccitt.html
  auto accumulator =
    []( uint16_t crc, uint8_t byte ) -> uint16_t {
      constexpr uint16_t polynomial = 0x1021;
      for( size_t i = 0; i < 8; ++i )
      {
        bool const high_bit = crc & 0x8000;
        crc <<= 1;
        crc += static_cast< bool >( byte & ( 1 << ( 7 - i ) ) );
        if( high_bit )
        {
          crc ^= polynomial;
        }
      }
      return crc;
    };

  // CRC of given data
  auto crc =
    std::accumulate( data_begin, data_end, initial_value, accumulator );

  // Proper CRC-16 pads input with 16 bits of zeros at the end
  std::vector< uint8_t > const padding = { 0x00, 0x00 };
  return std::accumulate( padding.cbegin(), padding.cend(), crc, accumulator );
}

// ----------------------------------------------------------------------------
template < class Iterator >
uint32_t
klv_crc_32_mpeg( Iterator data_begin, Iterator data_end,
                 uint32_t initial_value )
{
  auto accumulator =
    []( uint32_t crc, uint8_t byte ) -> uint32_t {
      constexpr uint32_t polynomial = 0x04C11DB7;
      crc ^= static_cast< uint32_t >( byte ) << 24;
      for( size_t i = 0; i < 8; ++i )
      {
        bool const high_bit = crc & 0x80000000;
        crc <<= 1;
        if( high_bit )
        {
          crc ^= polynomial;
        }
      }
      return crc;
    };

  // CRC of given data
  return std::accumulate( data_begin, data_end, initial_value, accumulator );
}

// ----------------------------------------------------------------------------
// Instantiate templates for common iterators
#define KLV_INSTANTIATE( FN, T, RETURN, ... ) \
  template KWIVER_ALGO_KLV_EXPORT FN< T >( T, T, RETURN __VA_ARGS__ )
#define KLV_INSTANTIATE_ALL( FN, RETURN, ... ) \
  KLV_INSTANTIATE( RETURN FN, uint8_t const*, RETURN, __VA_ARGS__ ); \
  KLV_INSTANTIATE( RETURN FN, typename std::vector< uint8_t >::iterator, RETURN, __VA_ARGS__ ); \
  KLV_INSTANTIATE( RETURN FN, typename std::vector< uint8_t >::const_iterator, RETURN, __VA_ARGS__ )

// Double comma is intentional
KLV_INSTANTIATE_ALL( klv_running_sum_16, uint16_t,, bool );
KLV_INSTANTIATE_ALL( klv_crc_16_ccitt, uint16_t );
KLV_INSTANTIATE_ALL( klv_crc_32_mpeg, uint32_t );

#undef KLV_INSTANTIATE
#undef KLV_INSTANTIATE_ALL

// ----------------------------------------------------------------------------
klv_checksum_packet_format
::klv_checksum_packet_format( klv_bytes_t const& header, size_t payload_size )
  : klv_data_format_< uint64_t >{ header.size() + payload_size },
    m_header{ header }, m_payload_size{ payload_size }
{}

// ----------------------------------------------------------------------------
klv_bytes_t
klv_checksum_packet_format
::header() const
{
  return m_header;
}

// ----------------------------------------------------------------------------
uint64_t
klv_checksum_packet_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  if( !std::equal( m_header.cbegin(), m_header.cend(), data ) )
  {
    VITAL_THROW( kv::metadata_exception,
                 "checksum header not present" );
  }
  data += m_header.size();
  return klv_read_int< uint64_t >( data, m_payload_size );
}

// ----------------------------------------------------------------------------
void
klv_checksum_packet_format
::write_typed( uint64_t const& value,
              klv_write_iter_t& data, size_t length ) const
{
  data = std::copy( m_header.cbegin(), m_header.cend(), data );
  klv_write_int( value, data, m_payload_size );
}

// ----------------------------------------------------------------------------
size_t
klv_checksum_packet_format
::length_of_typed( VITAL_UNUSED uint64_t const& value ) const
{
  return m_header.size() + m_payload_size;
}

// ----------------------------------------------------------------------------
std::ostream&
klv_checksum_packet_format
::print_typed( std::ostream& os, uint64_t const& value ) const
{
  return os << "0x" << std::hex << std::setfill( '0' )
            << std::setw( m_payload_size * 2 )
            << value;
}

// ----------------------------------------------------------------------------
klv_running_sum_16_packet_format
::klv_running_sum_16_packet_format( klv_bytes_t const& header )
  : klv_checksum_packet_format{ header, 2 }
{}

// ----------------------------------------------------------------------------
std::string
klv_running_sum_16_packet_format
::description() const
{
  return "running 16-byte sum packet of " + length_description();
}

// ----------------------------------------------------------------------------
uint64_t
klv_running_sum_16_packet_format
::evaluate( klv_read_iter_t data, size_t length ) const
{
  return klv_running_sum_16( data, data + length );
}

// ----------------------------------------------------------------------------
klv_crc_16_ccitt_packet_format
::klv_crc_16_ccitt_packet_format( klv_bytes_t const& header )
  : klv_checksum_packet_format{ header, 2 }
{}

// ----------------------------------------------------------------------------
std::string
klv_crc_16_ccitt_packet_format
::description() const
{
  return "CRC-16-CCITT packet of " + length_description();
}

// ----------------------------------------------------------------------------
uint64_t
klv_crc_16_ccitt_packet_format
::evaluate( klv_read_iter_t data, size_t length ) const
{
  return klv_crc_16_ccitt( data, data + length );
}

// ----------------------------------------------------------------------------
klv_crc_32_mpeg_packet_format
::klv_crc_32_mpeg_packet_format( klv_bytes_t const& header )
  : klv_checksum_packet_format{ header, 4 }
{}

// ----------------------------------------------------------------------------
std::string
klv_crc_32_mpeg_packet_format
::description() const
{
  return "CRC-32-MPEG packet of " + length_description();
}

// ----------------------------------------------------------------------------
uint64_t
klv_crc_32_mpeg_packet_format
::evaluate( klv_read_iter_t data, size_t length ) const
{
  return klv_crc_32_mpeg( data, data + length );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
