// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test interval_map template class.

#include <vital/util/interval_map.h>

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
constexpr auto finf = std::numeric_limits< float >::infinity();
constexpr auto fnan = std::numeric_limits< float >::quiet_NaN();
constexpr auto dinf = std::numeric_limits< double >::infinity();

// ----------------------------------------------------------------------------
TEST ( interval_map, construct )
{
  using map_type = interval_map< double, uint64_t >;

  // Construct empty
  EXPECT_EQ( map_type{}, map_type{} );

  // Empty intervals ignored
  EXPECT_EQ( map_type{}, map_type( { { { 0.0, 0.0 }, 1 } } ) );

  // Non-empty intervals not ignored
  EXPECT_NE( map_type{}, map_type( { { { 0.0, 0.1 }, 1 } } ) );

  // Equality test
  EXPECT_EQ( map_type( { { { 0.0, 0.1 }, 1 } } ),
             map_type( { { { 0.0, 0.1 }, 1 } } ) );

  // Intervals differentiate
  EXPECT_NE( map_type( { { { 0.0, 0.1 }, 1 } } ),
             map_type( { { { 0.0, 0.2 }, 1 } } ) );

  // Values differentiate
  EXPECT_NE( map_type( { { { 0.0, 0.1 }, 1 } } ),
             map_type( { { { 0.0, 0.1 }, 2 } } ) );

  // Order independent
  EXPECT_EQ( map_type( { { { 0.0, 0.1 }, 1 }, { { 0.1, 0.2 }, 2 } } ),
             map_type( { { { 0.1, 0.2 }, 2 }, { { 0.0, 0.1 }, 1 } } ) );

  // Adjacent intervals merge
  EXPECT_EQ( map_type( { { { 0.0, 0.2 }, 1 } } ),
             map_type( { { { 0.1, 0.2 }, 1 }, { { 0.0, 0.1 }, 1 } } ) );

  // Can't construct from overlapping intervals
  EXPECT_THROW( map_type( { { { 1.0, 2.0 }, 1 }, { { 1.5, 2.5 }, 1 } } ),
                std::invalid_argument );

  // Empty intervals still considered overlapping
  EXPECT_THROW( map_type( { { { 1.0, 2.0 }, 1 }, { { 1.5, 1.5 }, 1 } } ),
                std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( interval_map, size )
{
  using map_type = interval_map< int, int >;

  // Zero size
  {
    auto const test_map = map_type{};
    EXPECT_EQ( 0, test_map.size() );
    EXPECT_TRUE( test_map.empty() );
  }

  // Nonzero size
  {
    auto const test_map = map_type{ { { { 0, 1 }, 0 }, { { 1, 2 }, 1 } } };
    EXPECT_EQ( 2, test_map.size() );
    EXPECT_FALSE( test_map.empty() );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval_map, clear )
{
  using map_type = interval_map< int, int >;
  auto test_map = map_type{ { { { 0, 1 }, 0 }, { { 1, 2 }, 1 } } };
  test_map.clear();
  EXPECT_EQ( map_type{}, test_map );
}

// ----------------------------------------------------------------------------
TEST ( interval_map, iterators )
{
  using entry_type = interval_map_entry_t< int, int >;
  using map_type = interval_map< int, int >;
  {
    std::vector< entry_type > const entries = {
      { { 0, 1 }, 0 },
      { { 1, 2 }, 1 } };
    std::vector< entry_type > const modified_entries = {
      { { 0, 1 }, 1 },
      { { 1, 2 }, 2 } };

    // Construct from iterators
    auto test_map = map_type{ entries.cbegin(), entries.cend() };

    // Use iterators to modify values
    for( auto& entry : test_map )
    {
      ++entry.value;
    }

    // Use iterators to test that all entries have been modified
    EXPECT_TRUE( std::equal( test_map.cbegin(), test_map.cend(),
                            modified_entries.cbegin() ) );
  }

  {
    auto test_map = map_type{};
    EXPECT_EQ( test_map.cbegin(), test_map.cend() );
    EXPECT_EQ( test_map.begin(), test_map.end() );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval_map, find_point )
{
  using map_type = interval_map< float, int >;
  using opt_type = std::optional< int >;

  auto const empty_map = map_type{};
  EXPECT_EQ( std::nullopt, empty_map.at( 0.0f ) );

  auto const test_map = map_type{
    { { -finf, -100.0f }, -1 },
    { { 0.0f, 1.0f }, 0 },
    { { 1.0f, 5.0f }, 1 },
    { { 10.0f, finf }, 2 }, };

  // Key with no value
  EXPECT_EQ( std::nullopt, test_map.at( -1.0f ) );

  // Bottom of interval
  EXPECT_EQ( opt_type{ 0 }, test_map.at( 0.0f ) );
  EXPECT_EQ( opt_type{ 2 }, test_map.at( 10.0f ) );

  // Middle of interval
  EXPECT_EQ( opt_type{ 0 }, test_map.at( 0.5f ) );
  EXPECT_EQ( opt_type{ 1 }, test_map.at( 1.5f ) );
  EXPECT_EQ( opt_type{ 2 }, test_map.at( 100.0f ) );

  // Top of interval
  EXPECT_EQ( std::nullopt, test_map.at( 5.0f ) );

  // Between adjacent intervals
  EXPECT_EQ( opt_type{ 1 }, test_map.at( 1.0f ) );

  // Infinity
  // Notably, given the half-open nature of the ranges, it is impossible to
  // create an interval_map which will return a value for the largest possible
  // value of a given type (e.g. infinity, SIZE_MAX). Practically, unsure
  // whether this limitation is particularly pressing.
  EXPECT_EQ( opt_type{ -1 }, test_map.at( -finf ) );
  EXPECT_EQ( std::nullopt, test_map.at( finf ) );

  // NaN
  EXPECT_THROW( test_map.at( fnan ), std::invalid_argument );
}

// ----------------------------------------------------------------------------
TEST ( interval_map, find_interval )
{
  using map_type = interval_map< int, int >;

  auto const empty_map = map_type{};
  {
    auto const result = empty_map.find( { 0, 100 } );
    EXPECT_EQ( empty_map.end(), result.begin() );
    EXPECT_EQ( empty_map.end(), result.end() );
  }

  auto const test_map = map_type{
    { { 0, 5 }, 0 },
    { { 5, 10 }, 1 },
    { { 10, 12 }, 2 },
    { { 15, 20 }, 3 },
    { { 50, 100 }, 4 }, };

  // Point interval - inside
  {
    auto const result = test_map.find( { 2, 2 } );
    EXPECT_EQ( std::next( test_map.begin(), 0 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 1 ), result.end() );
  }

  // Point interval - outside
  {
    auto const result = test_map.find( { -1, -1 } );
    EXPECT_EQ( std::next( test_map.begin(), 0 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 0 ), result.end() );
  }

  // Point interval - top edge
  {
    auto const result = test_map.find( { 20, 20 } );
    EXPECT_EQ( std::next( test_map.begin(), 4 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 4 ), result.end() );
  }

  // Exact match
  {
    auto const result = test_map.find( { 5, 10 } );
    EXPECT_EQ( std::next( test_map.begin(), 1 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 2 ), result.end() );
  }

  // Exact match - multiple
  {
    auto const result = test_map.find( { 0, 12 } );
    EXPECT_EQ( std::next( test_map.begin(), 0 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 3 ), result.end() );
  }

  // No match
  {
    auto const result = test_map.find( { 100, 200 } );
    EXPECT_EQ( std::next( test_map.begin(), 5 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 5 ), result.end() );
  }

  // Non-contiguous match
  {
    auto const result = test_map.find( { 5, 50 } );
    EXPECT_EQ( std::next( test_map.begin(), 1 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 4 ), result.end() );
  }

  // Non-contiguous partial match
  {
    auto const result = test_map.find( { 7, 17 } );
    EXPECT_EQ( std::next( test_map.begin(), 1 ), result.begin() );
    EXPECT_EQ( std::next( test_map.begin(), 4 ), result.end() );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval_map, set )
{
  using map_type = interval_map< double, int >;

  auto const empty_map = map_type{};
  {
    auto test_map = empty_map;
    test_map.set( { 0.0, 100.0 }, 0 );
    EXPECT_EQ( map_type( { { { 0.0, 100.0 }, 0 } } ), test_map );
  }

  auto const basis_map = map_type{
    { { -dinf, -50.0 }, -1 },
    { { 0.0, 5.0 }, 0 },
    { { 5.0, 10.0 }, 1 },
    { { 10.0, 12.0 }, 2 },
    { { 15.0, 20.0 }, 3 },
    { { 50.0, 100.0 }, 4 }, };

  // Point interval - inside
  {
    auto test_map = basis_map;
    test_map.set( { 11.0, 11.0 }, 20 );
    EXPECT_EQ( basis_map, test_map );
  }

  // Point interval - outside
  {
    auto test_map = basis_map;
    test_map.set( { -1.0, -1.0 }, 20 );
    EXPECT_EQ( basis_map, test_map );
  }

  // Exact replacement
  {
    auto test_map = basis_map;
    test_map.set( { 5.0, 10.0 }, 20 );
    EXPECT_EQ(
      map_type( {
        { { -dinf, -50.0 }, -1 },
        { { 0.0, 5.0 }, 0 },
        { { 5.0, 10.0 }, 20 },
        { { 10.0, 12.0 }, 2 },
        { { 15.0, 20.0 }, 3 },
        { { 50.0, 100.0 }, 4 }, } ),
      test_map );
  }

  // Exact replacement - merge left
  {
    auto test_map = basis_map;
    test_map.set( { 5.0, 10.0 }, 0 );
    EXPECT_EQ(
      map_type( {
        { { -dinf, -50.0 }, -1 },
        { { 0.0, 10.0 }, 0 },
        { { 10.0, 12.0 }, 2 },
        { { 15.0, 20.0 }, 3 },
        { { 50.0, 100.0 }, 4 }, } ),
      test_map );
  }

  // Exact replacement - merge right
  {
    auto test_map = basis_map;
    test_map.set( { 5.0, 10.0 }, 2 );
    EXPECT_EQ(
      map_type( {
        { { -dinf, -50.0 }, -1 },
        { { 0.0, 5.0 }, 0 },
        { { 5.0, 12.0 }, 2 },
        { { 15.0, 20.0 }, 3 },
        { { 50.0, 100.0 }, 4 }, } ),
      test_map );
  }

  // Partial replacement
  {
    auto test_map = basis_map;
    test_map.set( { -100.0, -60.0 }, -2 );
    EXPECT_EQ(
      map_type( {
        { { -dinf, -100.0 }, -1 },
        { { -100, -60.0 }, -2 },
        { { -60.0, -50.0 }, -1 },
        { { 0.0, 5.0 }, 0 },
        { { 5.0, 10.0 }, 1 },
        { { 10.0, 12.0 }, 2 },
        { { 15.0, 20.0 }, 3 },
        { { 50.0, 100.0 }, 4 }, } ),
      test_map );
  }

  // Partial replacement - merge both sides
  {
    auto test_map = basis_map;
    test_map.set( { -100.0, -60.0 }, -1 );
    EXPECT_EQ( basis_map, test_map );
  }

  // Multiple replacement - merge right
  {
    auto test_map = basis_map;
    test_map.set( { -100.0, 60.0 }, 4 );
    EXPECT_EQ(
      map_type( {
        { { -dinf, -100.0 }, -1 },
        { { -100.0, 100.0 }, 4 }, } ),
      test_map );
  }

  // Full replacement
  {
    auto test_map = basis_map;
    test_map.set( { -dinf, dinf }, -1 );
    EXPECT_EQ(
      map_type( {
        { { -dinf, dinf }, -1 }, } ),
      test_map );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval_map, weak_set )
{
  using map_type = interval_map< int, int >;
  auto const basis_map = map_type{
    { { 0, 5 }, 0 },
    { { 5, 10 }, 1 },
    { { 10, 12 }, 2 },
    { { 15, 20 }, 3 },
    { { 50, 100 }, 4 } };

  // Point interval - inside
  {
    auto test_map = basis_map;
    test_map.weak_set( { 11, 11 }, 20 );
    EXPECT_EQ( basis_map, test_map );
  }

  // Point interval - outside
  {
    auto test_map = basis_map;
    test_map.weak_set( { -1, -1 }, 20 );
    EXPECT_EQ( basis_map, test_map );
  }

  // No effect across one entry
  {
    auto test_map = basis_map;
    test_map.weak_set( { 10, 12 }, 10 );
    EXPECT_EQ( basis_map, test_map );
  }

  // No effect across multiple entries
  {
    auto test_map = basis_map;
    test_map.weak_set( { 0, 12 }, 10 );
    test_map.weak_set( { 3, 11 }, 10 );
    EXPECT_EQ( basis_map, test_map );
  }

  // No existing entries inside
  {
    auto test_map = basis_map;
    test_map.weak_set( { 30, 40 }, 10 );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 30, 40 }, 10 },
        { { 50, 100 }, 4 } } ),
      test_map );
  }

  // Merge left
  {
    auto test_map = basis_map;
    test_map.weak_set( { 20, 50 }, 3 );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 50 }, 3 },
        { { 50, 100 }, 4 } } ),
      test_map );
  }

  // Merge right
  {
    auto test_map = basis_map;
    test_map.weak_set( { 20, 50 }, 4 );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 20, 100 }, 4 } } ),
      test_map );
  }

  // Exact match
  {
    auto test_map = basis_map;
    test_map.weak_set( { 50, 100 }, 10 );
    EXPECT_EQ( basis_map, test_map );
  }

  // Around one existing entry
  {
    auto test_map = basis_map;
    test_map.weak_set( { 40, 150 }, 10 );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 40, 50 }, 10 },
        { { 50, 100 }, 4 },
        { { 100, 150 }, 10 } } ),
      test_map );
  }

  // Full span - ends touching inside edges
  {
    auto test_map = basis_map;
    test_map.weak_set( { 5, 50 }, 3 );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 12, 50 }, 3 },
        { { 50, 100 }, 4 } } ),
      test_map );
  }

  // Full span - ends touching outside edges
  {
    auto test_map = basis_map;
    test_map.weak_set( { 0, 100 }, 3 );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 12, 50 }, 3 },
        { { 50, 100 }, 4 } } ),
      test_map );
  }

  // Full span - ends outside
  {
    auto test_map = basis_map;
    test_map.weak_set( { -200, 200 }, 3 );
    EXPECT_EQ(
      map_type( {
        { { -200, 0 }, 3 },
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 12, 50 }, 3 },
        { { 50, 100 }, 4 },
        { { 100, 200 }, 3 } } ),
      test_map );
  }
}

// ----------------------------------------------------------------------------
TEST ( interval_map, erase )
{
  using map_type = interval_map< int, int >;
  auto const basis_map = map_type{
    { { 0, 5 }, 0 },
    { { 5, 10 }, 1 },
    { { 10, 12 }, 2 },
    { { 15, 20 }, 3 },
    { { 50, 100 }, 4 }, };

  // Point iterator
  {
    auto test_map = basis_map;
    test_map.erase( test_map.cbegin() );
    EXPECT_EQ(
      map_type( {
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Iterator range - empty
  {
    auto test_map = basis_map;
    test_map.erase( test_map.cbegin(), test_map.cbegin() );
    EXPECT_EQ( basis_map, test_map );
  }

  // Iterator range
  {
    auto test_map = basis_map;
    test_map.erase( test_map.cbegin(), std::prev( test_map.cend() ) );
    EXPECT_EQ( map_type( { { { 50, 100 }, 4 }, } ), test_map );
  }

  // Interval - empty map
  auto const empty_map = map_type{};
  {
    auto test_map = empty_map;
    test_map.erase( { -100, 100 } );
    EXPECT_EQ( empty_map, test_map );
  }

  // Interval - overreach both sides
  {
    auto test_map = basis_map;
    test_map.erase( { 40, 120 } );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 }, } ),
      test_map );
  }

  // Interval - exact match
  {
    auto test_map = basis_map;
    test_map.erase( { 5, 10 } );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - underreach left
  {
    auto test_map = basis_map;
    test_map.erase( { 6, 10 } );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 6 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - underreach right
  {
    auto test_map = basis_map;
    test_map.erase( { 5, 9 } );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 9, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - underreach both sides
  {
    auto test_map = basis_map;
    test_map.erase( { 7, 9 } );
    EXPECT_EQ(
      map_type( {
        { { 0, 5 }, 0 },
        { { 5, 7 }, 1 },
        { { 9, 10 }, 1 },
        { { 10, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - multiple - overreach both sides
  {
    auto test_map = basis_map;
    test_map.erase( { -1, 14 } );
    EXPECT_EQ(
      map_type( {
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - multiple - exact match
  {
    auto test_map = basis_map;
    test_map.erase( { 0, 12 } );
    EXPECT_EQ(
      map_type( {
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - multiple - underreach both sides
  {
    auto test_map = basis_map;
    test_map.erase( { 3, 11 } );
    EXPECT_EQ(
      map_type( {
        { { 0, 3 }, 0 },
        { { 11, 12 }, 2 },
        { { 15, 20 }, 3 },
        { { 50, 100 }, 4 }, } ),
      test_map );
  }

  // Interval - no match
  {
    auto test_map = basis_map;
    test_map.erase( { 30, 35 } );
    EXPECT_EQ( basis_map, test_map );
  }

  // Interval - match everything
  {
    auto test_map = basis_map;
    test_map.erase( { std::numeric_limits< int >::lowest(),
                      std::numeric_limits< int >::max() } );
    EXPECT_EQ( map_type{}, test_map );
  }
}
