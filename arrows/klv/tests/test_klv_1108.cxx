// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test KLV 1108 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>
#include <arrows/klv/klv_packet.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_read_write( klv_value const& expected_result,
                 klv_bytes_t const& input_bytes )
{
  using format_t = klv_1108_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

auto const expected_metric_set = klv_local_set{
  { KLV_1108_METRIC_SET_NAME,       std::string{ "VNIIRS" } },
  { KLV_1108_METRIC_SET_VERSION,    std::string{ "3.0" } },
  { KLV_1108_METRIC_SET_IMPLEMENTER,
    klv_1108_metric_implementer{ "KW", "CV" } },
  { KLV_1108_METRIC_SET_PARAMETERS, std::string{ "A0+A1" } },
  { KLV_1108_METRIC_SET_TIME,       uint64_t{ 1630000000000000 } },
  { KLV_1108_METRIC_SET_VALUE,      7.12345678901234 } };

auto const expected_result = klv_local_set{
  { KLV_1108_ASSESSMENT_POINT,    KLV_1108_ASSESSMENT_POINT_ARCHIVE },
  { KLV_1108_METRIC_PERIOD_PACK,
    klv_1108_metric_period_pack{ 1630000000000000, 7000000 } },
  { KLV_1108_WINDOW_CORNERS_PACK,
    klv_1108_window_corners_pack{ { 0, 0, 1280, 720 } } },
  { KLV_1108_METRIC_LOCAL_SET,    expected_metric_set },
  { KLV_1108_COMPRESSION_TYPE,    KLV_1108_COMPRESSION_TYPE_H264 },
  { KLV_1108_COMPRESSION_PROFILE, KLV_1108_COMPRESSION_PROFILE_HIGH },
  { KLV_1108_COMPRESSION_LEVEL,   std::string{ "5.2" } },
  { KLV_1108_COMPRESSION_RATIO,   klv_value{ 25.200000762939453, 4 } },
  { KLV_1108_STREAM_BITRATE,      uint64_t{ 1024 } },
  { KLV_1108_DOCUMENT_VERSION,    uint64_t{ 3 } } };

auto const input_bytes = klv_bytes_t{
  KLV_1108_ASSESSMENT_POINT,       1,  KLV_1108_ASSESSMENT_POINT_ARCHIVE,
  KLV_1108_METRIC_PERIOD_PACK,     12,
  0x00, 0x05, 0xCA, 0x79, 0xF2, 0xFB, 0xe0, 0x00, 0x00, 0x6A, 0xCF, 0xC0,
  KLV_1108_WINDOW_CORNERS_PACK,    6,  0x00, 0x00, 0x85, 0x50, 0x8A, 0x00,
  KLV_1108_METRIC_LOCAL_SET,       47,
  KLV_1108_METRIC_SET_NAME,        6,  'V', 'N', 'I', 'I', 'R', 'S',
  KLV_1108_METRIC_SET_VERSION,     3,  '3', '.', '0',
  KLV_1108_METRIC_SET_IMPLEMENTER, 5,  'K', 'W', 30, 'C', 'V',
  KLV_1108_METRIC_SET_PARAMETERS,  5,  'A', '0', '+', 'A', '1',
  KLV_1108_METRIC_SET_TIME,        8,
  0x00, 0x05, 0xCA, 0x79, 0xF2, 0xFB, 0xe0, 0x00,
  KLV_1108_METRIC_SET_VALUE,       8,
  0x40, 0x1C, 0x7E, 0x6B, 0x74, 0xDD, 0x1B, 0xD3,
  KLV_1108_COMPRESSION_TYPE,       1,  KLV_1108_COMPRESSION_TYPE_H264,
  KLV_1108_COMPRESSION_PROFILE,    1,  KLV_1108_COMPRESSION_PROFILE_HIGH,
  KLV_1108_COMPRESSION_LEVEL,      3,  '5', '.', '2',
  KLV_1108_COMPRESSION_RATIO,      4,  0x41, 0xC9, 0x99, 0x9A,
  KLV_1108_STREAM_BITRATE,         2,  0x04, 0x00,
  KLV_1108_DOCUMENT_VERSION,       1,  0x03, };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1108 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1108_packet )
{
  auto const packet_header = klv_bytes_t{
    0x06, 0x0E, 0x2B, 0x34, 0x02, 0x03, 0x01, 0x01,
    0x0E, 0x01, 0x03, 0x03, 0x1C, 0x00, 0x00, 0x00,
    102, };
  auto const packet_footer = klv_bytes_t{ KLV_1108_CHECKSUM, 2, 0x89, 0x9F };

  // Assemble the target packet's serialized form
  auto packet_bytes =
    klv_bytes_t( packet_header.size() +
                 input_bytes.size() +
                 packet_footer.size() );
  auto bytes_it = packet_bytes.begin();
  bytes_it = std::copy( packet_header.cbegin(), packet_header.cend(),
                        bytes_it );
  bytes_it = std::copy( input_bytes.cbegin(), input_bytes.cend(),
                        bytes_it );
  bytes_it = std::copy( packet_footer.cbegin(), packet_footer.cend(),
                        bytes_it );

  // Assemble the target packet's unserialized form
  auto const test_packet = klv_packet{ klv_1108_key(), expected_result };

  // Deserialize
  auto read_it = packet_bytes.cbegin();
  auto const read_packet = klv_read_packet( read_it, packet_bytes.size() );
  EXPECT_EQ( packet_bytes.cend(), read_it );
  EXPECT_EQ( test_packet, read_packet );

  // Reserialize
  klv_bytes_t written_bytes( klv_packet_length( read_packet ) );
  auto write_it = written_bytes.begin();
  klv_write_packet( read_packet, write_it, written_bytes.size() );
  EXPECT_EQ( written_bytes.end(), write_it );
  EXPECT_EQ( packet_bytes, written_bytes );
}
