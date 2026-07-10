// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test color_space enumeration and its string conversions

#include <tests/test_gtest.h>

#include <vital/types/color_space.h>

#include <string>
#include <vector>

using namespace kwiver::vital;

namespace {

std::vector< color_space > const all_spaces = {
  RGB, BGR, HSV, HSL, HLS, XYZ, Lab, Luv, CMYK, YCrCb, YCbCr, };

} // namespace

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( color_space, round_trip )
{
  for( auto const cs : all_spaces )
  {
    EXPECT_EQ( cs, string_to_color_space( color_space_to_string( cs ) ) )
      << "round trip failed for " << color_space_to_string( cs );
  }
}

// ----------------------------------------------------------------------------
TEST ( color_space, to_string_is_unique )
{
  for( size_t i = 0; i < all_spaces.size(); ++i )
  {
    for( size_t j = i + 1; j < all_spaces.size(); ++j )
    {
      EXPECT_NE(
        color_space_to_string( all_spaces[ i ] ),
        color_space_to_string( all_spaces[ j ] ) );
    }
  }
}

// ----------------------------------------------------------------------------
TEST ( color_space, unrecognized_string_is_invalid )
{
  EXPECT_EQ( INVALID_CS, string_to_color_space( "not_a_color_space" ) );
  EXPECT_EQ( INVALID_CS, string_to_color_space( "" ) );
}

// ----------------------------------------------------------------------------
TEST ( color_space, invalid_round_trip )
{
  EXPECT_EQ(
    INVALID_CS,
    string_to_color_space( color_space_to_string( INVALID_CS ) ) );
}
