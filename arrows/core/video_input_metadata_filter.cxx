// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "video_input_metadata_filter.h"

#include <vital/exceptions.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class video_input_metadata_filter::priv
{
public:
  vital::algo::video_input_sptr video_input;
};

// ----------------------------------------------------------------------------
video_input_metadata_filter
::video_input_metadata_filter()
  : m_d( new video_input_metadata_filter::priv )
{
  attach_logger( "arrows.core.video_input_metadata_filter" );
}

// ------------------------------------------------------------------
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
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::check_configuration(
  vital::config_block_sptr config ) const
{
  return vital::algo::video_input::check_nested_algo_configuration(
    "video_input", config );
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::open( std::string name )
{
  if( !m_d->video_input )
  {
    VITAL_THROW( kwiver::vital::algorithm_configuration_exception,
                 type_name(), impl_name(), "invalid video_input." );
  }
  m_d->video_input->open( name );

  auto const& vi_caps = m_d->video_input->get_implementation_capabilities();

  using vi = vital::algo::video_input;
  using cn = kwiver::vital::algorithm_capabilities::capability_name_t;

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
::next_frame( kwiver::vital::timestamp& ts, uint32_t timeout )
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
  kwiver::vital::timestamp& ts,
  kwiver::vital::timestamp::frame_t frame_number,
  uint32_t timeout )
{
  if( !m_d->video_input )
  {
    return false;
  }

  return m_d->video_input->seek_frame( ts, frame_number, timeout );
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
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
kwiver::vital::metadata_vector
video_input_metadata_filter
::frame_metadata()
{
  if( !m_d->video_input )
  {
    return {};
  }

  return this->transform_frame_metadata(
    m_d->video_input->frame_metadata() );
}

// ----------------------------------------------------------------------------
kwiver::vital::metadata_map_sptr
video_input_metadata_filter
::metadata_map()
{
  if( !m_d->video_input )
  {
    return {};
  }

  auto const& map_ptr = m_d->video_input->metadata_map();
  if( !map_ptr )
  {
    return nullptr;
  }

  auto out = vital::metadata_map::map_metadata_t{};
  for( auto const& i : map_ptr->metadata() )
  {
    out.emplace( i.first, this->transform_frame_metadata( i.second ) );
  }

  return std::make_shared< kwiver::vital::simple_metadata_map >( out );
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
