// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV checksum functions.

#include "klv_checksum.h"

#include <numeric>
#include <vector>

#include <cstddef>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
template < class Iterator >
uint16_t
klv_running_sum_16( Iterator data_begin, Iterator data_end )
{
  constexpr uint16_t initializer = 0x0000;

  // Counter to check even / odd byte
  size_t i = 0;
  auto accumulator =
    [ &i ]( uint16_t sum, uint8_t byte ) -> uint16_t {
      uint16_t const new_value = ( ++i % 2 ) ? ( byte << 8 ) : byte;
      return sum + new_value;
    };

  return std::accumulate( data_begin, data_end, initializer, accumulator );
}

// ----------------------------------------------------------------------------
template < class Iterator >
uint16_t
klv_crc_16_ccitt( Iterator data_begin, Iterator data_end )
{
  constexpr uint16_t initializer = 0xFFFF;

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
  auto crc = std::accumulate( data_begin, data_end, initializer, accumulator );

  // Proper CRC-16 pads input with 16 bits of zeros at the end
  std::vector< uint8_t > const padding = { 0x00, 0x00 };
  return std::accumulate( padding.cbegin(), padding.cend(), crc, accumulator );
}

// ----------------------------------------------------------------------------
// Instantiate templates for common iterators
#define KLV_INSTANTIATE( FN, T ) \
  template KWIVER_ALGO_KLV_EXPORT FN< T >( T, T )
#define KLV_INSTANTIATE_ALL( FN ) \
  KLV_INSTANTIATE( FN, uint8_t const* ); \
  KLV_INSTANTIATE( FN, typename std::vector< uint8_t >::iterator ); \
  KLV_INSTANTIATE( FN, typename std::vector< uint8_t >::const_iterator )

KLV_INSTANTIATE_ALL( uint16_t klv_crc_16_ccitt );
KLV_INSTANTIATE_ALL( uint16_t klv_running_sum_16 );

#undef KLV_INSTANTIATE
#undef KLV_INSTANTIATE_ALL

} // namespace klv

} // namespace arrows

} // namespace kwiver
