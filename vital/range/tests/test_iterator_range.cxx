// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test iterator range.

#include "test_values.h"

#include <vital/range/iterator_range.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( iterator_range, equal )
{
  auto const begin = test_values;
  auto const end = test_values + sizeof( test_values ) / sizeof( int );
  auto const test_range = range::iterator_range< int const* >{ begin, end };

  auto it = begin;
  for( auto const x : test_range )
  {
    EXPECT_EQ( *it, x );
    ++it;
  }
  EXPECT_EQ( end, it );
}
