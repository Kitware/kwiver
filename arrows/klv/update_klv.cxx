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

#include <vital/range/iterator_range.h>

#include <list>
#include <optional>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class update_klv::impl
{
public:
  using st1108_buffer_t = std::list< std::vector< klv_packet > >;
  using md_buffer_t = std::list< vital::metadata_vector >;

  struct stream
  {
    stream();
    stream( stream const& ) = delete;
    stream( stream&& ) = delete;
    stream& operator=( stream const& ) = delete;
    stream& operator=( stream&& ) = delete;

    klv_timeline timeline;
    klv_demuxer demuxer;
    st1108_buffer_t st1108_buffer;
  };

  impl();

  stream& get_stream( int index );

  std::vector< klv_packet > create_1108_packets(
    vital::metadata const& desired_data,
    vital::metadata const& present_data,
    uint64_t timestamp );

  void combine_1108_packets( st1108_buffer_t& packet_frames, size_t count );

  void flush( size_t count );

  std::map< int, std::unique_ptr< stream > > streams;
  md_buffer_t in_buffer;
  md_buffer_t out_buffer;

  size_t st1108_frequency;
  std::string st1108_inter;
};

// ----------------------------------------------------------------------------
update_klv::impl::stream
::stream()
  : timeline{},
    demuxer{ timeline },
    st1108_buffer{}
{}

// ----------------------------------------------------------------------------
update_klv::impl
::impl()
 : streams{},
   st1108_frequency{ 1 },
   st1108_inter{ "sample" }
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

    // Determine strings describing each metric
    std::string metric_name;
    std::string version;
    std::string parameters;
    switch( vital_tag )
    {
      case vital::VITAL_META_AVERAGE_GSD:
        metric_name = "GSD";
        version = "";
        parameters = "Geo. mean of horiz. and vert. GSD of central pixel";
        break;
      case vital::VITAL_META_VNIIRS:
        metric_name = "VNIIRS";
        version = "GIQE5";
        parameters = "Terms a0, a1 only";
        break;
      default:
        break;
    }

    // Time of metric calculation
    auto const metric_time =
      static_cast< uint64_t >( klv::misp_microseconds_now().count() );

    // Package up this metric's info
    klv_local_set metric_set{
      { KLV_1108_METRIC_SET_NAME, metric_name },
      { KLV_1108_METRIC_SET_VERSION, version },
      { KLV_1108_METRIC_SET_IMPLEMENTER,
        klv_1108_kwiver_metric_implementer() },
      { KLV_1108_METRIC_SET_PARAMETERS, parameters },
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
void
update_klv::impl
::combine_1108_packets( st1108_buffer_t& packet_frames, size_t count )
{
  if( !count )
  {
    return;
  }

  // Sanity check
  if( packet_frames.size() < count )
  {
    throw std::logic_error( "Cannot combine more packets than exist" );
  }

  // Count off the right number of packets
  vital::range::iterator_range< st1108_buffer_t::iterator > const frame_range{
    packet_frames.begin(), std::next( packet_frames.begin(), count ) };

  // Initialize the metric averaging data
  std::map< klv_local_set, std::pair< double, size_t > > means;
  std::map< klv_local_set, uint64_t > metric_times;
  if( st1108_inter == "mean" )
  {
    for( auto const& packet : packet_frames.front() )
    {
      auto const& parent_set = packet.value.get< klv_local_set >();
      for( auto const& entry : parent_set.all_at( KLV_1108_METRIC_LOCAL_SET ) )
      {
        auto const index_set =
          klv_1108_create_index_set( parent_set, entry.second );
        auto const& metric_set = entry.second.get< klv_local_set >();

        // Set initial value
        auto const value =
          metric_set
          .at( KLV_1108_METRIC_SET_VALUE )
          .get< klv_lengthy< double > >()
          .value;
        means.emplace( index_set, std::make_pair( value, 1 ) );

        // Set initial metric calculation timestamp
        auto const timestamp =
          metric_set
          .at( KLV_1108_METRIC_SET_TIME )
          .get< uint64_t >();
        metric_times.emplace( index_set, timestamp );
      }
    }
  }

  // Determine the range of time this group of frames spans
  uint64_t end_timestamp = 0;
  if( st1108_inter != "sample" )
  {
    for( auto const& packet_frame : frame_range )
    {
      for( auto const& packet : packet_frame )
      {
        auto const& period_pack =
          packet.value
          .get< klv_local_set >()
          .at( KLV_1108_METRIC_PERIOD_PACK )
          .get< klv_1108_metric_period_pack >();
        end_timestamp =
          std::max( end_timestamp, period_pack.timestamp + period_pack.offset );
      }
    }
  }

  // Accumulate the metric averaging data, clearing as we go
  for( auto it = std::next( frame_range.begin() );
       it != frame_range.end(); ++it )
  {
    if( st1108_inter == "mean" )
    {
      for( auto const& packet : *it )
      {
        auto const& parent_set = packet.value.get< klv_local_set >();
        for( auto const& entry :
             parent_set.all_at( KLV_1108_METRIC_LOCAL_SET ) )
        {
          auto const index_set =
            klv_1108_create_index_set( parent_set, entry.second );
          auto const& metric_set = entry.second.get< klv_local_set >();

          // Update the value averaging data
          auto const value =
            metric_set
            .at( KLV_1108_METRIC_SET_VALUE )
            .get< klv_lengthy< double > >()
            .value;
          auto const jt =
            means.emplace( index_set, std::make_pair( 0.0, 0 ) ).first;
          jt->second.first += value;
          ++jt->second.second;

          // Update the calculation timestamp
          auto const timestamp =
            metric_set
            .at( KLV_1108_METRIC_SET_TIME )
            .get< uint64_t >();
          auto const result = metric_times.emplace( index_set, timestamp );
          if( !result.second )
          {
            result.first->second = std::max( timestamp, result.first->second );
          }
        }
      }
    }
    it->clear();
  }

  // Modify the first frame's packets as appropriate
  for( auto& packet : packet_frames.front() )
  {
    // Set the timestamp to cover the whole time period
    if( st1108_inter != "sample" )
    {
      auto& period_pack =
        packet.value
        .get< klv_local_set >()
        .at( KLV_1108_METRIC_PERIOD_PACK )
        .get< klv_1108_metric_period_pack >();
      period_pack.offset = end_timestamp - period_pack.timestamp;
    }

    // Set each metrics to the average value
    if( st1108_inter == "mean" )
    {
      auto& parent_set = packet.value.get< klv_local_set >();

      // We need to clear and then re-create all metric sets in the first frame,
      // in case there are metrics that exist in other frames that don't in the
      // first frame
      parent_set.erase( KLV_1108_METRIC_LOCAL_SET );
      for( auto const& entry : means )
      {
        auto metric_set =
          entry.first.at( KLV_1108_METRIC_LOCAL_SET ).get< klv_local_set >();

        // Add metric calculation timestamp
        metric_set.add(
          KLV_1108_METRIC_SET_TIME, metric_times.at( entry.first ) );

        // Add average metric value
        auto const mean = entry.second.first / entry.second.second;
        metric_set.add(
          KLV_1108_METRIC_SET_VALUE, klv_lengthy< double >{ mean, 8 } );

        parent_set.add( KLV_1108_METRIC_LOCAL_SET, std::move( metric_set ) );
      }
    }
  }
}

// ----------------------------------------------------------------------------
void
update_klv::impl
::flush( size_t count )
{
  // Combine all the packets in each stream
  for( auto& entry : streams )
  {
    combine_1108_packets( entry.second->st1108_buffer, count );
  }

  // Collect all the packets from all the streams and put them in the right
  // metadata objects for each frame
  for( size_t i = 0; i < count; ++i )
  {
    for( auto& entry : streams )
    {
      for( auto& md : in_buffer.front() )
      {
        auto const stream_index =
          md->find( vital::VITAL_META_VIDEO_DATA_STREAM_INDEX ).get< int >();
        if( stream_index != entry.first )
        {
          continue;
        }

        auto const klv_md = dynamic_cast< klv_metadata* >( md.get() );
        if( !klv_md )
        {
          continue;
        }

        // Found the metadata object that corresponds to this stream on this
        // frame; put the packets inside
        auto& packets = entry.second->st1108_buffer.front();
        klv_md->klv().insert(
          klv_md->klv().end(),
          std::make_move_iterator( packets.begin() ),
          std::make_move_iterator( packets.end() ) );

        break;
      }

      // Delete packet storage
      entry.second->st1108_buffer.pop_front();
    }

    // Mark this frame's metadata as finished
    out_buffer.splice( out_buffer.end(), in_buffer, in_buffer.begin() );
  }
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
  auto config = algorithm::get_configuration();

  config->set_value(
    "st1108_frequency", d->st1108_frequency,
    "How often (in frames) to encode a ST1108 packet." );
  config->set_value(
    "st1108_inter", d->st1108_inter,
    "How to deal with a group of multiple frames when st1108_frequency > 1. "

    "'sample' will create a packet with the metric values of the first frame "
    "of the group and associate it with the first frame only, leaving the rest "
    "of the frames in the group with no associated values. "

    "'sample_smear' will create a packet with the metric values of the first "
    "frame of the group and associate it with all frames in the group. "

    "'mean' will create a packet with the averages of the group's metric "
    "values and associate it with all frames in the group." );

  return config;
}

// ----------------------------------------------------------------------------
void
update_klv
::set_configuration( vital::config_block_sptr config )
{
  auto existing_config = algorithm::get_configuration();
  d->st1108_frequency =
    config->get_value< size_t >( "st1108_frequency", 1 );
  d->st1108_inter =
    config->get_value< std::string >( "st1108_inter", "sample" );

  existing_config->merge_config( config );
}

// ----------------------------------------------------------------------------
bool
update_klv
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  static std::set< std::string > st1108_inter_options{
    "sample", "sample_smear", "mean"
  };
  return st1108_inter_options.count(
    config->get_value< std::string >( "st1108_inter", "sample" ) );
}

// ----------------------------------------------------------------------------
size_t
update_klv
::send(
  vital::metadata_vector const& input_metadata,
  VITAL_UNUSED vital::image_container_scptr const& input_image )
{
  auto& metadata = d->in_buffer.emplace_back();
  for( auto const& input_md : input_metadata )
  {
    // Copy input metadata
    if ( !input_md )
    {
      metadata.emplace_back( nullptr );
      continue;
    }
    auto& md = *metadata.emplace_back( input_md->clone() );

    auto const klv_md = dynamic_cast< klv_metadata* >( &md );
    if( !klv_md )
    {
      // No KLV
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
    stream.st1108_buffer.emplace_back(
      d->create_1108_packets( *derived_md, md, timestamp ) );
  }

  // Fill streams with empty frames if they didn't have any associated metadata
  for( auto& stream : d->streams )
  {
    while( stream.second->st1108_buffer.size() < d->in_buffer.size() )
    {
      stream.second->st1108_buffer.emplace_back();
    }
  }

  // Process the batched frames
  if( d->in_buffer.size() >= d->st1108_frequency )
  {
    d->flush( d->st1108_frequency );
  }

  return available_frames();
}

// ----------------------------------------------------------------------------
vital::metadata_vector
update_klv
::receive()
{
  if( d->out_buffer.empty() )
  {
    throw std::runtime_error(
      "update_klv: receive() called with no available frames" );
  }

  auto result = std::move( d->out_buffer.front() );
  d->out_buffer.pop_front();
  return result;
}

// ----------------------------------------------------------------------------
size_t
update_klv
::flush()
{
  d->flush( d->in_buffer.size() );
  return available_frames();
}

// ----------------------------------------------------------------------------
size_t
update_klv
::available_frames() const
{
  return d->out_buffer.size();
}

// ----------------------------------------------------------------------------
size_t
update_klv
::unavailable_frames() const
{
  return d->in_buffer.size();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
