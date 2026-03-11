// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 *
 * \brief Various functions for creating a collections of tracks for running
 * tests
 *
 * These functions are shared by various tests
 */

#ifndef KWIVER_TEST_TEST_TRACKS_H_
#define KWIVER_TEST_TEST_TRACKS_H_

#include <random>

#include <vital/types/track_set.h>

namespace kwiver {

namespace testing {

// Generate a set of generic tracks
//
// paramters are:
//   frames - total number of frames to span
//   max_tracks_per_frame - maximum number of track states per frame
//   min_tracks_per_frame - minimum number of track states per frame
//   termination_fraction - fraction of tracks to terminate on each frame
//   skip_fraction - fraction of tracks to miss a state on each frame
//   frame_drop_fraction - fraction of frames with no tracks (skipped)
//
//  if the number of tracks drops below min_tracks_per_frame, create new
//  tracks to achieve max_tracks_per_frame.
kwiver::vital::track_set_sptr
generate_tracks(
  size_t frames = 100,
  size_t max_tracks_per_frame = 1000,
  size_t min_tracks_per_frame = 500,
  double termination_fraction = 0.1,
  double skip_fraction = 0.01,
  double frame_drop_fraction = 0.01 )
{
  using namespace kwiver::vital;

  std::default_random_engine rand_gen( 0 );
  std::uniform_real_distribution< double > uniform_dist( 0.0, 1.0 );

  track_id_t track_id = 0;
  std::vector< track_sptr > all_tracks, active_tracks;
  for( size_t f = 0; f < frames; ++f )
  {
    // randomly decide to skip some frames
    if( uniform_dist( rand_gen ) < frame_drop_fraction )
    {
      continue;
    }

    if( active_tracks.size() < min_tracks_per_frame )
    {
      // create tracks as needed to get enough on this frame
      while( active_tracks.size() < max_tracks_per_frame )
      {
        auto t = track::create();
        t->set_id( track_id++ );
        active_tracks.push_back( t );
        all_tracks.push_back( t );
      }
    }

    // add a state for each track to this frame
    for( auto t : active_tracks )
    {
      if( t->empty() || uniform_dist( rand_gen ) >= skip_fraction )
      {
        t->append( std::make_shared< track_state >( f ) );
      }
    }

    // randomly select tracks to terminate
    std::vector< track_sptr > next_tracks;
    for( auto t : active_tracks )
    {
      if( uniform_dist( rand_gen ) >= termination_fraction )
      {
        next_tracks.push_back( t );
      }
    }
    active_tracks.swap( next_tracks );
  }
  return std::make_shared< track_set >( all_tracks );
}

// ----------------------------------------------------------------------------
// Helper function to generate deterministic track set
// paramters are:
//  frames - total number of frames to span
//  max_tracks_per_frame - maximum number of track states per frame

//  Manually drops:
//    tracks 0, 1, 3 from frame 1
//    tracks 2, 7 from frame 2
//    tracks 5, 9 from frame 4

kwiver::vital::track_set_sptr
gen_set_tracks(
  size_t frames = 100,
  size_t max_tracks_per_frame = 1000 )
{
  using namespace kwiver::vital;

  // Manually terminate tracks on frames 1, 2 and 4
  track_id_t track_id = 0;
  std::vector< track_sptr > all_tracks, active_tracks;
  for( size_t f = 0; f < frames; ++f )
  {
    // Create tracks as needed to get enough on this frame
    while( active_tracks.size() < max_tracks_per_frame )
    {
      auto t = track::create();
      t->set_id( track_id++ );
      active_tracks.push_back( t );
      all_tracks.push_back( t );
    }

    // Add a state for each track to this frame
    for( auto t : active_tracks )
    {
      t->append( std::make_shared< track_state >( f ) );
    }

    if( f == 0 )
    {
      // Terminate tracks 0 and 3 on frame 1
      std::vector< track_sptr > next_tracks;
      for( auto t : active_tracks )
      {
        if( t->id() != 0 && t->id() != 3 )
        {
          next_tracks.push_back( t );
        }
      }
      active_tracks.swap( next_tracks );
    }

    if( f == 1 )
    {
      // Terminate tracks 2 and 7 on frame 2
      std::vector< track_sptr > next_tracks;
      for( auto t : active_tracks )
      {
        if( t->id() != 2 && t->id() != 7 )
        {
          next_tracks.push_back( t );
        }
      }
      active_tracks.swap( next_tracks );
    }

    if( f == 3 )
    {
      // Terminate tracks 5 and 9 on frame 4
      std::vector< track_sptr > next_tracks;
      for( auto t : active_tracks )
      {
        if( t->id() != 5 && t->id() != 9 )
        {
          next_tracks.push_back( t );
        }
      }
      active_tracks.swap( next_tracks );
    }
  }
  return std::make_shared< track_set >( all_tracks );
}

} // end namespace testing

} // end namespace kwiver

#endif
