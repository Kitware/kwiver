// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1303 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1303.hpp>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_read_write_imap( klv_value const& expected_result,
                      klv_bytes_t const& input_bytes,
                      double minimum, double maximum, size_t fixed_length )
{
  using format_t =
    klv_1303_mdap_format< klv_lengthless_format< klv_imap_format > >;
  test_read_write_format< format_t >(
    expected_result, input_bytes,
    { kwiver::vital::interval< double >{ minimum, maximum }, fixed_length } );
}

// ----------------------------------------------------------------------------
void
test_read_write_float( klv_value const& expected_result,
                       klv_bytes_t const& input_bytes,
                       size_t fixed_length )
{
  using format_t =
    klv_1303_mdap_format< klv_lengthless_format< klv_float_format > >;
  test_read_write_format< format_t >( expected_result, input_bytes,
                                      { fixed_length } );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_imap )
{
  auto const expected_result = klv_1303_mdap< double >{
    { 4, 2 },
    { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 } };

  auto const input_bytes = klv_bytes_t{
    0x02, // Number of dimensions
    0x04, 0x02, // Dimensions
    0x02, // Element size
    0x02, // APA
    0x3F, 0x80, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, // APA params
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, // Elements
    0x40, 0x00, 0x50, 0x00, 0x60, 0x00, 0x70, 0x00, };

  CALL_TEST( test_read_write_imap, expected_result, input_bytes, 1.0, 8.0, 2 );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_float )
{
  auto const expected_result = klv_1303_mdap< double >{
    { 4, 2 },
    { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 } };

  auto const input_bytes = klv_bytes_t{
    0x02, // Number of dimensions
    0x04, 0x02, // Dimensions
    0x04, // Element size
    0x01, // APA
    0x3F, 0x80, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, // Elements
    0x40, 0x40, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00,
    0x40, 0xA0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00,
    0x40, 0xE0, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, };

  CALL_TEST( test_read_write_float, expected_result, input_bytes, 4 );
}
