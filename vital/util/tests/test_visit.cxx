// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Test visit utility functions

#include <vital/util/visit.h>

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
struct print_visitor
{
  template < class T >
  void
  operator()() const
  {
    ss << static_cast< T >( value );
  }

  std::stringstream& ss;
  int value;
};

// ----------------------------------------------------------------------------
struct to_string_visitor
{
  template < class T >
  std::string
  operator()() const
  {
    std::stringstream ss;
    ss << static_cast< T >( value );
    return ss.str();
  }

  int value;
};

// ----------------------------------------------------------------------------
TEST ( visit, visit_types )
{
  auto const test_fn = visit_types< print_visitor, int, char >;
  {
    std::stringstream ss;
    test_fn( { ss, 75 }, typeid( int ) );
    EXPECT_EQ( "75", ss.str() );
  }

  {
    std::stringstream ss;
    test_fn( { ss, 75 }, typeid( char ) );
    EXPECT_EQ( "K", ss.str() );
  }

  {
    std::stringstream ss;
    EXPECT_THROW( test_fn( { ss, 75 }, typeid( unsigned int ) ),
                  std::out_of_range );
  }
}

// ----------------------------------------------------------------------------
TEST ( visit, visit_types_return )
{
  auto const test_fn =
    &visit_types_return< std::string, to_string_visitor, int, char >;
  {
    auto const result = test_fn( { 75 }, typeid( int ) );
    EXPECT_EQ( "75", result );
  }

  {
    auto const result = test_fn( { 75 }, typeid( char ) );
    EXPECT_EQ( "K", result );
  }

  {
    std::stringstream ss;
    EXPECT_THROW( test_fn( { 75 }, typeid( unsigned int ) ),
                  std::out_of_range );
  }
}

using variant_t = std::variant< int, char >;

// ----------------------------------------------------------------------------
TEST ( visit, visit_variant_types )
{
  auto const test_fn = &visit_variant_types< variant_t, print_visitor >;
  {
    std::stringstream ss;
    test_fn( { ss, 75 }, typeid( int ) );
    EXPECT_EQ( "75", ss.str() );
  }

  {
    std::stringstream ss;
    test_fn( { ss, 75 }, typeid( char ) );
    EXPECT_EQ( "K", ss.str() );
  }

  {
    std::stringstream ss;
    EXPECT_THROW( test_fn( { ss, 75 }, typeid( unsigned int ) ),
                  std::out_of_range );
  }
}

// ----------------------------------------------------------------------------
TEST ( visit, visit_variant_types_return )
{
  auto const test_fn =
    &visit_variant_types_return< std::string, variant_t, to_string_visitor >;
  {
    auto const result = test_fn( { 75 }, typeid( int ) );
    EXPECT_EQ( "75", result );
  }

  {
    auto const result = test_fn( { 75 }, typeid( char ) );
    EXPECT_EQ( "K", result );
  }

  {
    std::stringstream ss;
    EXPECT_THROW( test_fn( { 75 }, typeid( unsigned int ) ),
                  std::out_of_range );
  }
}
