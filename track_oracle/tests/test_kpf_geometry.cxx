/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Test reading KPF tracks
 */

#include <gtest/gtest.h>
#include <test_gtest.h>

#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/core/track_field.h>
#include <track_oracle/data_terms/data_terms.h>
#include <track_oracle/file_formats/file_format_manager.h>

#include <map>
#include <utility>

namespace to = ::kwiver::track_oracle;
namespace dt = ::kwiver::track_oracle::dt;

using std::string;
using std::map;
using std::pair;

namespace { //anon

string g_data_dir;

map< dt::tracking::external_id::Type, to::track_handle_type >
map_track_ids_to_handles( const to::track_handle_list_type& tracks )
{
  to::track_field< dt::tracking::external_id > id;
  map< dt::tracking::external_id::Type, to::track_handle_type > m;
  for (const auto& h: tracks )
  {
    m[ id(h.row) ] = h;
  }
  return m;
}

map< dt::tracking::frame_number::Type, to::frame_handle_type >
map_frame_num_to_handles( const to::frame_handle_list_type& frames )
{
  to::track_field< dt::tracking::frame_number > frame_num;
  map< dt::tracking::frame_number::Type, to::frame_handle_type > m;
  for (const auto& h: frames )
  {
    m[ frame_num(h.row) ] = h;
  }
  return m;
}
} // ...anon


// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  GET_ARG(1, g_data_dir);
  return RUN_ALL_TESTS();
}

// ------------------------------------------------------------------
TEST(track_oracle, file_globs)
{
  const pair<size_t, string> test_globs[] = { { 1, "*.kw18" },
                                              { 1, "*.geom.yml" },
                                              { 2, "*.viratdata.events.txt" },
                                              { 0, "*.nothing_matches_me" } };
  for (const auto& g: test_globs)
  {
    auto matching_formats = to::file_format_manager::globs_match( g.second );
    EXPECT_EQ( matching_formats.size(), g.first ) << "Glob '" << g.second << "' matches " << g.first << " formats";
  }
}

// ------------------------------------------------------------------
TEST(track_oracle, kpf_geometry)
{

  to::track_handle_list_type kpf_tracks, kw18_tracks;
  to::track_field< dt::tracking::bounding_box > bbox_field;

  // load the kw18 reference tracks
  {
    string fn = g_data_dir+"/generic_tracks.kw18";
    bool rc = to::file_format_manager::read( fn, kw18_tracks );
    EXPECT_TRUE( rc ) << " reading from '" << fn << "'";
  }

  // load the KPF tracks (should have the same content)
  {
    string fn = g_data_dir+"/generic_tracks.geom.yml";
    bool rc = to::file_format_manager::read( fn, kpf_tracks );
    EXPECT_TRUE( rc ) << " reading from '" << fn << "'";
  }

  // same number of tracks?
  EXPECT_EQ( kpf_tracks.size(), kpf_tracks.size() ) << " same number of tracks in kw18 and kpf";

  auto kw18_map = map_track_ids_to_handles( kw18_tracks );
  auto kpf_map = map_track_ids_to_handles( kpf_tracks );

  // catch duplicated IDs
  EXPECT_EQ( kpf_tracks.size(), kpf_map.size() ) << " same number of KPF tracks in map";
  EXPECT_EQ( kw18_tracks.size(), kw18_map.size() ) << " same number of kw18 tracks in map";

  for (const auto& kw18: kw18_map )
  {
    auto kw18_id = kw18.first;
    auto kpf_probe = kpf_map.find( kw18_id );
    EXPECT_TRUE( kpf_probe != kpf_map.end() ) << " kw18 track " << kw18_id << " is in KPF";
    if ( kpf_probe == kpf_map.end() )
    {
      continue;
    }

    auto kw18_h = kw18_map[ kw18_id ];
    auto kpf_h = kpf_map[ kw18_id ];
    auto kw18_frames = to::track_oracle_core::get_frames( kw18_h );
    auto kpf_frames = to::track_oracle_core::get_frames( kpf_h );
    auto n_kw18_frames( kw18_frames.size() ), n_kpf_frames( kpf_frames.size() );
    EXPECT_EQ( n_kw18_frames, n_kpf_frames ) << " track " << kw18_id << " have the same number of frames";
    if ( n_kw18_frames != n_kpf_frames )
    {
      continue;
    }

    auto kw18_frame_map = map_frame_num_to_handles( kw18_frames );
    auto kpf_frame_map = map_frame_num_to_handles( kpf_frames );

    // catch duplicated frame numbers
    EXPECT_EQ( n_kw18_frames, kw18_frame_map.size() ) << " track " << kw18_id << " same number of kw18 frames in map";
    EXPECT_EQ( n_kpf_frames, kpf_frame_map.size() ) << " track " << kw18_id << " same number of kpf frames in map";

    for (const auto& kw18_f: kw18_frame_map )
    {
      auto kw18_frame_num = kw18_f.first;
      auto kpf_frame_probe = kpf_frame_map.find( kw18_frame_num );
      EXPECT_TRUE( kpf_frame_probe != kpf_frame_map.end() ) << " kw18 track " << kw18_id << " frame " << kw18_frame_num << " exists in kpf";
      if ( kpf_frame_probe == kpf_frame_map.end() )
      {
        continue;
      }

      // actually, the only thing we carry over is the bounding box...

      auto kw18_box = bbox_field( kw18_f.second.row );
      auto kpf_box = bbox_field( kpf_frame_probe->second.row );


      EXPECT_NEAR( kw18_box.min_x(), kpf_box.min_x(), 1.0e-8 ) << " track " << kw18_id << " frame " << kw18_frame_num;
      EXPECT_NEAR( kw18_box.min_y(), kpf_box.min_y(), 1.0e-8 ) << " track " << kw18_id << " frame " << kw18_frame_num;
      EXPECT_NEAR( kw18_box.max_x(), kpf_box.max_x(), 1.0e-8 ) << " track " << kw18_id << " frame " << kw18_frame_num;
      EXPECT_NEAR( kw18_box.max_y(), kpf_box.max_y(), 1.0e-8 ) << " track " << kw18_id << " frame " << kw18_frame_num;

    }

  } // ...for each track
}

// ------------------------------------------------------------------
TEST(track_oracle, kpf_load_long_ids)
{
  to::track_handle_list_type kpf_tracks;
  string fn = g_data_dir+"/2018-03-07.16-55-06.17-00-06.school.G336.geom.yml";
  bool rc = to::file_format_manager::read( fn, kpf_tracks );
  EXPECT_TRUE( rc ) << " reading from '" << fn << "'";
  size_t n_read = kpf_tracks.size();
  EXPECT_EQ( n_read, 118 ) << " number of tracks read";
}
