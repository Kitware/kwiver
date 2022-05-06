// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of KLV muxer.

#include "klv_0104.h"
#include "klv_0601.h"
#include "klv_0806.h"
#include "klv_0903.h"
#include "klv_1002.h"
#include "klv_1108.h"
#include "klv_1108_metric_set.h"
#include "klv_1204.h"
#include "klv_muxer.h"

#include <vital/range/iterator_range.h>

#include <tuple>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
// Values are valid for 30 seconds if not explicitly overridden.
constexpr uint64_t klv_packet_default_duration = 30000000;
constexpr uint64_t klv_0104_default_duration = klv_packet_default_duration;
constexpr uint64_t klv_0601_default_duration = klv_packet_default_duration;

// ----------------------------------------------------------------------------
// Custom compare function which determines if two ST1108 local sets which
// occupy the same time frame are mergeable
bool
klv_1108_cmp( klv_local_set const& lhs, klv_local_set const& rhs )
{
  return
    std::make_tuple( lhs.at( KLV_1108_ASSESSMENT_POINT ),
                     lhs.count( KLV_1108_WINDOW_CORNERS_PACK )
                     ? lhs.at( KLV_1108_WINDOW_CORNERS_PACK )
                     : klv_value{},
                     lhs.at( KLV_1108_COMPRESSION_TYPE ),
                     lhs.at( KLV_1108_COMPRESSION_PROFILE ),
                     lhs.at( KLV_1108_COMPRESSION_LEVEL ),
                     lhs.at( KLV_1108_COMPRESSION_RATIO ),
                     lhs.at( KLV_1108_STREAM_BITRATE ),
                     lhs.at( KLV_1108_DOCUMENT_VERSION ) ) <
    std::make_tuple( rhs.at( KLV_1108_ASSESSMENT_POINT ),
                     rhs.count( KLV_1108_WINDOW_CORNERS_PACK )
                     ? rhs.at( KLV_1108_WINDOW_CORNERS_PACK )
                     : klv_value{},
                     rhs.at( KLV_1108_COMPRESSION_TYPE ),
                     rhs.at( KLV_1108_COMPRESSION_PROFILE ),
                     rhs.at( KLV_1108_COMPRESSION_LEVEL ),
                     rhs.at( KLV_1108_COMPRESSION_RATIO ),
                     rhs.at( KLV_1108_STREAM_BITRATE ),
                     rhs.at( KLV_1108_DOCUMENT_VERSION ) );
}

// ----------------------------------------------------------------------------
// Similar to klv_1108_cmp, except checks time ranges as well
bool
klv_1108_timed_cmp( klv_local_set const& lhs, klv_local_set const& rhs )
{
  if( klv_1108_cmp( lhs, rhs ) )
  {
    return true;
  }
  if( klv_1108_cmp( rhs, lhs ) )
  {
    return false;
  }
  return lhs.at( KLV_1108_METRIC_PERIOD_PACK ) < rhs.at(
           KLV_1108_METRIC_PERIOD_PACK );
}

// ----------------------------------------------------------------------------
bool
klv_1108_timed_eq( klv_local_set const& lhs, klv_local_set const& rhs )
{
  return !klv_1108_timed_cmp( lhs, rhs ) && !klv_1108_timed_cmp( rhs, lhs );
}

} // namespace

// ----------------------------------------------------------------------------
klv_muxer
::klv_muxer( klv_timeline const& timeline )
  : m_timeline( timeline ),
    m_packets{},
    m_frames{},
    m_prev_frame{ 0 },
    m_cached_1108{ klv_1108_cmp } {}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame( uint64_t timestamp )
{
  m_frames.emplace_back( timestamp );
  if( !check_timestamp( timestamp ) )
  {
    return;
  }

  send_frame_unknown( timestamp );
  send_frame_local_set( KLV_PACKET_MISB_0102_LOCAL_SET, timestamp );
  send_frame_universal_set( KLV_PACKET_MISB_0104_UNIVERSAL_SET, timestamp,
                            KLV_0104_USER_DEFINED_TIMESTAMP );
  send_frame_0601( timestamp );
  send_frame_local_set( KLV_PACKET_MISB_0806_LOCAL_SET, timestamp,
                        KLV_0806_TIMESTAMP );
  send_frame_local_set( KLV_PACKET_MISB_0903_LOCAL_SET, timestamp,
                        KLV_0903_PRECISION_TIMESTAMP );
  send_frame_local_set( KLV_PACKET_MISB_1002_LOCAL_SET, timestamp,
                        KLV_1002_PRECISION_TIMESTAMP );
  send_frame_1108( timestamp );
  send_frame_local_set( KLV_PACKET_MISB_1202_LOCAL_SET, timestamp );
  send_frame_1204( timestamp );
  send_frame_local_set( KLV_PACKET_MISB_1206_LOCAL_SET, timestamp );
  send_frame_local_set( KLV_PACKET_MISB_1601_LOCAL_SET, timestamp );

  m_prev_frame = timestamp;
}

// ----------------------------------------------------------------------------
uint64_t
klv_muxer
::next_frame_time() const
{
  return m_frames.empty() ? UINT64_MAX : m_frames.front();
}

// ----------------------------------------------------------------------------
std::vector< klv_packet >
klv_muxer
::receive_frame()
{
  flush_frame();

  if( m_frames.empty() )
  {
    throw std::logic_error( "more frames requested than sent" );
  }

  auto const frame = m_frames.front();
  m_frames.pop_front();

  using iterator_t = typename decltype( m_packets )::iterator;
  using iterator_range_t = kvr::iterator_range< iterator_t >;

  iterator_range_t const range =
  { m_packets.begin(), m_packets.upper_bound( frame ) };
  std::vector< klv_packet > result;
  for( auto const& entry : range )
  {
    result.emplace_back( entry.second );
  }

  m_packets.erase( range.begin(), range.end() );

  return result;
}

// ----------------------------------------------------------------------------
klv_timeline const&
klv_muxer
::timeline() const
{
  return m_timeline;
}

// ----------------------------------------------------------------------------
void
klv_muxer
::reset()
{
  m_packets.clear();
  m_frames.clear();
  m_prev_frame = 0;
  m_cached_1108.clear();
}

// ----------------------------------------------------------------------------
void
klv_muxer
::flush_frame()
{
  flush_frame_1108();
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_unknown( uint64_t timestamp )
{
  constexpr auto standard = KLV_PACKET_UNKNOWN;
  for( auto const& entry : m_timeline.find_all( standard ) )
  {
    for( auto const& inner_entry :
         entry.second.find( { m_prev_frame, timestamp } ) )
    {
      for( auto const& packet :
           inner_entry.value.get< std::set< klv_packet > >() )
      {
        m_packets.emplace( timestamp, packet );
      }
    }
  }
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_0104( uint64_t timestamp )
{
  constexpr auto standard = KLV_PACKET_MISB_0104_UNIVERSAL_SET;
  auto const& lookup = klv_0104_traits_lookup();

  // Create a set of all tags present at timestamp
  klv_universal_set set;
  for( auto const& entry : m_timeline.find_all( standard ) )
  {
    auto const it = entry.second.find( timestamp );
    if( it != entry.second.end() )
    {
      set.add( lookup.by_tag( entry.first.tag ).uds_key(), it->value );
    }
  }

  // If any tags were present, put the set into a packet and ship it
  if( !set.empty() )
  {
    set.add(
        lookup.by_tag( KLV_0104_USER_DEFINED_TIMESTAMP ).uds_key(),
        timestamp );
    m_packets.emplace(
        timestamp, klv_packet{ klv_0104_key(), std::move( set ) } );
  }
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_0601( uint64_t timestamp )
{
  constexpr auto standard = KLV_PACKET_MISB_0601_LOCAL_SET;
  auto const& lookup = klv_0601_traits_lookup();

  // We may in the future want to do more fancy Report-On-Change things here to
  // save on bandwidth. For right now we just dump all data at each frame
  klv_local_set set;
  std::vector< klv_0601_wavelength_record > wavelength_list;
  std::vector< klv_0601_payload_record > payload_list;
  std::vector< klv_0601_waypoint_record > waypoint_list;
  std::vector< uint64_t > control_command_verify_list;
  for( auto const& entry : m_timeline.find_all( standard ) )
  {
    auto const tag = entry.first.tag;

    // Tags which only make sense as point occurences
    if( tag == KLV_0601_WEAPON_FIRED ||
        tag == KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST ||
        tag == KLV_0601_SEGMENT_LOCAL_SET ||
        tag == KLV_0601_AMEND_LOCAL_SET )
    {
      for( auto const& subentry :
           entry.second.find( { m_prev_frame, timestamp } ) )
      {
        if( tag == KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST )
        {
          auto const& typed_value =
            subentry.value.get< std::vector< uint64_t > >();
          control_command_verify_list.insert(
            control_command_verify_list.end(),
            typed_value.cbegin(), typed_value.cend() );
        }
        else
        {
          if( tag == KLV_0601_WEAPON_FIRED )
          {
            // Multiples not allowed, so just use the most recent one
            set.erase( tag );
          }
          set.add( tag, subentry.value );
        }
      }
      continue;
    }

    // Tags which hold a value over time
    auto const it = entry.second.find( timestamp );
    if( it != entry.second.end() )
    {
      switch( tag )
      {
        // List tags
        case KLV_0601_WAVELENGTHS_LIST:
          wavelength_list.emplace_back(
            it->value.get< klv_0601_wavelength_record >() );
          break;
        case KLV_0601_PAYLOAD_LIST:
          payload_list.emplace_back(
            it->value.get< klv_0601_payload_record >() );
          break;
        case KLV_0601_WAYPOINT_LIST:
          waypoint_list.emplace_back(
            it->value.get< klv_0601_waypoint_record >() );
          break;

        // Non-list tags
        case KLV_0601_SDCC_FLP:
        case KLV_0601_CONTROL_COMMAND:
        default:
          set.add( tag, it->value );
          break;
      }
    }
    else if( lookup.by_tag( entry.first.tag ).tag_count_range().upper() == 1 )
    {
      // Check if we need to explicitly cancel the data
      // Only possible if the tag does not allow multiples
      auto const jt = entry.second.find( m_prev_frame );
      if( jt != entry.second.end() &&
          jt->key_interval.upper() - jt->key_interval.lower() <
          klv_0601_default_duration )
      {
        set.add( entry.first.tag, {} );
      }
    }
  }

  // Put any assembled lists into the packet
  if( !wavelength_list.empty() )
  {
    set.add( KLV_0601_WAVELENGTHS_LIST, wavelength_list );
  }
  if( !payload_list.empty() )
  {
    set.add( KLV_0601_PAYLOAD_LIST, payload_list );
  }
  if( !waypoint_list.empty() )
  {
    set.add( KLV_0601_WAYPOINT_LIST, waypoint_list );
  }
  if( !control_command_verify_list.empty() )
  {
    set.add( KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST,
             control_command_verify_list );
  }

  // If any tags were present, put the set into a packet and ship it
  if( !set.empty() )
  {
    set.add( KLV_0601_PRECISION_TIMESTAMP, timestamp );
    m_packets.emplace( timestamp,
                       klv_packet{ klv_0601_key(), std::move( set ) } );
  }
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_1108( uint64_t timestamp )
{
  constexpr auto standard = KLV_PACKET_MISB_1108_LOCAL_SET;

  // Find each metric
  std::vector< klv_local_set > sets;
  for( auto const& entry :
       m_timeline.find_all( standard, KLV_1108_METRIC_LOCAL_SET ) )
  {
    auto const index = entry.first.index;

    // Find each change to that metric within the span of this frame
    for( auto start_timestamp = m_prev_frame;
         start_timestamp < timestamp;)
    {
      auto next_timestamp = timestamp;

      // Assemble a local set, if all mandatory tags are present
      klv_local_set set;
      auto is_valid_set = true;
      klv_timeline::interval_t time_interval = { start_timestamp, timestamp };
      for( auto const tag : { KLV_1108_ASSESSMENT_POINT,
                              KLV_1108_WINDOW_CORNERS_PACK,
                              KLV_1108_COMPRESSION_TYPE,
                              KLV_1108_COMPRESSION_PROFILE,
                              KLV_1108_COMPRESSION_LEVEL,
                              KLV_1108_COMPRESSION_RATIO,
                              KLV_1108_STREAM_BITRATE,
                              KLV_1108_DOCUMENT_VERSION } )
      {
        auto const is_mandatory =
          klv_1108_traits_lookup().by_tag( tag ).tag_count_range().lower() > 0;
        auto const it = m_timeline.find( standard, tag, index );
        if( it == m_timeline.end() )
        {
          // This tag is never present
          if( is_mandatory )
          {
            is_valid_set = false;
            break;
          }
        }
        else
        {
          auto const range = it->second.find( { start_timestamp, timestamp } );
          if( range.empty() )
          {
            // This tag is not present during this frame
            if( is_mandatory )
            {
              is_valid_set = false;
              break;
            }
          }
          else if( !range.begin()->key_interval.contains( start_timestamp ) )
          {
            // This tag is present later in the frame, but not now
            next_timestamp =
              std::min( next_timestamp, range.begin()->key_interval.lower() );
            if( is_mandatory )
            {
              is_valid_set = false;
              break;
            }
          }
          else
          {
            // This tag is present currently in the frame
            set.add( tag, range.begin()->value );
            next_timestamp =
              std::min( next_timestamp, range.begin()->key_interval.upper() );
            time_interval.truncate_upper(
                range.begin()->key_interval.upper() );
          }
        }
      }

      // If a set could be assembled, keep it
      if( is_valid_set )
      {
        set.add( KLV_1108_METRIC_PERIOD_PACK,
                 klv_1108_metric_period_pack{
                time_interval.lower(),
                static_cast< uint32_t >(
                  time_interval.upper() - time_interval.lower() ) } );
        set.add( KLV_1108_METRIC_LOCAL_SET,
                 *entry.second.at( start_timestamp ) );
        sets.emplace_back( std::move( set ) );
      }

      // Go to the next change in the metric this frame
      start_timestamp = next_timestamp;
    }
  }

  // Sort sets, guaranteeing compatible ones end up next to each other
  std::sort( sets.begin(), sets.end(), klv_1108_timed_cmp );
  for( auto it = sets.begin(); it != sets.end();)
  {
    // Copy set
    auto set = *it;

    // Find all concurrent sets compatible with this one and merge them into
    // the copy
    auto jt = std::next( it );
    for(; jt != sets.end() && klv_1108_timed_eq( *it, *jt ); ++jt )
    {
      set.add( KLV_1108_METRIC_LOCAL_SET,
               jt->at( KLV_1108_METRIC_LOCAL_SET ) );
    }
    it = jt;

    // Get ready to possibly merge this set with a past one
    auto was_merged = false;
    auto const period =
      set.at( KLV_1108_METRIC_PERIOD_PACK )
      .get< klv_1108_metric_period_pack >();
    set.erase( KLV_1108_METRIC_PERIOD_PACK );

    auto const range = m_cached_1108.equal_range( set );
    for( auto kt = range.first; kt != range.second;)
    {
      auto const next_kt = std::next( kt );

      // Copy past set
      auto cached_set = *kt;

      // If compatible, merge the current set into the past set
      auto cached_period =
        kt->at( KLV_1108_METRIC_PERIOD_PACK )
        .get< klv_1108_metric_period_pack >();
      cached_set.erase( KLV_1108_METRIC_PERIOD_PACK );
      if( period.timestamp <= cached_period.timestamp + cached_period.offset &&
          set == cached_set )
      {
        cached_period.offset =
          period.timestamp + period.offset - cached_period.timestamp;
        cached_set.add( KLV_1108_METRIC_PERIOD_PACK, cached_period );
        m_cached_1108.erase( kt );
        m_cached_1108.emplace( std::move( cached_set ) );
        was_merged = true;
      }

      kt = next_kt;
    }
    if( !was_merged )
    {
      // No compatible past sets found, so we have to add the current set as a
      // new member
      set.add( KLV_1108_METRIC_PERIOD_PACK, period );
      m_cached_1108.emplace( std::move( set ) );
    }
  }
}

// ----------------------------------------------------------------------------
void
klv_muxer
::flush_frame_1108()
{
  for( auto& set : m_cached_1108 )
  {
    m_packets.emplace(
        set.at( KLV_1108_METRIC_PERIOD_PACK )
          .get< klv_1108_metric_period_pack >().timestamp,
        klv_packet{ klv_1108_key(), set } );
  }
  m_cached_1108.clear();
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_1204( uint64_t timestamp )
{
  constexpr auto standard = KLV_PACKET_MISB_1204_MIIS_ID;

  for( auto const& entry : m_timeline.find_all( standard ) )
  {
    auto const it = entry.second.find( timestamp );
    if( it != entry.second.end() )
    {
      m_packets.emplace( timestamp, klv_packet{ klv_1204_key(), it->value } );
    }
  }
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_local_set( klv_top_level_tag standard, uint64_t timestamp,
                        klv_lds_key timestamp_tag )
{
  auto const key =
    klv_lookup_packet_traits().by_tag( standard ).uds_key();
  auto const lookup =
    klv_lookup_packet_traits().by_tag( standard ).subtag_lookup();
  if( !lookup )
  {
    throw std::logic_error(
      "klv_muxer: given local set without any tag trait information" );
  }

  // Create a set of all tags present at timestamp
  klv_local_set set;
  for( auto const& entry : m_timeline.find_all( standard ) )
  {
    auto const it = entry.second.find( timestamp );
    if( it != entry.second.end() )
    {
      set.add( entry.first.tag, it->value );
    }
  }

  // If any tags were present, put the set into a packet and ship it
  if( !set.empty() )
  {
    if( timestamp_tag )
    {
      set.add( timestamp_tag, timestamp );
    }
    m_packets.emplace( timestamp, klv_packet{ key, std::move( set ) } );
  }
}

// ----------------------------------------------------------------------------
void
klv_muxer
::send_frame_universal_set( klv_top_level_tag standard, uint64_t timestamp,
                            klv_lds_key timestamp_tag )
{
  auto const key = klv_lookup_packet_traits().by_tag( standard ).uds_key();
  auto const lookup =
    klv_lookup_packet_traits().by_tag( standard ).subtag_lookup();
  if( !lookup )
  {
    throw std::logic_error(
      "klv_muxer: given universal set without any tag trait information" );
  }

  // Create a set of all tags present at timestamp
  klv_universal_set set;
  for( auto const& entry : m_timeline.find_all( standard ) )
  {
    auto const it = entry.second.find( timestamp );
    if( it != entry.second.end() )
    {
      set.add( lookup->by_tag( entry.first.tag ).uds_key(), it->value );
    }
  }

  // If any tags were present, put the set into a packet and ship it
  if( !set.empty() )
  {
    if( timestamp_tag )
    {
      set.add( lookup->by_tag( timestamp_tag ).uds_key(), timestamp );
    }
    m_packets.emplace( timestamp, klv_packet{ key, std::move( set ) } );
  }
}

// ----------------------------------------------------------------------------
bool
klv_muxer
::check_timestamp( uint64_t timestamp ) const
{
  // Demuxer can only output packets in chronological order
  auto const result = timestamp >= m_prev_frame;
  if( !result )
  {
    LOG_WARN( kv::get_logger( "klv" ),
              "muxer: refusing to emit packets out-of-order "
               << "( " << timestamp << " less than "
               << m_prev_frame << " )" );
  }
  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
