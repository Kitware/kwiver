/*ckwg +29
 * Copyright 2016-2018 by Kitware, Inc.
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
 * \brief Implementation for detected_object_set_input_kpf
 */

#include "detected_object_set_input_kpf.h"

#include "yaml/kpf_reader.h"
#include "yaml/kpf_yaml_parser.h"
#include "yaml/kpf_canonical_io_adapter.h"

#include "vital_kpf_adapters.h"

#include <vital/util/data_stream_reader.h>
#include <vital/exceptions.h>

#include <map>
#include <sstream>
#include <cstdlib>

namespace
{

struct track_activity_key_t
{
  unsigned long long ID1;  // track ID
  unsigned start_frame, end_frame;
  track_activity_key_t()
    : ID1(0), start_frame(0), end_frame(0) {}
  track_activity_key_t( unsigned long long id1, unsigned start, unsigned end )
    : ID1(id1), start_frame( start ), end_frame( end ) {}
};

bool operator<( const track_activity_key_t& lhs, const track_activity_key_t& rhs )
{
  if (lhs.ID1 == rhs.ID1)
  {
    if (lhs.start_frame == rhs.start_frame)
    {
      return lhs.end_frame < rhs.end_frame;
    }
    else
    {
      return lhs.start_frame < rhs.start_frame;
    }
  }
  else
  {
    return lhs.ID1 < rhs.ID1;
  }
}

} // ...anon

namespace kwiver {
namespace arrows {
namespace kpf {

struct null_kpf_act_adapter: public KPF::kpf_act_adapter< KPFC::activity_t >
{
  null_kpf_act_adapter():
    kpf_act_adapter< KPFC::activity_t > (
      []( const KPFC::activity_t& c, KPFC::activity_t& user ) {
        user = c;
      },
      []( const KPFC::activity_t& user) {
        return user;
      })
  {}
};

// ------------------------------------------------------------------
class detected_object_set_input_kpf::priv
{
public:
  priv( detected_object_set_input_kpf* parent)
    : m_parent( parent )
    , m_first( true )
    , m_kpf_types_path( "" )
    , m_kpf_activities_path( "" )
  { }

  ~priv() { }

  void read_types();
  void read_activities();
  void read_all();

  detected_object_set_input_kpf* m_parent;
  bool m_first;

  int m_current_idx;
  int m_last_idx;

  // full path to KPF types file
  std::string m_kpf_types_path;
  // full path to KPF activities file
  std::string m_kpf_activities_path;

  // Map of detected objects indexed by frame number. Each set
  // contains all detections for a single frame.
  std::map< int, kwiver::vital::detected_object_set_sptr > m_detected_sets;

  // Map of track ID1 to object type name
  std::map< unsigned long long, std::string > m_object_types;

  // Map of track segments to activity names
  std::map< track_activity_key_t, std::string > m_track_to_activity;
};


// ==================================================================
detected_object_set_input_kpf::
detected_object_set_input_kpf()
  : d( new detected_object_set_input_kpf::priv( this ) )
{
  attach_logger( "arrows.kpf.detected_object_set_input_kpf" );
}


detected_object_set_input_kpf::
~detected_object_set_input_kpf()
{
}


// ------------------------------------------------------------------
void
detected_object_set_input_kpf::
set_configuration(vital::config_block_sptr config)
{
  d->m_kpf_types_path = config->get_value<std::string>( "kpf_types_filepath", "" );
  d->m_kpf_activities_path = config->get_value<std::string>( "kpf_activities_filepath", "" );
}


// ------------------------------------------------------------------
bool
detected_object_set_input_kpf::
check_configuration(vital::config_block_sptr config) const
{
  return true;
}


// ------------------------------------------------------------------
bool
detected_object_set_input_kpf::
read_set( kwiver::vital::detected_object_set_sptr & set, std::string& image_name )
{
  if ( d->m_first )
  {
    // Read in all detections
    d->read_all();
    d->m_first = false;

    // set up iterators for returning sets.
    d->m_current_idx = d->m_detected_sets.begin()->first;
    d->m_last_idx = d->m_detected_sets.rbegin()->first;
  } // end first

  // we do not return image name
  image_name.clear();

  // return detection set at current index if there is one
  if ( 0 == d->m_detected_sets.count( d->m_current_idx ) )
  {
    // return empty set
    set = std::make_shared< kwiver::vital::detected_object_set>();
  }
  else
  {
    // Return detections for this frame.
    set = d->m_detected_sets[d->m_current_idx];
  }

  ++d->m_current_idx;

  return true;
}


// ------------------------------------------------------------------
void
detected_object_set_input_kpf::
new_stream()
{
  d->m_first = true;
}

std::pair< bool, std::string >
detected_object_set_input_kpf::
get_typestring_for_id( unsigned long long object_id ) const
{
  auto probe = d->m_object_types.find( object_id );
  if ( probe == d->m_object_types.end())
  {
    return std::make_pair( false, "" );
  }
  else
  {
    return std::make_pair( true, probe->second );
  }
}

std::pair< bool, std::string >
detected_object_set_input_kpf::
get_activityname_for_id_and_frame( unsigned long long object_id, unsigned frame_number ) const
{
  for (const auto& p: d->m_track_to_activity)
  {
    const auto& key = p.first;
    if ( (key.ID1 == object_id) &&
         (key.start_frame <= frame_number) &&
         (frame_number <= key.end_frame))
    {
      return std::make_pair(true, p.second);
    }
  }
  return std::make_pair( false, "" );
}



// ==================================================================
void
detected_object_set_input_kpf::priv::
read_types()
{
  m_object_types.clear();
  if (m_kpf_types_path.empty())
  {
    LOG_INFO( m_parent->logger(), "No KPF types file set; no type information available" );
    return;
  }

  std::ifstream is( m_kpf_types_path.c_str() );
  if ( ! is )
  {
    LOG_ERROR( m_parent->logger(), "Couldn't open KPF types file '" << m_kpf_types_path
               << "' for reading; no type information available" );
    return;
  }

  KPF::kpf_yaml_parser_t parser( is );
  KPF::kpf_reader_t reader( parser );

  unsigned long long id1(0);
  KPFC::cset_t cset;

  // accept either domain 2 or 3
  while ( reader
          >> KPF::reader< KPFC::id_t >( id1, KPFC::id_t::TRACK_ID )
          >> KPF::reader< KPFC::cset_t >( cset, KPF::packet_header_t::ANY_DOMAIN ))
  {
    // find the cset with confidence 1.0
    auto probe = std::find_if( cset.d.cbegin(),
                               cset.d.cend(),
                               []( const std::pair< std::string, double>& p ) { return p.second == 1.0; });
    if (probe == cset.d.cend())
    {
      LOG_ERROR( m_parent->logger(), "KPF types file '" << m_kpf_types_path << "' "
                 << " ID1 of " << id1 << " cset does not have confidence 1.0?" );
    }
    else
    {
      m_object_types[ id1 ] = probe->first;
    }

    reader.flush();
  }
  LOG_INFO( m_parent->logger(), "Loaded " << m_object_types.size() << " KPF object types" );
}

// ==================================================================
void
detected_object_set_input_kpf::priv::
read_activities()
{
  m_track_to_activity.clear();
  if (m_kpf_activities_path.empty())
  {
    LOG_INFO( m_parent->logger(), "No KPF activities file set; no activities information available" );
    return;
  }

  std::ifstream is( m_kpf_activities_path.c_str() );
  if ( ! is )
  {
    LOG_ERROR( m_parent->logger(), "Couldn't open KPF activities file '" << m_kpf_types_path
               << "' for reading; no activities information available" );
    return;
  }

  KPF::kpf_yaml_parser_t parser( is );
  KPF::kpf_reader_t reader( parser );

  null_kpf_act_adapter adapter;
  KPFC::activity_t act;

  // accept any domain for now
  while ( reader
          >> KPF::reader< KPFC::activity_t >( adapter, KPF::packet_header_t::ANY_DOMAIN ))
  {
    adapter.get( act );
    // lookup the activity type
    auto probe = std::find_if( act.activity_labels.d.cbegin(),
                               act.activity_labels.d.cend(),
                               []( const std::pair< std::string, double>& p ) { return p.second == 1.0; });
    if (probe == act.activity_labels.d.cend())
    {
      LOG_ERROR( m_parent->logger(), "KPF activities file '" << m_kpf_activities_path << "' "
                 << " ID2 of " << act.activity_id.t.d << " cset does not have confidence 1.0?" );
      continue;
    }
    std::string activity_name = probe->first;

    for (const auto& actor: act.actors)
    {
      // hmm, assumes actor ID is always domain 1, will need updating when world IDs come along
      unsigned long long id1 = actor.actor_id.t.d;
      auto probe = std::find_if( actor.actor_timespan.cbegin(),
                                 actor.actor_timespan.cend(),
                                 [] ( const KPFC::scoped< KPFC::timestamp_range_t >& p ) { return p.domain == KPFC::timestamp_t::FRAME_NUMBER; });
      if (probe == actor.actor_timespan.cend())
      {
        LOG_ERROR( m_parent->logger(), "KPF activities file '" << m_kpf_activities_path << "' "
                   << "activity " << act.activity_id.t.d << " actor " << id1 << " timespan has no frame numbers?" );
      }
      else
      {
        unsigned start_frame = probe->t.start;
        unsigned stop_frame = probe->t.stop;

        track_activity_key_t key( id1, start_frame, stop_frame );
        m_track_to_activity[ key ] = activity_name;
      }
    } // ... for each actor

    reader.flush();
  } // ...for each activity

  LOG_INFO( m_parent->logger(), "Loaded " << m_track_to_activity.size() << " actor activity names" );

}


// ==================================================================
void
detected_object_set_input_kpf::priv::
read_all()
{
  m_detected_sets.clear();



  KPF::kpf_yaml_parser_t parser(m_parent->stream());
  KPF::kpf_reader_t reader(parser);

  size_t      detection_id(0);
  double      frame_number;
  vital_box_adapter_t box_adapter;
  kwiver::vital::detected_object_type_sptr types(new kwiver::vital::detected_object_type());
  kwiver::vital::detected_object_set_sptr frame_detections;

  // This will only work for files for which each non-Meta record contains at least
  // these elements (the minimum necessary to build a detection).  Should heterogenous
  // KPF files become common in the wild, this would have to be revisited.
  while ( reader >> KPF::reader< KPFC::id_t >(detection_id, KPFC::id_t::DETECTION_ID)
                >> KPF::reader< KPFC::timestamp_t>(frame_number, KPFC::timestamp_t::FRAME_NUMBER)
                >> KPF::reader< KPFC::bbox_t >(box_adapter, KPFC::bbox_t::IMAGE_COORDS) )
  {
    std::string detector_name = "kpf_reader";
    double      confidence = 1.0;
    uint64_t    index = 0;

    // We've gotten a record that has the least possible info for a detections.  What
    // else can we find that might be useful?  In particular pick up the elements
    // our sister writer writes
    auto det_name_packet = reader.transfer_kv_packet_from_buffer("detector_name");
    if ( det_name_packet.first )
    {
      detector_name = det_name_packet.second.kv.val;
    }

    auto confidence_packet = reader.transfer_packet_from_buffer( KPF::packet_header_t( KPF::packet_style::CONF, DETECTOR_DOMAIN ) );
    if ( confidence_packet.first )
    {
      confidence = confidence_packet.second.conf.d;
    }

    auto index_packet = reader.transfer_packet_from_buffer( KPF::packet_header_t( KPF::packet_style::ID, KPFC::id_t::TRACK_ID ) );
    if ( index_packet.first )
    {
      index = index_packet.second.conf.d;
    }

    kwiver::vital::bounding_box_d bbox(0, 0, 0, 0);
    box_adapter.get(bbox);
    kwiver::vital::detected_object_sptr det(new kwiver::vital::detected_object(bbox, confidence, types));
    det->set_detector_name(detector_name);
    det->set_index(index);

    frame_detections = m_detected_sets[frame_number];
    if (frame_detections.get() == nullptr)
    {
      // create a new detection set entry
      frame_detections = std::make_shared<kwiver::vital::detected_object_set>();
      m_detected_sets[frame_number] = frame_detections;
    }
    frame_detections->add(det);

    // did we receive any metadata?
    for (auto m : reader.get_meta_packets())
    {
      std::cout << "Metadata: '" << m << "'\n";
    }
    LOG_TRACE( m_parent->logger(), "FLUSHING" );
    reader.flush();
  }
  LOG_TRACE( m_parent->logger(), "DONE" );

} // read_all

} } } // end namespace
