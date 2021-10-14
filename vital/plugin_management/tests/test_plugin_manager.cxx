// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/plugin_management/plugin_manager.h>

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
// By default the plugin manager should have at least one search path.
TEST ( plugin_manager, default_search_paths )
{
  plugin_manager& vpm = plugin_manager::instance();
  EXPECT_GT( vpm.search_path().size(), 0 );
}

//// ----------------------------------------------------------------------------

// Tests to add
//
// - Load known file and test to see if contents are as expected.
// - Test API

// - test reload by loading a set of plugins, add one more plugin,
// - test for that plugin(present), reload plugins, test for plugin(not there)
//
