// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test interval template class.

#include <vital/util/interval.h>

#include <gtest/gtest.h>

#include <limits>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
constexpr auto fnan = std::numeric_limits< float >::quiet_NaN();
constexpr auto dnan = std::numeric_limits< double >::quiet_NaN();
constexpr auto dinf = std::numeric_limits< double >::infinity();

// ----------------------------------------------------------------------------
TEST ( interval, construct )
{
  {
    auto const test_interval = interval< double >{ 5.0, 7.0 };
    EXPECT_EQ( 5.0, test_interval.lower() );
    EXPECT_EQ( 7.0, test_interval.upper() );
  }

  {
    auto const test_interval = interval< int >{ 9, 2 };
    EXPECT_EQ( 2, test_interval.lower() );
    EXPECT_EQ( 9, test_interval.upper() );
    EXPECT_EQ( interval< int >( 2, 9 ), test_interval );
  }

  {
    auto const test_interval = interval< double >{ dinf, -dinf };
    EXPECT_EQ( -dinf, test_interval.lower() );
    EXPECT_EQ( dinf, test_interval.upper() );
  }

  {
    EXPECT_THROW( interval< float >( fnan, 5.0f ), std::invalid_argument );
    EXPECT_THROW( interval< double >( 1.0, dnan ), std::invalid_argument );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval, truncate )
{
  {
    auto test_interval = interval< double >{ 0.0, 15.0 };
    test_interval.truncate_lower( -1.0 );
    EXPECT_EQ( interval< double >( 0.0, 15.0 ), test_interval );
    test_interval.truncate_lower( 0.0 );
    EXPECT_EQ( interval< double >( 0.0, 15.0 ), test_interval );
    test_interval.truncate_lower( 5.0 );
    EXPECT_EQ( interval< double >( 5.0, 15.0 ), test_interval );
    test_interval.truncate_lower( 15.0 );
    EXPECT_EQ( interval< double >( 15.0, 15.0 ), test_interval );
    EXPECT_THROW( test_interval.truncate_lower( 16.0 ),
                  std::invalid_argument );
    EXPECT_THROW( test_interval.truncate_lower( dnan ),
                  std::invalid_argument );
  }

  {
    auto test_interval = interval< double >{ 0.0, 15.0 };
    test_interval.truncate_upper( 20.0 );
    EXPECT_EQ( interval< double >( 0.0, 15.0 ), test_interval );
    test_interval.truncate_upper( 15.0 );
    EXPECT_EQ( interval< double >( 0.0, 15.0 ), test_interval );
    test_interval.truncate_upper( 5.0 );
    EXPECT_EQ( interval< double >( 0.0, 5.0 ), test_interval );
    test_interval.truncate_upper( 0.0 );
    EXPECT_EQ( interval< double >( 0.0, 0.0 ), test_interval );
    EXPECT_THROW( test_interval.truncate_upper( -1.0 ),
                  std::invalid_argument );
    EXPECT_THROW( test_interval.truncate_upper( dnan ),
                  std::invalid_argument );
  }

  {
    auto test_interval = interval< double >{ -dinf, dinf };
    test_interval.truncate_lower( -10.0 );
    EXPECT_EQ( interval< double >( -10.0, dinf ), test_interval );
    test_interval.truncate_upper( 10.0 );
    EXPECT_EQ( interval< double >( -10.0, 10.0 ), test_interval );
    test_interval.truncate_lower( -dinf );
    EXPECT_EQ( interval< double >( -10.0, 10.0 ), test_interval );
    test_interval.truncate_upper( dinf );
    EXPECT_EQ( interval< double >( -10.0, 10.0 ), test_interval );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval, encompass )
{
  auto test_interval = interval< double >{ 0.0, 15.0 };
  test_interval.encompass( 5.0 );
  EXPECT_EQ( interval< double >( 0.0, 15.0 ), test_interval );
  test_interval.encompass( -100.0 );
  EXPECT_EQ( interval< double >( -100.0, 15.0 ), test_interval );
  test_interval.encompass( 100.0 );
  EXPECT_EQ( interval< double >( -100.0, 100.0 ), test_interval );
  test_interval.encompass( -dinf );
  EXPECT_EQ( interval< double >( -dinf, 100.0 ), test_interval );
  test_interval.encompass( dinf );
  EXPECT_EQ( interval< double >( -dinf, dinf ), test_interval );
  EXPECT_THROW( test_interval.encompass( dnan ), std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( interval, contains )
{
  EXPECT_FALSE( interval< int >( 0, 0 ).contains( 0 ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( 5 ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( 6 ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( -2 ) );
  EXPECT_TRUE(  interval< int >( -1, 5 ).contains( 4 ) );
  EXPECT_TRUE(  interval< int >( -1, 5 ).contains( -1 ) );
  EXPECT_TRUE(  interval< double >( -dinf, dinf ).contains( 100.0 ) );
  EXPECT_FALSE( interval< double >( -dinf, dinf ).contains( dinf ) );
  EXPECT_FALSE( interval< double >( -dinf, dinf ).contains( dnan ) );

  EXPECT_TRUE(  interval< int >( 0, 0 ).contains( 0, true, true ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( 5, false, false ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( 6, false, true ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( -2, true, true ) );
  EXPECT_TRUE(  interval< int >( -1, 5 ).contains( 4, false, false ) );
  EXPECT_FALSE( interval< int >( -1, 5 ).contains( -1, false, true ) );
  EXPECT_TRUE(  interval< double >( 0, dinf )
                  .contains( dinf, false, true ) );
  EXPECT_FALSE( interval< double >( -dinf, 0 )
                  .contains( -dinf, false, true ) );
  EXPECT_FALSE( interval< double >( -dinf, dinf )
                  .contains( dnan, true, true ) );
}
