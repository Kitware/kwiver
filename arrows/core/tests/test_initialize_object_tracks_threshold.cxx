// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/initialize_object_tracks_threshold.h>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

#include <algorithm>

using namespace kwiver::vital;
using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( initialize_object_tracks_threshold, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm<
      algo::initialize_object_tracks >( "threshold" ) );
}

// ----------------------------------------------------------------------------
TEST ( initialize_object_tracks_threshold, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    initialize_object_tracks_threshold,
    "Perform thresholding on detection confidence values to create tracks.",
    PARAM_DEFAULT(
      max_new_tracks, size_t,
      "Maximum number of new tracks to initialize on a single frame.",
      10000 ),
    PARAM(
      filter, vital::algo::detected_object_filter_sptr,
      "filter" )
  );
}
