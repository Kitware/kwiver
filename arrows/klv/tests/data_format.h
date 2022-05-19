// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utility function templates for testing read/write for KLV data
/// formats.

#ifndef KWIVER_ARROWS_KLV_TESTS_DATA_FORMAT_H_
#define KWIVER_ARROWS_KLV_TESTS_DATA_FORMAT_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_packet.h>

#include <vital/util/demangle.h>

#include <tests/test_gtest.h>

// ----------------------------------------------------------------------------
using namespace kwiver::arrows::klv;
namespace kv = kwiver::vital;

// ----------------------------------------------------------------------------
template < class Format >
void
test_read_format( klv_value const& expected_result, klv_bytes_t const& bytes )
{
  Format const format;
  auto it = bytes.cbegin();
  auto const result = format.read( it, bytes.size() );
  ASSERT_EQ( bytes.cend(), it );
  ASSERT_EQ( expected_result.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( expected_result.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( expected_result, result )
        << "\n  --value difference--\n  "
        << format.to_string( expected_result )
        << "\n  --versus--\n  "
        << format.to_string( result );
}

// ----------------------------------------------------------------------------
template < class Format >
void
test_write_format( klv_value const& value )
{
  Format const format;
  klv_bytes_t bytes( format.length_of( value ) );
  auto write_it = bytes.begin();
  format.write( value, write_it, bytes.size() );
  ASSERT_EQ( bytes.end(), write_it );

  auto read_it = bytes.cbegin();
  auto const result = format.read( read_it, bytes.size() );
  ASSERT_EQ( value.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( value.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( value, result )
        << "\n  --value difference--\n  "
        << format.to_string( value )
        << format.to_string( result );
}

// ----------------------------------------------------------------------------
template < class Format >
void
test_read_write_format( klv_value const& expected_result,
                        klv_bytes_t const& bytes,
                        Format const& format = Format{} )
{
  auto it = bytes.cbegin();
  auto result = format.read( it, bytes.size() );
  ASSERT_EQ( bytes.cend(), it );
  ASSERT_EQ( expected_result.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( expected_result.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( expected_result, result )
        << "\n  --value difference--\n  "
        << format.to_string( expected_result )
        << "\n  --versus--\n  "
        << format.to_string( result );

  klv_bytes_t buffer( format.length_of( result ) );
  auto write_it = buffer.begin();
  format.write( result, write_it, buffer.size() );
  ASSERT_EQ( buffer.end(), write_it );

  auto read_it = buffer.cbegin();
  result = format.read( read_it, buffer.size() );
  ASSERT_EQ( expected_result.type(), result.type() )
        << "\n  --type difference--\n  "
        << kv::demangle( expected_result.type().name() )
        << "\n  --versus--\n  "
        << kv::demangle( result.type().name() );
  EXPECT_EQ( expected_result, result )
        << "\n  --value difference--\n  "
        << format.to_string( expected_result )
        << "\n  --versus--\n  "
        << format.to_string( result );
}

// ----------------------------------------------------------------------------
void
test_read_write_packet( klv_value const& expected_result,
                        klv_bytes_t const& payload_bytes,
                        klv_bytes_t const& footer_bytes,
                        klv_uds_key const& key )
{
  // Assemble the target packet's serialized form
  auto packet_bytes =
    klv_bytes_t( klv_uds_key::length +
                 klv_ber_length( payload_bytes.size() + footer_bytes.size() ) +
                 payload_bytes.size() +
                 footer_bytes.size() );
  auto bytes_it = packet_bytes.begin();
  bytes_it = std::copy( key.cbegin(), key.cend(),
                        bytes_it );
  klv_write_ber( payload_bytes.size() + footer_bytes.size(), bytes_it,
                 std::distance( bytes_it, packet_bytes.end() ) );
  bytes_it = std::copy( payload_bytes.cbegin(), payload_bytes.cend(),
                        bytes_it );
  bytes_it = std::copy( footer_bytes.cbegin(), footer_bytes.cend(),
                        bytes_it );

  // Assemble the target packet's unserialized form
  auto const test_packet = klv_packet{ key, expected_result };

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

#endif
