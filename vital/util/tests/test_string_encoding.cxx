// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test string encoding utilities.

#include <vital/util/string_encoding.h>

#include <gtest/gtest.h>

#include <stdexcept>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( string_encoding, utf8_code_point_count )
{
  // Valid UTF-8 of varying character lengths
  EXPECT_EQ( 0, utf8_code_point_count( "" ) );
  EXPECT_EQ( 7, utf8_code_point_count( "Kitware" ) );
  EXPECT_EQ( 7, utf8_code_point_count( "Kĩtwārę" ) );
  EXPECT_EQ( 7, utf8_code_point_count( "Ḱḯṫẃḁṝḕ" ) );
  EXPECT_EQ( 5, utf8_code_point_count( "🁙🂽🫖🦋🛸" ) );

  // Byte starting with five 1's
  EXPECT_THROW( utf8_code_point_count( "\xF8xxxxxxxx" ),
                std::runtime_error );

  // Starting with continuation byte
  EXPECT_THROW( utf8_code_point_count( "\xBFxxxxxxxx" ),
                std::runtime_error );

  // Continuation byte doesn't start with '10'
  EXPECT_THROW( utf8_code_point_count( "\xE0\xBF\x3F" ),
                std::runtime_error );

  // String ends before multi-byte character is complete
  EXPECT_THROW( utf8_code_point_count( "\xE0\xBF" ),
                std::runtime_error );
}
