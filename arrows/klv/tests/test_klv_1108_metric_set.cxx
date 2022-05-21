// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 1108 metric set read / write.

#include "data_format.h"

#include <arrows/klv/klv_1108_metric_set.h>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_read( klv_value const& expected_result, klv_bytes_t const& input_bytes )
{
  using format_t = klv_1108_metric_local_set_format;
  test_read_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_1108_metric_set )
{
  using kld = klv_lengthy< double >;
  auto const input_bytes = klv_bytes_t{
    KLV_1108_METRIC_SET_NAME,        3, 'G', 'S', 'D',
    KLV_1108_METRIC_SET_VERSION,     5, 'H', 'u', 'm', 'a', 'n',
    KLV_1108_METRIC_SET_IMPLEMENTER, 5, 'K', 'W', 30, 'C', 'V',
    KLV_1108_METRIC_SET_PARAMETERS,  3, 'x', '=', '7',
    KLV_1108_METRIC_SET_TIME,        8,
    0x00, 0x00, 0x00, 0x00, 0x61, 0x27, 0xD3, 0x80,
    KLV_1108_METRIC_SET_VALUE,       8,
    0x3F, 0xF3, 0xC0, 0xC9, 0x53, 0x9B, 0x88, 0x87,
    // Unknown tag
    KLV_1108_METRIC_SET_ENUM_END,    2, 0x01, 0x02 };
  auto const expected_result = klv_local_set{
    { KLV_1108_METRIC_SET_NAME,        std::string{ "GSD" } },
    { KLV_1108_METRIC_SET_VERSION,     std::string{ "Human" } },
    { KLV_1108_METRIC_SET_IMPLEMENTER,
      klv_1108_metric_implementer{ "KW", "CV" } },
    { KLV_1108_METRIC_SET_PARAMETERS,  std::string{ "x=7" } },
    { KLV_1108_METRIC_SET_TIME,        uint64_t{ 1630000000 } },
    { KLV_1108_METRIC_SET_VALUE,       kld{ 1.234567 } },
    { KLV_1108_METRIC_SET_ENUM_END,    klv_blob{ 0x01, 0x02 } } };

  CALL_TEST( test_read, {}, {} );
  CALL_TEST( test_read, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
void
test_write( klv_value const& value )
{
  test_write_format< klv_1108_metric_local_set_format >( value );
}

// ----------------------------------------------------------------------------
TEST ( klv, write_1108_metric_set )
{
  using kld = klv_lengthy< double >;
  auto const test_data = klv_local_set{
    { KLV_1108_METRIC_SET_NAME,        std::string{ "METRIC" } },
    { KLV_1108_METRIC_SET_VERSION,     std::string{ "13 and a half" } },
    { KLV_1108_METRIC_SET_IMPLEMENTER, klv_1108_metric_implementer{ "", "" } },
    { KLV_1108_METRIC_SET_PARAMETERS,  klv_value{} },
    { KLV_1108_METRIC_SET_TIME,        uint64_t{ 1630000001000000 } },
    { KLV_1108_METRIC_SET_VALUE,
      kld{ std::numeric_limits< double >::infinity(), 4 } }, };
  CALL_TEST( test_write, {} );
  CALL_TEST( test_write, test_data );
}
