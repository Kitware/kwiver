// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the video_input_buffered_metadata_filter class.

#include <arrows/core/video_input_buffered_metadata_filter.h>

#include <vital/algo/buffered_metadata_filter.h>

#include <vital/exceptions.h>

#include <chrono>
#include <list>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class video_input_buffered_metadata_filter::impl
{
public:
  impl();

  struct frame_info
  {
    frame_info( kv::algo::video_input& input );

    kv::timestamp timestamp;
    kv::image_container_sptr image;
    kv::video_raw_image_sptr raw_image;
    kv::video_uninterpreted_data_sptr uninterpreted_data;
  };

  kv::algo::video_input_sptr video_input;
  kv::algo::buffered_metadata_filter_sptr filter;
  std::list< frame_info > frames;
  kv::metadata_vector frame_metadata;

  bool use_image;
};

// ----------------------------------------------------------------------------
video_input_buffered_metadata_filter::impl::frame_info
::frame_info( kv::algo::video_input& input )
  : timestamp{ input.frame_timestamp() },
    image{ input.frame_image() },
    raw_image{ input.raw_frame_image() },
    uninterpreted_data{ input.uninterpreted_frame_data() }
{}

// ----------------------------------------------------------------------------
video_input_buffered_metadata_filter::impl
::impl()
  : video_input{ nullptr },
    filter{},
    frames{},
    frame_metadata{},
    use_image{ true }
{}

// ----------------------------------------------------------------------------
video_input_buffered_metadata_filter
::video_input_buffered_metadata_filter() : d{ new impl }
{
  attach_logger( "klv.video_input_buffered_metadata_filter" );
}

// ----------------------------------------------------------------------------
video_input_buffered_metadata_filter
::~video_input_buffered_metadata_filter()
{}

// ----------------------------------------------------------------------------
kv::config_block_sptr
video_input_buffered_metadata_filter
::get_configuration() const
{
  auto config = kv::algo::video_input::get_configuration();

  kv::algo::video_input::get_nested_algo_configuration(
    "video_input", config, d->video_input );
  kv::algo::buffered_metadata_filter::get_nested_algo_configuration(
    "metadata_filter", config, d->filter );

  return config;
}

// ----------------------------------------------------------------------------
void
video_input_buffered_metadata_filter
::set_configuration( kv::config_block_sptr in_config )
{
  auto config = get_configuration();
  config->merge_config( in_config );

  kv::algo::video_input::set_nested_algo_configuration(
    "video_input", config, d->video_input );
  kv::algo::buffered_metadata_filter::set_nested_algo_configuration(
    "metadata_filter", config, d->filter );

  if( d->filter )
  {
    d->use_image =
      d->filter->get_implementation_capabilities()
        .capability( kv::algo::buffered_metadata_filter::CAN_USE_FRAME_IMAGE );
  }
}

// ----------------------------------------------------------------------------
bool
video_input_buffered_metadata_filter
::check_configuration( kv::config_block_sptr config ) const
{
  return
    kv::algo::video_input::check_nested_algo_configuration(
      "video_input", config ) &&
    kv::algo::buffered_metadata_filter::check_nested_algo_configuration(
      "metadata_filter", config );
}

// ----------------------------------------------------------------------------
void
video_input_buffered_metadata_filter
::open( std::string name )
{
  if( !d->video_input )
  {
    VITAL_THROW( kv::algorithm_configuration_exception,
                 type_name(), impl_name(), "Invalid video_input." );
  }

  d->video_input->open( name );

  auto const& capabilities = d->video_input->get_implementation_capabilities();

  using vi = kv::algo::video_input;
  for( auto const& capability : {
    vi::HAS_EOV,
    vi::HAS_FRAME_NUMBERS,
    vi::HAS_FRAME_DATA,
    vi::HAS_FRAME_TIME,
    vi::HAS_METADATA,
    vi::HAS_ABSOLUTE_FRAME_TIME,
    vi::HAS_TIMEOUT,
    vi::HAS_RAW_IMAGE,
    vi::HAS_RAW_METADATA,
    vi::HAS_UNINTERPRETED_DATA, } )
  {
    set_capability( capability, capabilities.capability( capability ) );
  }

  // Only supports single forward pass
  set_capability( vi::IS_SEEKABLE, false );
}

// ----------------------------------------------------------------------------
void
video_input_buffered_metadata_filter
::close()
{
  if( d->video_input )
  {
    d->video_input->close();
    d->video_input.reset();
  }
}

// ----------------------------------------------------------------------------
bool
video_input_buffered_metadata_filter
::end_of_video() const
{
  return
    !d->video_input ||
    ( d->video_input->end_of_video() &&
      ( !d->filter || !d->filter->available_frames() ) );
}

// ----------------------------------------------------------------------------
bool
video_input_buffered_metadata_filter
::good() const
{
  return d->video_input && !d->frames.empty();
}

// ----------------------------------------------------------------------------
bool
video_input_buffered_metadata_filter
::seekable() const
{
  return false;
}

// ----------------------------------------------------------------------------
size_t
video_input_buffered_metadata_filter
::num_frames() const
{
  return d->video_input ? d->video_input->num_frames() : 0;
}

// ----------------------------------------------------------------------------
bool
video_input_buffered_metadata_filter
::next_frame( kv::timestamp& out_ts, uint32_t timeout )
{
  if( end_of_video() )
  {
    out_ts.set_invalid();
    return false;
  }

  // Get rid of the image from the last frame
  if( !d->frames.empty() )
  {
    d->frames.pop_front();
  }

  if( !d->filter )
  {
    kv::timestamp ts;
    if( d->video_input->next_frame( ts, timeout ) )
    {
      d->frames.emplace_back( *d->video_input );
      out_ts = d->frames.front().timestamp;
      return true;
    }
    return false;
  }

  // Ensure there is at least one metadata frame to output
  bool video_error = false;
  while( !d->filter->available_frames() )
  {
    if( d->video_input->end_of_video() || video_error )
    {
      if( d->filter->unavailable_frames() && d->filter->flush() )
      {
        // Found some metadata frames by flushing
        break;
      }

      // No more metadata frames
      if( !d->frames.empty() )
      {
        throw std::logic_error(
          "video_input_buffered_metadata_filter: "
          "filter produced too few metadata frames" );
      }
      out_ts.set_invalid();
      return false;
    }

    // Get the next frame from the embedded video input
    kv::timestamp ts;
    if( !d->video_input->next_frame( ts, timeout ) )
    {
      LOG_DEBUG(
        logger(),
        "Failed to get next frame even though end_of_video() is false" );
      video_error = true;
      continue;
    }
    d->frames.emplace_back( *d->video_input );
    d->filter->send(
      d->video_input->frame_metadata(),
      d->use_image ? d->frames.back().image : nullptr );
  }

  if( d->frames.empty() )
  {
    throw std::logic_error(
      "video_input_buffered_metadata_filter: "
      "filter produced too many metadata frames" );
  }

  // Return next frame in queue
  d->frame_metadata = d->filter->receive();
  out_ts = d->frames.front().timestamp;
  return true;
}

// ----------------------------------------------------------------------------
bool
video_input_buffered_metadata_filter
::seek_frame( kv::timestamp&, kv::timestamp::frame_t, uint32_t )
{
  return false;
}

// ----------------------------------------------------------------------------
kv::timestamp
video_input_buffered_metadata_filter
::frame_timestamp() const
{
  if( end_of_video() || d->frames.empty() )
  {
    return kv::timestamp{};
  }

  return d->frames.front().timestamp;
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
video_input_buffered_metadata_filter
::frame_image()
{
  if( end_of_video() || d->frames.empty() )
  {
    return nullptr;
  }

  return d->frames.front().image;
}

// ----------------------------------------------------------------------------
kv::video_raw_image_sptr
video_input_buffered_metadata_filter
::raw_frame_image()
{
  if( end_of_video() || d->frames.empty() )
  {
    return nullptr;
  }

  return d->frames.front().raw_image;
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_buffered_metadata_filter
::frame_metadata()
{
  if( end_of_video() || d->frames.empty() )
  {
    return {};
  }

  return d->frame_metadata;
}

// ----------------------------------------------------------------------------
vital::video_uninterpreted_data_sptr
video_input_buffered_metadata_filter
::uninterpreted_frame_data()
{
  if( end_of_video() || d->frames.empty() )
  {
    return nullptr;
  }

  return d->frames.front().uninterpreted_data;
}

// ----------------------------------------------------------------------------
kv::metadata_map_sptr
video_input_buffered_metadata_filter
::metadata_map()
{
  return nullptr;
}

// ----------------------------------------------------------------------------
kv::video_settings_uptr
video_input_buffered_metadata_filter
::implementation_settings() const
{
  return d->video_input ? d->video_input->implementation_settings() : nullptr;
}

} // namespace core

} // namespace arrows

} // namespace kwiver
