// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV checksum functions.

#ifndef KWIVER_ARROWS_KLV_KLV_CHECKSUM_H_
#define KWIVER_ARROWS_KLV_KLV_CHECKSUM_H_

#include <arrows/klv/kwiver_algo_klv_export.h>
#include <arrows/klv/klv_data_format.h>

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
KWIVER_ALGO_KLV_EXPORT
uint16_t
klv_running_sum_16( klv_read_iter_t data_begin, klv_read_iter_t data_end,
                    uint16_t initial_value = 0x0000, bool parity = false );

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
KWIVER_ALGO_KLV_EXPORT
uint16_t
klv_crc_16_ccitt( klv_read_iter_t data_begin, klv_read_iter_t data_end,
                  uint16_t initial_value = 0xFFFF );

// ----------------------------------------------------------------------------
/// Calculate the CRC-32-MPEG checksum of the given bytes.
///
/// The CRC-32-MPEG specification is a 32-bit CRC with the polynomial \c
/// 0x04C11DB7 and an initial value of \c 0xFFFFFFFF. No special modification
/// is made to the input data or output CRC.
///
/// \param data_begin Iterator to the beginning of a buffer of \c uint8_t.
/// \param data_end Iterator to the end of a buffer of \c uint8_t.
///
/// \return Checksum of the data buffer.
KWIVER_ALGO_KLV_EXPORT
uint32_t
klv_crc_32_mpeg( klv_read_iter_t data_begin, klv_read_iter_t data_end,
                 uint32_t initial_value = 0xFFFFFFFF );

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_checksum_packet_format
  : public klv_data_format_< uint64_t >
{
public:
  klv_checksum_packet_format( klv_bytes_t const& header, size_t payload_size );

  virtual uint64_t
  evaluate( klv_read_iter_t data, size_t length ) const = 0;

  klv_bytes_t header() const;

protected:
  uint64_t
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( uint64_t const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( uint64_t const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os, uint64_t const& value ) const override;

private:
  klv_bytes_t m_header;
  size_t m_payload_size;
};

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_running_sum_16_packet_format
  : public klv_checksum_packet_format
{
  public:
  klv_running_sum_16_packet_format( klv_bytes_t const& header );

  std::string
  description() const override;

  uint64_t
  evaluate( klv_read_iter_t data, size_t length ) const override;
};

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_crc_16_ccitt_packet_format
  : public klv_checksum_packet_format
{
  public:
  klv_crc_16_ccitt_packet_format( klv_bytes_t const& header );

  std::string
  description() const override;

  uint64_t
  evaluate( klv_read_iter_t data, size_t length ) const override;
};

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_crc_32_mpeg_packet_format
  : public klv_checksum_packet_format
{
  public:
  klv_crc_32_mpeg_packet_format( klv_bytes_t const& header );

  std::string
  description() const override;

  uint64_t
  evaluate( klv_read_iter_t data, size_t length ) const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
