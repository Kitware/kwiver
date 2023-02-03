// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of update_klv metadata filter.

#include <arrows/klv/update_klv.h>

#include <arrows/klv/klv_demuxer.h>
#include <arrows/klv/klv_metadata.h>
#include <arrows/klv/klv_timeline.h>
#include <arrows/klv/klv_convert_vital.h>
#include <arrows/klv/klv_1108.h>
#include <arrows/klv/klv_1108_metric_set.h>
#include <arrows/klv/misp_time.h>

#include <optional>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class update_klv::impl
{
public:
  struct stream
  {
    stream();
    stream( stream const& ) = delete;
    stream( stream&& ) = delete;
    stream& operator=( stream const& ) = delete;
    stream& operator=( stream&& ) = delete;

    klv_timeline timeline;
    klv_demuxer demuxer;
  };

  impl();

  stream& get_stream( int index );

  std::vector< klv_packet > create_1108_packets(
    vital::metadata const& desired_data,
    vital::metadata const& present_data,
    uint64_t timestamp );

  std::map< int, std::unique_ptr< stream > > streams;
};

// ----------------------------------------------------------------------------
update_klv::impl::stream
::stream()
  : timeline{},
    demuxer{ timeline }
{}

// ----------------------------------------------------------------------------
update_klv::impl
::impl()
 : streams{}
{}

// ----------------------------------------------------------------------------
update_klv::impl::stream&
update_klv::impl
::get_stream( int index )
{
  // Create stream if none exists at that index
  auto result = streams.emplace( index, nullptr );
  if( result.second )
  {
    result.first->second.reset( new stream );
  }

  return *result.first->second;
}

// ----------------------------------------------------------------------------
std::vector< klv_packet >
update_klv::impl
::create_1108_packets(
  vital::metadata const& desired_data,
  vital::metadata const& present_data,
  uint64_t timestamp )
{
  klv_local_set set;

  // Derive ST1108 video quality fields from vital metadata
  if( !klv_1108_fill_in_metadata( present_data, set ) )
  {
    return {};
  }

  // Duration of metrics should be the duration of one frame
  auto const frame_rate =
    present_data.find( vital::VITAL_META_VIDEO_FRAME_RATE ).as_double();
  klv_1108_metric_period_pack period_pack{
    timestamp, static_cast< uint32_t >( 1000000.0 / frame_rate ) };
  set.add( KLV_1108_METRIC_PERIOD_PACK, period_pack );

  // Add metric-specific data
  for( auto const vital_tag : {
    vital::VITAL_META_AVERAGE_GSD,
    vital::VITAL_META_VNIIRS
  } )
  {
    auto const desired_entry = desired_data.find( vital_tag );
    auto const present_entry = present_data.find( vital_tag );

    // Don't insert anything if the metrics are already in the stream
    if( desired_entry == present_entry || !present_entry )
    {
      continue;
    }

    std::string metric_name;
    switch( vital_tag )
    {
      case vital::VITAL_META_AVERAGE_GSD:
        metric_name = "GSD";
        break;
      case vital::VITAL_META_VNIIRS:
        metric_name = "VNIIRS";
        break;
      default:
        break;
    }

    // Time of metric calculation
    auto const metric_time =
      static_cast< uint64_t >( klv::misp_microseconds_now().count() );

    // Package up this metric's info
    // TODO: What to put for version and parameters?
    klv_local_set metric_set{
      { KLV_1108_METRIC_SET_NAME, metric_name },
      { KLV_1108_METRIC_SET_VERSION, std::string{} },
      { KLV_1108_METRIC_SET_IMPLEMENTER,
        klv_1108_kwiver_metric_implementer() },
      { KLV_1108_METRIC_SET_PARAMETERS, std::string{} },
      { KLV_1108_METRIC_SET_TIME, metric_time },
      { KLV_1108_METRIC_SET_VALUE,
        klv_lengthy< double >{ present_entry.as_double(), 8 } }
    };

    // Put this metric in the set
    set.add( KLV_1108_METRIC_LOCAL_SET, std::move( metric_set ) );
  }

  // Have any metrics been calculated?
  if( set.has( KLV_1108_METRIC_LOCAL_SET ) )
  {
    return { klv_packet{ klv_1108_key(), std::move( set ) } };
  }

  return {};
}

// ----------------------------------------------------------------------------
update_klv
::update_klv()
  : d{ new impl }
{}

// ----------------------------------------------------------------------------
update_klv
::~update_klv()
{}

// ----------------------------------------------------------------------------
vital::config_block_sptr
update_klv
::get_configuration() const
{
  return algorithm::get_configuration();
}

// ----------------------------------------------------------------------------
void
update_klv
::set_configuration( vital::config_block_sptr config )
{
  auto existing_config = algorithm::get_configuration();
  existing_config->merge_config( config );
}

// ----------------------------------------------------------------------------
bool
update_klv
::check_configuration( vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
vital::metadata_vector
update_klv
::filter(
  vital::metadata_vector const& input_metadata,
  VITAL_UNUSED vital::image_container_scptr const& input_image )
{
  vital::metadata_vector results;
  for( auto const& src_md : input_metadata )
  {
    // Case: null
    if( !src_md )
    {
      results.emplace_back( nullptr );
      continue;
    }

    // Deep copy
    results.emplace_back( src_md->clone() );

    // Case: no KLV
    auto const klv_md = dynamic_cast< klv_metadata* >( results.back().get() );
    if( !klv_md )
    {
      continue;
    }

    // Determine which KLV stream to modify
    auto const index_entry =
      klv_md->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX );
    if( !index_entry )
    {
      continue;
    }

    // Get the state for that stream
    auto& stream = d->get_stream( index_entry.get< int >() );

    // Determine the timestamp for this frame
    auto const timestamp_entry =
      klv_md->find( vital::VITAL_META_UNIX_TIMESTAMP );
    std::optional< uint64_t > backup_timestamp;
    if( timestamp_entry )
    {
      backup_timestamp = timestamp_entry.as_uint64();
    }

    // Determine the current state of these metrics via timeline, to determine
    // whether there already is a ST1108 packet for the metrics we're writing
    stream.demuxer.send_frame( klv_md->klv(), backup_timestamp );

    auto timestamp = stream.demuxer.frame_time();
    auto const timestamp_source_entry =
      klv_md->find( vital::VITAL_META_UNIX_TIMESTAMP_SOURCE );
    if( timestamp_entry && timestamp_source_entry &&
        timestamp_source_entry.as_string() == "misp" )
    {
      timestamp = timestamp_entry.as_uint64();
    }

    auto const derived_md =
      klv_to_vital_metadata( stream.timeline, timestamp );

    // Add any new ST1108 packets
    auto klv_packets = klv_md->klv();
    auto const new_1108_packets =
      d->create_1108_packets( *derived_md, *results.back(), timestamp );
    klv_packets.insert(
      klv_packets.end(), new_1108_packets.begin(), new_1108_packets.end() );
    klv_md->set_klv( klv_packets );
  }

  return results;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
