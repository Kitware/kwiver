// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/types/metadata_types.h>

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
