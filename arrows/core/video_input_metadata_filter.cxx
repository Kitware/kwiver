// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "video_input_metadata_filter.h"

#include <vital/algo/metadata_filter.h>

#include <vital/exceptions.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class video_input_metadata_filter::priv
{
public:
  kv::image_container_scptr current_image_for_transform() const;

  kv::metadata_vector transform_frame_metadata(
    kv::metadata_vector const&,
    kv::image_container_scptr const& ) const;

  kv::metadata_vector transform_current_frame_metadata() const;

  vital::algo::video_input_sptr video_input;
  vital::algo::metadata_filter_sptr metadata_filter;

  bool filter_uses_image = true;
};

// ----------------------------------------------------------------------------
kv::image_container_scptr
video_input_metadata_filter::priv
::current_image_for_transform() const
{
  return ( this->filter_uses_image
    ? this->video_input->frame_image()
    : nullptr );
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_metadata_filter::priv
::transform_frame_metadata(
  kv::metadata_vector const& in,
  kv::image_container_scptr const& image ) const
{
  if( !metadata_filter )
  {
    return in;
  }

  return metadata_filter->filter( in, image );
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_metadata_filter::priv
::transform_current_frame_metadata() const
{
  return this->transform_frame_metadata(
    this->video_input->frame_metadata(), this->current_image_for_transform() );
}

// ----------------------------------------------------------------------------
video_input_metadata_filter
::video_input_metadata_filter()
  : m_d( new video_input_metadata_filter::priv )
{
  attach_logger( "arrows.core.video_input_metadata_filter" );
}

// ----------------------------------------------------------------------------
video_input_metadata_filter
::~video_input_metadata_filter()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
video_input_metadata_filter
::get_configuration() const
{
  auto config = vital::algo::video_input::get_configuration();

  vital::algo::video_input::get_nested_algo_configuration(
    "video_input", config, m_d->video_input );
  vital::algo::metadata_filter::get_nested_algo_configuration(
    "metadata_filter", config, m_d->metadata_filter );

  return config;
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::set_configuration( vital::config_block_sptr in_config )
{
  auto config = this->get_configuration();
  config->merge_config( in_config );

  vital::algo::video_input::set_nested_algo_configuration(
    "video_input", config, m_d->video_input );
  vital::algo::metadata_filter::set_nested_algo_configuration(
    "metadata_filter", config, m_d->metadata_filter );

  if( m_d->metadata_filter )
  {
    auto const& caps = m_d->metadata_filter->get_implementation_capabilities();
    m_d->filter_uses_image =
      caps.capability( vital::algo::metadata_filter::CAN_USE_FRAME_IMAGE );
  }
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::check_configuration(
  vital::config_block_sptr config ) const
{
  return
    vital::algo::video_input::check_nested_algo_configuration(
      "video_input", config ) &&
    vital::algo::metadata_filter::check_nested_algo_configuration(
      "metadata_filter", config );
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::open( std::string name )
{
  if( !m_d->video_input )
  {
    VITAL_THROW( kv::algorithm_configuration_exception,
                 type_name(), impl_name(), "invalid video_input." );
  }
  m_d->video_input->open( name );

  auto const& vi_caps = m_d->video_input->get_implementation_capabilities();

  using vi = vital::algo::video_input;
  using cn = kv::algorithm_capabilities::capability_name_t;

  auto copy_capability =
    [ & ]( cn const& cap ){
      this->set_capability( cap, vi_caps.capability( cap ) );
    };

  // Pass through capabilities
  copy_capability( vi::HAS_EOV );
  copy_capability( vi::HAS_FRAME_NUMBERS );
  copy_capability( vi::HAS_FRAME_DATA );
  copy_capability( vi::HAS_FRAME_TIME );
  copy_capability( vi::HAS_METADATA );
  copy_capability( vi::HAS_ABSOLUTE_FRAME_TIME );
  copy_capability( vi::HAS_TIMEOUT );
  copy_capability( vi::IS_SEEKABLE );
  copy_capability( vi::HAS_RAW_IMAGE );
  copy_capability( vi::HAS_RAW_METADATA );
  copy_capability( vi::HAS_UNINTERPRETED_DATA );
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::close()
{
  if( m_d->video_input )
  {
    m_d->video_input->close();
  }
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::next_frame( kv::timestamp& ts, uint32_t timeout )
{
  if( !m_d->video_input )
  {
    return false;
  }

  return m_d->video_input->next_frame( ts, timeout );
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::seek_frame(
  kv::timestamp& ts,
  kv::timestamp::frame_t frame_number,
  uint32_t timeout )
{
  if( !m_d->video_input )
  {
    return false;
  }

  return m_d->video_input->seek_frame( ts, frame_number, timeout );
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
video_input_metadata_filter
::frame_image()
{
  if( !m_d->video_input )
  {
    return nullptr;
  }

  return m_d->video_input->frame_image();
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_metadata_filter
::frame_metadata()
{
  if( !m_d->video_input )
  {
    return {};
  }

  return m_d->transform_current_frame_metadata();
}

// ----------------------------------------------------------------------------
kv::video_raw_image_sptr
video_input_metadata_filter
::raw_frame_image()
{
  if( !m_d->video_input )
  {
    return nullptr;
  }
  return m_d->video_input->raw_frame_image();
}

// ----------------------------------------------------------------------------
kv::video_uninterpreted_data_sptr
video_input_metadata_filter
::uninterpreted_frame_data()
{
  if( !m_d->video_input )
  {
    return nullptr;
  }

  return m_d->video_input->uninterpreted_frame_data();
}

// ----------------------------------------------------------------------------
kv::metadata_map_sptr
video_input_metadata_filter
::metadata_map()
{
  if( !m_d->video_input )
  {
    return std::make_shared< kv::simple_metadata_map >();
  }

  if( !m_d->metadata_filter )
  {
    return m_d->video_input->metadata_map();
  }

  auto out = vital::metadata_map::map_metadata_t{};

  if( m_d->filter_uses_image )
  {
    if( !m_d->video_input->seekable() )
    {
      return std::make_shared< kv::simple_metadata_map >();
    }

    auto const was_at_end = m_d->video_input->end_of_video();
    auto const previous_frame =
      m_d->video_input->frame_timestamp().get_frame();

    auto ts = kv::timestamp{};
    if( !m_d->video_input->seek_frame( ts, 0 ) )
    {
      return std::make_shared< kv::simple_metadata_map >();
    }

    while( !m_d->video_input->end_of_video() )
    {
      out.emplace( ts.get_frame(), m_d->transform_current_frame_metadata() );

      if( !m_d->video_input->next_frame( ts ) )
      {
        break;
      }
    }

    if( !was_at_end )
    {
      m_d->video_input->seek_frame( ts, previous_frame );
    }

    return std::make_shared< kv::simple_metadata_map >( out );
  }
  else
  {
    auto const& map_ptr = m_d->video_input->metadata_map();
    if( !map_ptr )
    {
      return nullptr;
    }

    for( auto const& i : map_ptr->metadata() )
    {
      out.emplace(
        i.first, m_d->transform_frame_metadata( i.second, nullptr ) );
    }

    return std::make_shared< kv::simple_metadata_map >( out );
  }
}

// ----------------------------------------------------------------------------
kwiver::vital::video_settings_uptr
video_input_metadata_filter
::implementation_settings() const
{
  return m_d->video_input->implementation_settings();
}

// ----------------------------------------------------------------------------
#define FORWARD_OR( name, fallback ) \
  auto video_input_metadata_filter::name() const \
    -> decltype( m_d->video_input->name() ) \
  { \
    if( m_d->video_input ) \
    { \
      return m_d->video_input->name(); \
    } \
    return fallback; \
  }

FORWARD_OR( end_of_video, true )
FORWARD_OR( good, false )
FORWARD_OR( seekable, false )
FORWARD_OR( num_frames, 0 )
FORWARD_OR( frame_timestamp, {} )

} // namespace core

} // namespace arrows

} // namespace kwiver
