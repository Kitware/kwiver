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
uint8_t
klv_crc_8_ccitt( klv_read_iter_t data_begin, klv_read_iter_t data_end,
                 uint8_t initial_value )
{
  // https://www.3dbrew.org/wiki/CRC-8-CCITT
  static const uint8_t table[ 256 ] = {
      0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
      0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
      0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
      0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
      0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
      0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
      0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
      0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
      0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
      0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
      0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
      0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
      0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
      0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
      0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
      0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
      0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
      0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
      0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
      0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
      0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
      0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
      0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
      0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
      0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
      0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
      0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
      0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
      0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
      0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
      0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
      0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
  };

  auto value = initial_value;
  for( auto it = data_begin; it != data_end; ++it )
  {
    value = table[ value ^ *it ];
  }

  return value;
}

// ----------------------------------------------------------------------------
uint16_t
klv_running_sum_16( klv_read_iter_t data_begin, klv_read_iter_t data_end,
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
uint16_t
klv_crc_16_ccitt( klv_read_iter_t data_begin, klv_read_iter_t data_end,
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
uint32_t
klv_crc_32_mpeg( klv_read_iter_t data_begin, klv_read_iter_t data_end,
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
klv_crc_8_ccitt_packet_format
::klv_crc_8_ccitt_packet_format( klv_bytes_t const& header )
  : klv_checksum_packet_format{ header, 1 }
{}

// ----------------------------------------------------------------------------
std::string
klv_crc_8_ccitt_packet_format
::description() const
{
  return "CRC-8-CCITT packet of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
uint64_t
klv_crc_8_ccitt_packet_format
::evaluate( klv_read_iter_t data, size_t length ) const
{
  return klv_crc_8_ccitt( data, data + length );
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
  return "running 16-byte sum packet of " + m_length_constraints.description();
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
  return "CRC-16-CCITT packet of " + m_length_constraints.description();
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
  return "CRC-32-MPEG packet of " + m_length_constraints.description();
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
