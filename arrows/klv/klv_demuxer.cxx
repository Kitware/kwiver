// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of KLV demuxer.

#include "klv_demuxer.h"

#include <arrows/klv/klv_all.h>

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
klv_local_set
indexify_1108( klv_local_set const& parent_set,
               klv_value const& metric_set_value )
{
  klv_local_set result;
  for( auto const tag : { KLV_1108_ASSESSMENT_POINT,
                          KLV_1108_WINDOW_CORNERS_PACK } )
  {
    auto const it = parent_set.find( tag );
    if( it != parent_set.cend() )
    {
      result.add( tag, it->second );
    }
  }

  if( metric_set_value.valid() )
  {
    auto const& metric_set = metric_set_value.get< klv_local_set >();
    klv_local_set result_metric_set;
    for( auto const tag : { KLV_1108_METRIC_SET_NAME,
                            KLV_1108_METRIC_SET_VERSION,
                            KLV_1108_METRIC_SET_IMPLEMENTER,
                            KLV_1108_METRIC_SET_PARAMETERS } )
    {
      auto const it = metric_set.find( tag );
      if( it != metric_set.cend() )
      {
        result_metric_set.add( tag, it->second );
      }
    }
    result.add( KLV_1108_METRIC_LOCAL_SET, std::move( result_metric_set ) );
  }

  return result;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
klv_demuxer
::klv_demuxer( klv_timeline& timeline )
  : m_frame_timestamp{ 0 }, m_prev_frame_timestamp{ 0 }, m_timeline( timeline ) {}

// ----------------------------------------------------------------------------
void
klv_demuxer
::send_frame( std::vector< klv_packet > const& packets,
              kv::optional< uint64_t > backup_timestamp )
{
  m_prev_frame_timestamp = m_frame_timestamp;

  auto found_timestamp = false;
  for( auto const& packet : packets )
  {
    auto const derived_timestamp = klv_packet_timestamp( packet );
    if( derived_timestamp )
    {
      if( found_timestamp )
      {
        m_frame_timestamp = std::min( m_frame_timestamp, *derived_timestamp );
      }
      else
      {
        m_frame_timestamp = *derived_timestamp;
        found_timestamp = true;
      }
    }
  }

  if( !found_timestamp )
  {
    if( backup_timestamp )
    {
      LOG_DEBUG( kv::get_logger( "klv" ),
                 "demuxer: using backup timestamping method" );
      m_frame_timestamp = *backup_timestamp;
    }
    else
    {
      LOG_WARN( kv::get_logger( "klv" ),
                "demuxer: unable to update timestamp for new frame" );
    }
  }

  for( auto const& packet : packets )
  {
    demux_packet( packet );
  }
}

// ----------------------------------------------------------------------------
uint64_t
klv_demuxer
::frame_time() const
{
  return m_frame_timestamp;
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_packet( klv_packet const& packet )
{
  auto const& trait = klv_lookup_packet_traits().by_uds_key( packet.key );

  auto const derived_timestamp = klv_packet_timestamp( packet );
  auto const timestamp = derived_timestamp ? *derived_timestamp : m_frame_timestamp;
  auto const time_interval = interval_t{ timestamp, timestamp + klv_packet_default_duration };

  // Invalid or unrecognized packets are still saved in raw byte form
  if( !packet.value.valid() )
  {
    demux_unknown( packet, timestamp );
    return;
  }

  // For brevity
  auto const tag = static_cast< klv_top_level_tag >( trait.tag() );
  auto const& value = packet.value;

  // Determine which tag holds the timestamp, if any
  klv_lds_key timestamp_tag = 0;
  switch( tag )
  {
    case KLV_PACKET_MISB_0104_UNIVERSAL_SET:
      timestamp_tag = KLV_0104_USER_DEFINED_TIMESTAMP;
      break;
    case KLV_PACKET_MISB_0806_LOCAL_SET:
      timestamp_tag = KLV_0806_TIMESTAMP;
      break;
    case KLV_PACKET_MISB_0903_LOCAL_SET:
      timestamp_tag = KLV_0903_PRECISION_TIMESTAMP;
      break;
    case KLV_PACKET_MISB_1002_LOCAL_SET:
      timestamp_tag = KLV_1002_PRECISION_TIMESTAMP;
      break;
    case KLV_PACKET_MISB_1107_LOCAL_SET:
      timestamp_tag = KLV_1107_PRECISION_TIMESTAMP;
    default:
      break;
  }

  // Demux based on type of packet
  switch( tag )
  {
    case KLV_PACKET_MISB_0104_UNIVERSAL_SET:
      demux_set( tag, value.get< klv_universal_set >(), time_interval,
                 timestamp_tag );
      break;
    case KLV_PACKET_MISB_0601_LOCAL_SET:
      demux_0601( value.get< klv_local_set >(), timestamp );
      break;
    case KLV_PACKET_MISB_1108_LOCAL_SET:
      demux_1108( value.get< klv_local_set >(), timestamp );
      break;
    case KLV_PACKET_MISB_0102_LOCAL_SET:
    case KLV_PACKET_MISB_0806_LOCAL_SET:
    case KLV_PACKET_MISB_0903_LOCAL_SET:
    case KLV_PACKET_MISB_1002_LOCAL_SET:
    case KLV_PACKET_MISB_1107_LOCAL_SET:
    case KLV_PACKET_MISB_1202_LOCAL_SET:
    case KLV_PACKET_MISB_1206_LOCAL_SET:
    case KLV_PACKET_MISB_1601_LOCAL_SET:
      demux_set( tag, value.get< klv_local_set >(), time_interval,
                 timestamp_tag );
      break;
    case KLV_PACKET_MISB_1204_MIIS_ID:
      demux_single_entry( tag, 0, {}, time_interval, value );
      break;
    case KLV_PACKET_UNKNOWN:
    default:
      throw std::logic_error(
        "klv_demuxer: packet with unknown key but valid value" );
  }
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
  m_prev_frame_timestamp = 0;
  m_frame_timestamp = 0;
  m_cancel_points.clear();
  m_timeline.clear();
}

// ----------------------------------------------------------------------------
template< class T >
void
klv_demuxer
::demux_list( klv_top_level_tag standard,
              klv_lds_key tag,
              interval_t const& time_interval,
              std::vector< T > const& value )
{
  for( auto const& item : value )
  {
    demux_single_entry( standard, tag, uint64_t{ item.id },
                        time_interval, item );
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_set(
  klv_top_level_tag standard, klv_local_set const& set,
  interval_t const& time_interval, klv_lds_key timestamp_tag )
{
  for( auto const& entry : set )
  {
    auto const tag = entry.first;
    auto const& value = entry.second;

    // Timestamp already implicitly encoded
    if( timestamp_tag && tag == timestamp_tag )
    {
      continue;
    }

    demux_single_entry( standard, tag, {}, time_interval, value );
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_set(
  klv_top_level_tag standard, klv_universal_set const& set,
  interval_t const& time_interval, klv_lds_key timestamp_tag )
{
  auto const lookup =
    klv_lookup_packet_traits().by_tag( standard ).subtag_lookup();
  if( !lookup )
  {
    throw std::logic_error(
      "klv_demuxer: given universal set without any tag trait information" );
  }

  for( auto const& entry : set )
  {
    auto const key = entry.first;
    auto const& value = entry.second;

    // Timestamp already implicitly encoded
    if( timestamp_tag && key == lookup->by_tag( timestamp_tag ).uds_key() )
    {
      continue;
    }

    // No duplicate entries allowed, so this is relatively straightforward
    auto const& trait = lookup->by_uds_key( key );
    demux_single_entry( standard, trait.tag(), {}, time_interval, value );
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_unknown( klv_packet const& packet, uint64_t timestamp )
{
  auto& unknown_timeline =
    m_timeline.insert_or_find( KLV_PACKET_UNKNOWN, 0, packet.key )->second;

  // Add this packet to a list (created here if necessary) of unknown packets
  // at this timestamp.
  auto const unknown_it = unknown_timeline.find( timestamp );
  if( unknown_it == unknown_timeline.end() )
  {
    unknown_timeline.set( { timestamp, timestamp + 1 },
                          std::set< klv_packet >{ packet } );
  }
  else
  {
    unknown_it->value.get< std::set< klv_packet > >().emplace( packet );
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_0601( klv_local_set const& local_set, uint64_t timestamp )
{
  auto const standard = KLV_PACKET_MISB_0601_LOCAL_SET;

  // By default, valid for 30 seconds
  auto const time_interval =
    interval_t{ timestamp, timestamp + klv_0601_default_duration };
  auto const point_time_interval =
    interval_t{ timestamp, timestamp + 1 };

  for( auto const& entry : local_set )
  {
    auto const tag = entry.first;
    auto const& value = entry.second;
    switch( tag )
    {
      // Timestamp already implicitly encoded
      case KLV_0601_PRECISION_TIMESTAMP:
        continue;

      // List tags
      case KLV_0601_WAVELENGTHS_LIST:
        demux_list( standard, tag, time_interval,
                    value.get< std::vector< klv_0601_wavelength_record > >() );
        break;
      case KLV_0601_PAYLOAD_LIST:
        demux_list( standard, tag, time_interval,
                    value.get< std::vector< klv_0601_payload_record > >() );
        break;
      case KLV_0601_WAYPOINT_LIST:
        demux_list( standard, tag, time_interval,
                    value.get< std::vector< klv_0601_waypoint_record > >() );
        break;

      // Tags which only make sense as point occurences
      case KLV_0601_WEAPON_FIRED:
        demux_single_entry(
          standard, tag, value.get< uint64_t >(), point_time_interval, value );
        break;
      case KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST:
        demux_single_entry(
          standard, tag, {}, point_time_interval, value );
        break;
      case KLV_0601_SEGMENT_LOCAL_SET:
      case KLV_0601_AMEND_LOCAL_SET:
        demux_single_entry(
          standard, tag, value.get< klv_local_set >(),
          point_time_interval, value );
        break;

      // Tags which can have multiples
      case KLV_0601_SDCC_FLP:
        demux_single_entry(
          standard, tag, value.get< klv_1010_sdcc_flp >().members,
          time_interval, value );
        break;
      case KLV_0601_CONTROL_COMMAND:
        demux_single_entry(
          standard, tag,
          uint64_t{ value.get< klv_0601_control_command >().id },
          time_interval, value );
        break;

      // Standard single-entry tags
      default:
        demux_single_entry( standard, tag, {}, time_interval, value );
        break;
    };
  }
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_1108( klv_local_set const& value, uint64_t timestamp )
{
  constexpr auto standard = KLV_PACKET_MISB_1108_LOCAL_SET;

  // Extract timestamp
  auto const metric_period = value.at( KLV_1108_METRIC_PERIOD_PACK )
    .get< klv_1108_metric_period_pack >();

  // Valid for the period of time specified in METRIC_PERIOD_PACK field
  auto const time_interval =
    interval_t{ timestamp, timestamp + metric_period.offset };

  // Each 1108 local set can have multiple metrics, each contained in its own
  // metric local set. Items in the parent set are shared among the metric
  // sets. We want to create one index for each *metric set*, copying the
  // parent set's common data to each.
  for( auto const& metric_set_entry :
       value.all_at( KLV_1108_METRIC_LOCAL_SET ) )
  {
    // Create the and fill the index for this metric set
    auto const index = indexify_1108( value, metric_set_entry.second );
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
}

// ----------------------------------------------------------------------------
void
klv_demuxer
::demux_single_entry( klv_top_level_tag standard, klv_lds_key tag,
                      klv_value const& index, interval_t const& time_interval,
                      klv_value const& value )
{
  auto const key = klv_timeline::key_t{ standard, tag, index };
  if( value.empty() )
  {
    // Null value: erase timespan instead of adding a null entry
    m_cancel_points.emplace( key, time_interval.lower() );

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
    auto adjusted_interval = time_interval;
    auto const cancel_range = m_cancel_points.equal_range( key );
    for( auto it = cancel_range.first; it != cancel_range.second; ++it )
    {
      auto const cancel_time = it->second;
      if( cancel_time >= adjusted_interval.lower() )
      {
        adjusted_interval.truncate_upper( cancel_time );
      }
    }
    auto const it = m_timeline.insert_or_find( standard, tag, index );
    it->second.weak_set( adjusted_interval, value );
    auto const jt = it->second.find( adjusted_interval.lower() );
    if( jt != it->second.end() )
    {
      it->second.set( { adjusted_interval.lower(), jt->key_interval.upper() },
                      value );
    }
  }
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
