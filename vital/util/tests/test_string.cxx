// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test util string_editor class

#include <vital/util/string.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( string, starts_with )
{
  EXPECT_TRUE( starts_with( "input_string", "input" ) );
  EXPECT_FALSE( starts_with( "input_string", " input" ) );
  EXPECT_TRUE( starts_with( " input_string", " input" ) );
  EXPECT_FALSE( starts_with( "input_string", "string" ) );
}

// ----------------------------------------------------------------------------
TEST ( string, format )
{
  EXPECT_EQ( "1 2", string_format( "%d %d", 1, 2 ) );
  EXPECT_EQ( " 1 2", string_format( " %d %d", 1, 2 ) );

  auto const long_string =
    std::string{ "this is a very long string - relatively speaking" };
  EXPECT_EQ( long_string, string_format( "%s", long_string.c_str() ) );
}

// ----------------------------------------------------------------------------
TEST ( string, join )
{
  {
    std::vector< std::string > input_vec;
    EXPECT_EQ( "", join( input_vec, ", " ) ); }

  {
    std::vector< std::string > input_vec{ "one" };
    EXPECT_EQ( "one", join( input_vec, ", " ) );
  }

  {
    std::vector< std::string > input_vec{ "one", "two", "three" };
    EXPECT_EQ( "one, two, three", join( input_vec, ", " ) );
  }

  {
    std::set< std::string > input_set;
    EXPECT_EQ( "", join( input_set, ", " ) );
  }

  {
    std::set< std::string > input_set{ "one" };
    EXPECT_EQ( "one", join( input_set, ", " ) );
  }

  {
    std::set< std::string > input_set{ "one", "three", "two" };
    EXPECT_EQ( "one, three, two", join( input_set, ", " ) );
  }
}

// ----------------------------------------------------------------------------
TEST ( string, time_str_to_seconds )
{
  // HH:MM:SS and MM:SS forms
  EXPECT_DOUBLE_EQ( 3723.0, time_str_to_seconds( "01:02:03" ) );
  EXPECT_DOUBLE_EQ( 123.0, time_str_to_seconds( "02:03" ) );
  EXPECT_DOUBLE_EQ( 0.0, time_str_to_seconds( "00:00:00" ) );

  // Fractional seconds
  EXPECT_DOUBLE_EQ( 1.5, time_str_to_seconds( "00:00:01.5" ) );
  EXPECT_DOUBLE_EQ( 90.25, time_str_to_seconds( "01:30.25" ) );
}

// ----------------------------------------------------------------------------
TEST ( string, time_str_to_seconds_invalid )
{
  EXPECT_THROW( time_str_to_seconds( "not-a-time" ), std::runtime_error );
}
