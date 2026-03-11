// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/filter_tracks.h>
#include <vital/plugin_management/plugin_manager.h>

#include <arrows/core/match_matrix.h>
#include <test_tracks.h>
#include <vital/tests/test_track_set.h>

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

namespace {

// ----------------------------------------------------------------------------
// Establish constants and values for randomly generated track set

// Generate instance of filter function
filter_tracks filter_fn;

// Declare empty frames object
std::vector< frame_id_t > frames;

// These parameters can be varied for further testing
size_t const num_frames = 100;
size_t const max_tracks = 1000;

track_set_sptr test_tracks =
  kwiver::testing::generate_tracks( num_frames, max_tracks );

// Compute values for filtered large track set
track_set_sptr filtered_large_trk_set =
  filter_fn.filter( test_tracks );

// Calculate filtered match matrix
Eigen::SparseMatrix< size_t > filtered_large_mm =
  kwiver::arrows::match_matrix( filtered_large_trk_set, frames );

// Compute filtered importance scores
std::map< track_id_t, double > filtered_large_importance_scores =
  kwiver::arrows::match_matrix_track_importance(
    filtered_large_trk_set,
    frames, filtered_large_mm );

// ----------------------------------------------------------------------------
// Establish constants and values for small, deterministic track set
// DO NOT EDIT these two constants, might cause unit tests to fail
auto const set_num_frames = 5;
auto const set_max_tracks = 8;

track_set_sptr set_tracks =
  kwiver::testing::gen_set_tracks( set_num_frames, set_max_tracks );

// compute values for filtered track stet
track_set_sptr filtered_small_trk_set =
  filter_fn.filter( set_tracks );

} // end namespace anonymous

// ----------------------------------------------------------------------------
TEST ( filter_tracks, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, create_algorithm< algo::filter_tracks >( "core" ) );
}

// ----------------------------------------------------------------------------
TEST ( filter_tracks, track_ids )
{
  // Filter `set_tracks` should remain with track ids: 1, 4, 5, 6 and 8
  // based on default configuration parameters;
  // Length >= 3 and Importance Score >=1.0

  // These are the "set_tracks" that are to be filtered
  // Track ID: Length, Importance Score
  // Track 0: 1, 0.125
  // Track 1: 5, 2.66667
  // Track 2: 2, 0.416667
  // Track 3: 1, 0.125
  // Track 4: 5, 2.66667
  // Track 5: 4, 1.625
  // Track 6: 5, 2.66667
  // Track 7: 2, 0.416667
  // Track 8: 4, 1.54167
  // Track 9: 3, 0.833333
  // Track 10: 3, 0.833333
  // Track 11: 3, 0.833333
  // Track 12: 1, 0.125
  // Track 13: 1, 0.125

  std::set< track_id_t > expected_track_ids = { 1, 4, 5, 6, 8 };

  // Get the track IDs from the filtered track set
  std::set< track_id_t > filtered_track_ids;
  for( const auto& track : filtered_small_trk_set->tracks() )
  {
    filtered_track_ids.insert( track->id() );
  }

  EXPECT_EQ( filtered_track_ids, expected_track_ids );

  // Check number of filtered tracks from larger, random track set
  EXPECT_LE(
    filtered_large_trk_set->all_track_ids().size(),
    test_tracks->size() );
}

// ----------------------------------------------------------------------------
// Test that tracks are filtered out according to config parameters
TEST ( filter_tracks, config_params )
{
  algo::filter_tracks_sptr filter_algo =
    create_algorithm< algo::filter_tracks >( "core" );

  // Get the configuration of the filter_tracks algorithm
  config_block_sptr config = filter_algo->get_configuration();

  // Get the value of min_mm_importance parameter from the configuration
  const double threshold = config->get_value< double >( "min_mm_importance" );

  // Get the value of min_track_length parameter from the configuration
  auto const min_track_length = config->get_value< size_t >(
    "min_track_length" );

  // Check that the importance score is greater than the threshold
  for( const auto& entry : filtered_large_importance_scores )
  {
    EXPECT_GT( entry.second, threshold );
  }

  // Check that each filtered track has length >= to config value
  for( const auto& track : filtered_large_trk_set->tracks() )
  {
    EXPECT_GE( track->size(), min_track_length );
  }
}
