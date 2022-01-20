// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of KLV demuxer.

#include "klv_0104.h"
#include "klv_0601.h"
#include "klv_1108.h"
#include "klv_1108_metric_set.h"
#include "klv_demuxer.h"

#include <vital/logger/logger.h>
#include <vital/range/iota.h>

namespace kv = kwiver::vital;

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
uint64_t
klv_find_or_insert_1108( klv_timeline& timeline,
                         klv_local_set const& parent_set,
                         klv_value const& metric_set_value )
{
  constexpr auto standard = KLV_PACKET_MISB_1108_LOCAL_SET;

  // Invalid metric sets are all treated as unique and therefore an equivalent
  // sub-timeline cannot be found
  if( metric_set_value.valid() )
  {
    auto const& metric_set = metric_set_value.get< klv_local_set >();

    // The METRIC_LOCAL_SET tag is used to get all the unique metrics being
    // tracked.
    auto const candidates =
      timeline.find_all( standard, KLV_1108_METRIC_LOCAL_SET );
    for( auto candidate : candidates )
    {
      // We are trying to find out if this candidate set of sub-timelines is
      // tracking the same metric as the new one we want to insert
      auto is_match = true;
      auto const index = candidate.first.index;

      // Either of these tags in the parent set being different would mean the
      // new metric should be tracked separately
      for( auto const tag : { KLV_1108_ASSESSMENT_POINT,
                              KLV_1108_WINDOW_CORNERS_PACK } )
      {
        auto const it = timeline.find( standard, tag, index );

        // All this to check whether the values are different, the complexity
        // coming from the fact that we don't assume either value exists in the
        // first place, since WINDOWS_CORNER_PACK is optional
        if( it != timeline.end() && !it->second.empty() &&
            ( it->second.begin()->value.empty() != !parent_set.has( tag ) ||
              ( parent_set.has( tag ) &&
                it->second.begin()->value != parent_set.at( tag ) ) ) )
        {
          is_match = false;
          break;
        }
      }

      // Early exit
      if( !is_match )
      {
        continue;
      }

      // Find the embedded metric set for this candidate, ensuring it exists
      auto const metric_set_it =
        timeline.find( standard, KLV_1108_METRIC_LOCAL_SET, index );
      if( metric_set_it == timeline.end() || metric_set_it->second.empty() )
      {
        continue;
      }

      auto const& candidate_metric_set =
        metric_set_it->second.begin()->value.get< klv_local_set >();

      // Any of these tags being different would mean the new metric should be
      // tracked separately
      for( auto const tag : { KLV_1108_METRIC_SET_NAME,
                              KLV_1108_METRIC_SET_VERSION,
                              KLV_1108_METRIC_SET_IMPLEMENTER,
                              KLV_1108_METRIC_SET_PARAMETERS } )
      {
        // These are all mandatory tags, so we assume they exist
        if( candidate_metric_set.at( tag ) != metric_set.at( tag ) )
        {
          is_match = false;
          break;
        }
      }

      // Return the index found if successful
      if( is_match )
      {
        return index;
      }
    }
  }

  // Finding an existing sub-timeline to use failed; insert new one
  auto const index =
    timeline.insert( standard, KLV_1108_ASSESSMENT_POINT )->first.index;
  timeline.erase(
    timeline.find( standard, KLV_1108_ASSESSMENT_POINT, index ) );

  return index;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
klv_demuxer
::klv_demuxer( klv_timeline& timeline )
  : m_last_timestamp{ 0 }, m_unknown_key_indices{}, m_timeline( timeline ) {}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_packet( klv_packet const& packet )
{
  auto const& trait = klv_lookup_packet_traits().by_uds_key( packet.key );

  // Invalid or unrecognized packets are still saved in raw byte form
  if( !packet.value.valid() )
  {
    demux_unknown( packet );
    return;
  }

  // Demux based on type of packet
  switch( trait.tag() )
  {
    case KLV_PACKET_MISB_0601_LOCAL_SET:
      demux_0601( packet.value.get< klv_local_set >() );
      break;
    case KLV_PACKET_MISB_0104_UNIVERSAL_SET:
      demux_0104( packet.value.get< klv_universal_set >() );
      break;
    case KLV_PACKET_MISB_1108_LOCAL_SET:
      demux_1108( packet.value.get< klv_local_set >() );
      break;
    case KLV_PACKET_UNKNOWN:
    default:
      throw std::logic_error( "klv packet with unknown key but valid value" );
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::seek( uint64_t timestamp )
{
  m_last_timestamp = timestamp;
}

// ----------------------------------------------------------------------------
klv_timeline&
klv_demuxer
::timeline() const
{
  return m_timeline;
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::reset()
{
  m_last_timestamp = 0;
  m_unknown_key_indices.clear();
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_unknown( klv_packet const& packet )
{
  // Keep track of which unknown keys map to which timelines
  auto index_it = m_unknown_key_indices.find( packet.key );

  // Create new timeline for this key if it's new
  if( index_it == m_unknown_key_indices.end() )
  {
    auto const index = m_timeline.insert( KLV_PACKET_UNKNOWN, 0 )->first.index;
    index_it = m_unknown_key_indices.emplace( packet.key, index ).first;
  }

  auto& unknown_timeline =
    m_timeline.find( KLV_PACKET_UNKNOWN, 0, index_it->second )->second;

  // Add this packet to a list (created here if necessary) of unknown packets
  // at this timestamp.
  // m_last_timestamp used here because we can't extract a timestamp from a
  // packet of unknown format
  auto const unknown_it = unknown_timeline.find( m_last_timestamp );
  if( unknown_it == unknown_timeline.end() )
  {
    unknown_timeline.set( { m_last_timestamp, m_last_timestamp + 1 },
                          std::vector< klv_packet >{ packet } );
  }
  else
  {
    unknown_it->value.get< std::vector< klv_packet > >()
      .emplace_back( packet );
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_0104( klv_universal_set const& value )
{
  constexpr auto standard = KLV_PACKET_MISB_0104_UNIVERSAL_SET;

  // Extract timestamp
  auto const& lookup = klv_0104_traits_lookup();
  auto const timestamp_key =
    lookup.by_tag( KLV_0104_USER_DEFINED_TIMESTAMP ).uds_key();
  auto const timestamp = value.at( timestamp_key ).get< uint64_t >();
  if( !check_timestamp( timestamp ) )
  {
    return;
  }

  // By default, valid for 30 seconds
  auto const time_interval =
    interval_t{ timestamp, timestamp + klv_0104_default_duration };

  for( auto const& entry : value )
  {
    // Timestamp already implicitly encoded
    if( entry.first == timestamp_key )
    {
      continue;
    }

    // No duplicate entries allowed, so this is relatively straightforward
    auto const& trait = lookup.by_uds_key( entry.first );
    demux_single_entry( standard, trait.tag(), 0, time_interval,
                        entry.second );
  }

  // Update timestamp
  m_last_timestamp = timestamp;
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_0601( klv_local_set const& value )
{
  auto const standard = KLV_PACKET_MISB_0601_LOCAL_SET;

  // Extract timestamp
  auto const timestamp =
    value.at( KLV_0601_PRECISION_TIMESTAMP ).get< uint64_t >();
  if( !check_timestamp( timestamp ) )
  {
    return;
  }

  // By default, valid for 30 seconds
  auto const time_interval =
    interval_t{ timestamp, timestamp + klv_0601_default_duration };

  for( auto const& entry : value )
  {
    auto const tag = entry.first;

    // Timestamp already implicitly encoded
    if( tag == KLV_0601_PRECISION_TIMESTAMP )
    {
      continue;
    }

    // TODO: Add special list accumulation logic for tags:
    // KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST
    // KLV_0601_ACTIVE_WAVELENGTH_LIST
    // KLV_0601_WAVELENGTHS_LIST
    // KLV_0601_PAYLOAD_LIST
    // KLV_0601_WAYPOINT_LIST

    // TODO: Deal with tags which can have multiple entries:
    // KLV_0601_SEGMENT_LOCAL_SET
    // KLV_0601_AMEND_LOCAL_SET
    // KLV_0601_SDCC_FLP

    // CONTROL_COMMAND is a special case. It supports multiple entries, but is
    // nonetheless easy to implement, since it gives us a unique id that we can
    // use to track each entry over time, allowing it to be treated similarly
    // to a single-entry tag
    auto index = 0;
    if( entry.first == KLV_0601_CONTROL_COMMAND )
    {
      index = entry.second.get< klv_0601_control_command >().id;
    }

    // Single-entry tags passed off here
    demux_single_entry( standard, entry.first, index, time_interval,
                        entry.second );
  }

  // Update timestamp
  m_last_timestamp = timestamp;
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_1108( klv_local_set const& value )
{
  constexpr auto standard = KLV_PACKET_MISB_1108_LOCAL_SET;

  // Extract timestamp
  auto const metric_period = value.at( KLV_1108_METRIC_PERIOD_PACK )
    .get< klv_1108_metric_period_pack >();
  if( !check_timestamp( metric_period.timestamp ) )
  {
    return;
  }

  // Valid for the period of time specified in METRIC_PERIOD_PACK field
  auto const time_interval =
    interval_t{ metric_period.timestamp,
                metric_period.timestamp + metric_period.offset };

  // Each 1108 local set can have multiple metrics, each contained in its own
  // metric local set. Items in the parent set are shared among the metric
  // sets. We want to create one index for each *metric set*, copying the
  // parent set's common data to each.
  for( auto const& metric_set_entry :
       value.all_at( KLV_1108_METRIC_LOCAL_SET ) )
  {
    // Create the and fill the index for this metric set
    auto const index =
      klv_find_or_insert_1108( m_timeline, value, metric_set_entry.second );
    demux_single_entry( standard, KLV_1108_METRIC_LOCAL_SET, index,
                        time_interval, metric_set_entry.second );

    // Copy the parent's data to this metric set's index
    for( auto const& entry : value )
    {
      auto const tag = entry.first;

      // We've already encoded these
      if( tag == KLV_1108_METRIC_LOCAL_SET ||
          tag == KLV_1108_METRIC_PERIOD_PACK )
      {
        continue;
      }

      demux_single_entry( standard, tag, index, time_interval, entry.second );
    }
  }

  // Update timestamp
  m_last_timestamp = metric_period.timestamp;
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_single_entry( klv_top_level_tag standard, klv_lds_key tag,
                      uint64_t index, interval_t const& time_interval,
                      klv_value const& value )
{
  if( value.empty() )
  {
    // Null value: erase timespan instead of adding a null entry
    auto const it = m_timeline.find( standard, tag );
    if( it == m_timeline.end() )
    {
      return;
    }

    auto const inner_it = it->second.find( time_interval.lower() );
    if( inner_it == it->second.end() )
    {
      return;
    }
    it->second.erase( { time_interval.lower(),
                        inner_it->key_interval.upper() } );
  }
  else
  {
    // Non-null value: add new entry
    auto const it = m_timeline.insert_or_find( standard, tag, index );
    it->second.set( time_interval, value );
  }
}

// ----------------------------------------------------------------------------
bool
klv_demuxer
::check_timestamp( uint64_t timestamp ) const
{
  // Packets *must* be fed to demuxer in chronological order to prevent older
  // packets from incorrectly overriding newer packets.
  auto const result = timestamp >= m_last_timestamp;
  if( !result )
  {
    LOG_ERROR( kv::get_logger( "klv" ),
               "demuxer: dropping out-of-order packet ( " << timestamp << " less than " << m_last_timestamp << " )" );
  }
  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
