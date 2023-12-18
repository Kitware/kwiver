// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test KLV 0102 read / write.

#include "data_format.h"

#include <arrows/klv/klv_0102.h>
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
  using format_t = klv_0102_local_set_format;
  test_read_write_format< format_t >( expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
auto const expected_result = klv_local_set{
  { KLV_0102_SECURITY_CLASSIFICATION,
    KLV_0102_SECURITY_CLASSIFICATION_UNCLASSIFIED },
  { KLV_0102_COUNTRY_CODING_METHOD,
    KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER },
  { KLV_0102_CLASSIFYING_COUNTRY,
    std::string{ "//US" } },
  { KLV_0102_SCI_SHI_INFORMATION,
    std::string{ "SCI/SHI//" } },
  { KLV_0102_CAVEATS,
    std::string{ "CAVEAT" } },
  { KLV_0102_RELEASING_INSTRUCTIONS,
    std::string{ "NOW" } },
  { KLV_0102_CLASSIFIED_BY,
    std::string{ "Kitware" } },
  { KLV_0102_DERIVED_FROM,
    std::string{ "TEST" } },
  { KLV_0102_CLASSIFICATION_REASON,
    std::string{ "None" } },
  { KLV_0102_DECLASSIFICATION_DATE,
    std::string{ "19700101" } },
  { KLV_0102_CLASSIFICATION_AND_MARKING_SYSTEM,
    std::string{ "." } },
  { KLV_0102_OBJECT_COUNTRY_CODING_METHOD,
    KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER },
  { KLV_0102_OBJECT_COUNTRY_CODES,
    std::string{ "US" } },
  { KLV_0102_CLASSIFICATION_COMMENTS,
    std::string{ "TEST" } },
  { KLV_0102_VERSION,
    uint64_t{ 12 } },
  { KLV_0102_COUNTRY_CODING_METHOD_VERSION_DATE,
    std::string{ "1970-01-01" } },
  { KLV_0102_OBJECT_COUNTRY_CODING_METHOD_VERSION_DATE,
    std::string{ "1970-01-01" } }, };

auto const input_bytes = klv_bytes_t{
  KLV_0102_SECURITY_CLASSIFICATION, 1,
  KLV_0102_SECURITY_CLASSIFICATION_UNCLASSIFIED,
  KLV_0102_COUNTRY_CODING_METHOD, 1,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER,
  KLV_0102_CLASSIFYING_COUNTRY, 4,
  '/', '/', 'U', 'S',
  KLV_0102_SCI_SHI_INFORMATION, 9,
  'S', 'C', 'I', '/', 'S', 'H', 'I', '/', '/',
  KLV_0102_CAVEATS, 6,
  'C', 'A', 'V', 'E', 'A', 'T',
  KLV_0102_RELEASING_INSTRUCTIONS, 3,
  'N', 'O', 'W',
  KLV_0102_CLASSIFIED_BY, 7,
  'K', 'i', 't', 'w', 'a', 'r', 'e',
  KLV_0102_DERIVED_FROM, 4,
  'T', 'E', 'S', 'T',
  KLV_0102_CLASSIFICATION_REASON, 4,
  'N', 'o', 'n', 'e',
  KLV_0102_DECLASSIFICATION_DATE, 8,
  '1', '9', '7', '0', '0', '1', '0', '1',
  KLV_0102_CLASSIFICATION_AND_MARKING_SYSTEM, 1,
  '.',
  KLV_0102_OBJECT_COUNTRY_CODING_METHOD, 1,
  KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER,
  KLV_0102_OBJECT_COUNTRY_CODES, 2,
  'U', 'S',
  KLV_0102_CLASSIFICATION_COMMENTS, 4,
  'T', 'E', 'S', 'T',
  KLV_0102_VERSION, 2,
  0x00, 0x0C,
  KLV_0102_COUNTRY_CODING_METHOD_VERSION_DATE, 10,
  '1', '9', '7', '0', '-', '0', '1', '-', '0', '1',
  KLV_0102_OBJECT_COUNTRY_CODING_METHOD_VERSION_DATE, 10,
  '1', '9', '7', '0', '-', '0', '1', '-', '0', '1', };

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0102 )
{
  CALL_TEST( test_read_write, {}, {} );
  CALL_TEST( test_read_write, expected_result, input_bytes );
}

// ----------------------------------------------------------------------------
TEST ( klv, read_write_0102_packet )
{
  CALL_TEST( test_read_write_packet,
             expected_result, input_bytes, {}, klv_0102_key() );
}
