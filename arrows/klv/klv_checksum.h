// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV checksum functions.

#ifndef KWIVER_ARROWS_KLV_KLV_CHECKSUM_H_
#define KWIVER_ARROWS_KLV_KLV_CHECKSUM_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Calculate a running sum of each 16-byte word in the given bytes.
///
/// If there are an odd number of bytes, the result will be the same as if an
/// additional zero byte were appended to the back of the buffer.
///
/// \param data_begin Iterator to the beginning of a buffer of \c uint8_t.
/// \param data_end Iterator to the end of a buffer of \c uint8_t.
///
/// \return Running 16-bit sum of the data buffer.
template < class Iterator >
KWIVER_ALGO_KLV_EXPORT
uint16_t
klv_running_sum_16( Iterator data_begin, Iterator data_end,
                    uint16_t initial_value = 0x0000 );

// ----------------------------------------------------------------------------
/// Calculate the CRC-16-CCITT checksum of the given bytes.
///
/// The CRC-16-CCITT specification is a 16-bit CRC with the polynomial \c
/// 0x1021 and an initial value of \c 0xFFFF. No special modification is made
/// to the input data or output CRC.
///
/// \param data_begin Iterator to the beginning of a buffer of \c uint8_t.
/// \param data_end Iterator to the end of a buffer of \c uint8_t.
///
/// \return Checksum of the data buffer.
template < class Iterator >
KWIVER_ALGO_KLV_EXPORT
uint16_t
klv_crc_16_ccitt( Iterator data_begin, Iterator data_end,
                  uint16_t initial_value = 0xFFFF );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
