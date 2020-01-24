/*ckwg +29
 * Copyright 2017-2018 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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
 * @file
 * @brief Implementation of KPF activities
 *
 */

#include "track_kpf_activity.h"

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

#include <arrows/kpf/yaml/kpf_reader.h>
#include <arrows/kpf/yaml/kpf_yaml_writer.h>
#include <arrows/kpf/yaml/kpf_yaml_parser.h>
#include <arrows/kpf/yaml/kpf_canonical_io_adapter.h>
#include <arrows/kpf/yaml/kpf_canonical_types.h>

#include <track_oracle/utils/logging_map.h>
#include <track_oracle/file_formats/kpf_utils/kpf_utils.h>

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>
#include <sstream>

using std::map;
using std::make_pair;
using std::string;
using std::ostringstream;
using std::vector;

namespace KPF=::kwiver::vital::kpf;

namespace // anon
{

using namespace kwiver::track_oracle;

bool
build_lookup_map( const track_handle_list_type& ref_tracks,
                  map< unsigned, track_handle_type >& lookup_map )
{
  track_field< dt::tracking::external_id > id_field;
  for (size_t i=0; i<ref_tracks.size(); ++i)
  {
    auto probe = id_field.get( ref_tracks[i].row );
    if ( ! probe.first )
    {
      LOG_ERROR( main_logger, "KPF activity reference track without ID1?" );
      return false;
    }
    auto insert_test = lookup_map.insert( make_pair( probe.second, ref_tracks[i] ));
    if ( ! insert_test.second )
    {
      LOG_ERROR( main_logger, "KPF activity: duplicate track IDs " << probe.second );
      return false;
    }
  }
  return true;
}

struct kpf_actor_track_type: public track_base< kpf_actor_track_type >
{
  // empty, just exists to allow frames to be created on it
};

struct kpf_act_exception
{
  explicit kpf_act_exception( const string& msg ): what(msg) {}
  string what;
};

} // anon namespace

namespace kwiver {
namespace track_oracle {

track_handle_list_type
track_kpf_activity_type
::filter_on_activity_domain( const track_handle_list_type& source_activity_tracks,
                             int kpf_activity_domain )
{
  track_handle_list_type ret;
  track_field< dt::events::kpf_activity_domain > domain;
  for (const auto& a: source_activity_tracks )
  {
    if (domain( a.row ) == kpf_activity_domain )
    {
      ret.push_back( a );
    }
  }
  return ret;
}


bool
track_kpf_activity_type
::apply( const track_handle_list_type& act_tracks,
         const track_handle_list_type& source_geometry_tracks )
{
  track_kpf_activity_type act_schema;
  track_field< dt::tracking::frame_number > fn_field;
  kpf_actor_track_type actor_track_instance;

  logging_map_type wmsgs( main_logger, KWIVER_LOGGER_SITE );

  bool all_okay = true;

  // build lookup table from ID to track handle

  map< dt::tracking::external_id::Type, track_handle_type > lookup_table;
  if ( ! build_lookup_map( source_geometry_tracks, lookup_table ))
  {
    throw kpf_act_exception("id->track handle lookup failure");
  }

  try
  {
    for (const auto& activity: act_tracks)
    {
      //
      // for each actor, clone over the track and geometry within its time window
      //

      vector< dt::tracking::external_id::Type > missing;
      track_handle_list_type actor_tracks;

      auto act_int = act_schema( activity ).actor_intervals.get( activity.row );
      for (auto const& a: act_int.second)
      {
        auto id = static_cast<dt::tracking::external_id::Type>( a.track );
        auto id_probe = lookup_table.find( id );

        //
        // remember and log any missing actors in the ref set
        //
        if (id_probe == lookup_table.end())
        {
          missing.push_back( id );
          continue;
        }

        // create a new track; clone non-system fields
        track_handle_type new_track( track_oracle_core::get_next_handle() );
        if ( ! track_oracle_core::clone_nonsystem_fields( id_probe->second, new_track ))
        {
          ostringstream oss;
          oss << "Couldn't clone non-system track fields for " << id << "?";
          throw kpf_act_exception( oss.str() );
        }

        // copy over frames in the actor's time window
        wmsgs.add_msg( "Selecting relevant actor frames based on frame_number only" );
        for (const auto& src_frame: track_oracle_core::get_frames( id_probe->second ))
        {
          auto fn_probe = fn_field.get( src_frame.row );
          if (! fn_probe.first)
          {
            ostringstream oss;
            oss << "No frame number for frame in track " << id << "?";
            throw kpf_act_exception( oss.str() );
          }
          auto frame_num = fn_probe.second;

          // is the frame within the time window?
          if ((a.start.get_frame() <= frame_num) && (frame_num <= a.stop.get_frame()))
          {
            //
            // create a frame on the new track and clone the fields
            //
            frame_handle_type new_frame = actor_track_instance( new_track ).create_frame();
            if ( ! track_oracle_core::clone_nonsystem_fields( src_frame, new_frame ))
            {
              ostringstream oss;
              oss << "Couldn't clone non-system fields for track / frame " << id
                  << " / " << frame_num << "?";
              throw kpf_act_exception( oss.str() );
            }
          } // ... within time window
        } // ...for all source frames

        actor_tracks.push_back( new_track );
      } // ...for all actor tracks

      //
      // Did we miss anybody?
      //

      if ( ! missing.empty() )
      {
        ostringstream oss;
        oss << "Activity " << act_schema( activity ).activity_id()
            << " missing the following tracks:";
        for (auto m: missing) oss << " " << m;
        throw kpf_act_exception( oss.str() );
      }

      //
      // okay then-- add the actor tracks to the activity
      //

      act_schema( activity ).actor_tracks() = actor_tracks;

    } // ... for each activity row
  } // ... try

  catch (const kpf_act_exception& e )
  {
    LOG_ERROR( main_logger, "Error loading applying activities to geometry: " << e.what );
    all_okay = false;
  }
  //
  // anything to report?
  //

  if (! wmsgs.empty() )
  {
    LOG_INFO( main_logger, "KPF act reader: warnings begin" );
    wmsgs.dump_msgs();
    LOG_INFO( main_logger, "KPF act reader: warnings end" );
  }
  return all_okay;
}



} // ...track_oracle
} // ...kwiver
