// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1303 read / write.

#include "data_format.h"

#include <arrows/klv/klv_1303.h>

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
void
test_read_write_bool( klv_value const& expected_result,
                      klv_bytes_t const& input_bytes )
{
  using format_t = klv_1303_mdap_format< klv_bool_format >;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
void
test_read_write_uint( klv_value const& expected_result,
                      klv_bytes_t const& input_bytes )
{
  using format_t = klv_1303_mdap_format< klv_uint_format >;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
void
test_read_write_rle( klv_value const& expected_result,
                     klv_bytes_t const& input_bytes )
{
  using format_t = klv_1303_mdap_format< klv_sint_format >;
  test_read_write_format< format_t >( expected_result, input_bytes );

  // Ensure that the RLE encoding is producing compact output
  EXPECT_EQ( input_bytes.size(), format_t{}.length_of( expected_result ) );
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

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_bool )
{
  // Example value in ST1303.2 Appendix D.2
  auto const expected_result = klv_1303_mdap< bool >{
    { 5, 4 },
    { false, true, false, false,
      true, false, false, false,
      true, false, true, false,
      true, false, false, false,
      true, true, true, true } };

  auto const input_bytes = klv_bytes_t{
    0x02, // Number of dimensions
    0x05, 0x04, // Dimensions
    0x01, // Element size,
    0x03, // APA
    0x48, 0xA8, 0xF0, // Elements
  };

  CALL_TEST( test_read_write_bool, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_uint_example1 )
{
  // Example value 1 in ST1303.2 Appendix D.3
  auto const expected_result = klv_1303_mdap< uint64_t >{
    { 3, 3 },
    { 12, 54, 350,
      2, 2048, 0,
      127, 128, 1 } };

  auto const input_bytes = klv_bytes_t{
    0x02, // Number of dimensions
    0x03, 0x03, // Dimensions
    0x01, // Element size,
    0x04, // APA
    0x00, // APA params
    0x0C, 0x36, 0x82, 0x5E, // Elements
    0x02, 0x90, 0x00, 0x00,
    0x7F, 0x81, 0x00, 0x01, };

  CALL_TEST( test_read_write_uint, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_uint_example2 )
{
  // Example value 2 in ST1303.2 Appendix D.3
  auto const expected_result = klv_1303_mdap< uint64_t >{
    { 5 },
    { 130, 170, 155, 143, 190 } };

  auto const input_bytes = klv_bytes_t{
    0x01, // Number of dimensions
    0x05, // Dimensions
    0x01, // Element size
    0x04, // APA
    0x81, 0x02, // APA params
    0x00, 0x28, 0x19, 0x0D, 0x3C, // Elements
  };

  CALL_TEST( test_read_write_uint, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_rle_example )
{
  // Example value in ST1303.2 Appendix D.4
  auto const expected_result = klv_1303_mdap< int64_t >{
    { 10, 10 },
    { 1656, 1656, 1656, -1424, -1424, 0, 0, 0, 0, 0,
      1656, 1656, 1656, -1424, -1424, 0, 0, 0, 0, 0,
      1656, 1656, 1656, -1424, -1424, 0, 0, 0, 0, 0,
      1656, 1656, 1656, -1424, -1424, 0, 0, 0, 0, 0,
      -1015, -1015, -1015, -1424, -1424, 978, 978, 978, 978, 978,
      -1015, -1015, -1015, -1424, -1424, 978, 978, 978, 978, 978,
      -1015, -1015, -1015, -1424, -1424, 978, 978, 978, 978, 978,
      -1015, -1015, -1015, -1424, -1424, 1260, 1260, 1260, 1260, 1260,
      -1015, -1015, -1015, -1424, -1424, 1260, 1260, 1260, 1260, 1260,
      -1015, -1015, -1015, -1424, -1424, 1260, 1260, 1260, 1260, 1260, },
    2, KLV_1303_APA_RLE, 2, std::nullopt };

  auto const input_bytes = klv_bytes_t{
    0x02, // Number of dimensions
    0x0A, 0x0A, // Dimensions
    0x02, // Element size
    0x05, // APA
    0xFA, 0x70, // APA params
    0x06, 0x78, 0x00, 0x00, 0x04, 0x03, // Elements
    0x00, 0x00, 0x00, 0x05, 0x04, 0x05,
    0xFC, 0x09, 0x04, 0x00, 0x06, 0x03,
    0x03, 0xD2, 0x04, 0x05, 0x03, 0x05,
    0x04, 0xEC, 0x07, 0x05, 0x03, 0x05, };

  CALL_TEST( test_read_write_rle, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_1303_rle_3d )
{
  auto const expected_result = klv_1303_mdap< int64_t >{
    { 2, 3, 4 },
    { 1, 1, 2, 2,
      3, 3, 3, 4,
      3, 3, 4, 4,

      0, 1, 2, 2,
      3, 3, 3, 3,
      3, 3, 4, 4 },
    1, KLV_1303_APA_RLE, 1, std::nullopt };

  auto const input_bytes = klv_bytes_t{
    0x03, // Number of dimensions
    0x02, 0x03, 0x04, // Dimensions
    0x01, // Element size
    0x05, // APA
    0x03, // APA params
    0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, // Elements
    0x02, 0x00, 0x00, 0x02, 0x02, 0x01, 0x02,
    0x04, 0x00, 0x01, 0x03, 0x01, 0x01, 0x01,
    0x04, 0x00, 0x02, 0x02, 0x02, 0x01, 0x02,
    0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, };

  CALL_TEST( test_read_write_rle, expected_result, input_bytes );
}
