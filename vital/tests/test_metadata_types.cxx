// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/metadata_types.h>

#include <vital/exceptions.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace ::kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST( metadata, std_0102_lds )
{
  auto const obj = std_0102_lds{};
  std::stringstream ss;
  ss << obj;
  // When std_0102_lds is actually implemented, this will fail as a reminder
  // to write an actual unit test in this file
  EXPECT_EQ( ss.str(), "std_0102_local_set" );
}

// ----------------------------------------------------------------------------
TEST( metadata, unix_timestamp )
{
  auto fn = std_0104_datetime_to_unix_timestamp;

  // Wrongly formatted dates
  // YY, not YYYY
  EXPECT_THROW( fn( "030201T070809" );,   metadata_exception );
  // Missing T separator
  EXPECT_THROW( fn( "20030201070809" );,  metadata_exception );
  // Non-numeric
  EXPECT_THROW( fn( "20030201T07081A" );, metadata_exception );
  // Non-numeric but tricky
  EXPECT_THROW( fn( "20030201T07081 " );, metadata_exception );

  // Invalid dates
  // Out-of-range year
  EXPECT_THROW( fn( "19690101T070809" );, metadata_exception );
  // Out-of-range month
  EXPECT_THROW( fn( "20031301T070809" );, metadata_exception );
  // Feb. 29 on not-a-leap-year
  EXPECT_THROW( fn( "20030229T070809" );, metadata_exception );

  // Valid dates (validated by epochconverter.com)
  // Epoch
  EXPECT_EQ( 0ull,                 fn( "19700101T000000" ) );
  // Random date
  EXPECT_EQ( 1044083289000000ull,  fn( "20030201T070809" ) );
  // Feb. 29 on a leap year
  EXPECT_EQ( 1583014942000000ull,  fn( "20200229T222222" ) );
  // Random date
  EXPECT_EQ( 1600000000000000ull,  fn( "20200913T122640" ) );
  // Date far in future
  EXPECT_EQ( 32503679999000000ull, fn( "29991231T235959" ) );
}
