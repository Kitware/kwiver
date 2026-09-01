// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test the pluggable implementation macros

#include <vital/plugin_management/pluggable_macro_magic.h>

#include <gtest/gtest.h>

#include <string>

namespace {

// An implementation whose registered name matches its class name.
struct plain_impl
{
  PLUGGABLE_IMPL_BASIC( plain_impl, "a plain implementation" )
};

// An implementation whose registered name is given explicitly and differs
// from the class name.
struct renamed_impl
{
  PLUGGABLE_IMPL_BASIC_NAMED(
    renamed_impl, "custom_name", "a renamed implementation" )
};

} // namespace

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( pluggable_macros, basic_name_is_class_name )
{
  EXPECT_EQ( "plain_impl", plain_impl::plugin_name() );
  EXPECT_EQ( "a plain implementation", plain_impl::plugin_description() );
}

// ----------------------------------------------------------------------------
TEST ( pluggable_macros, named_overrides_the_class_name )
{
  // The whole point of the _NAMED variant: the registered name is decoupled
  // from the class name.
  EXPECT_EQ( "custom_name", renamed_impl::plugin_name() );
  EXPECT_NE( "renamed_impl", renamed_impl::plugin_name() );
  EXPECT_EQ( "a renamed implementation", renamed_impl::plugin_description() );
}
