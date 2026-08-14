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
  priv( video_input_metadata_filter& parent )
    : parent( parent )
  {}

  video_input_metadata_filter& parent;

  kv::image_container_scptr current_image_for_transform() const;

  kv::metadata_vector transform_frame_metadata(
    kv::metadata_vector const&,
    kv::image_container_scptr const& ) const;

  kv::metadata_vector transform_current_frame_metadata() const;

  // processing classes configured in PIMPLE macro
  vital::algo::video_input_sptr
  video_input() const { return parent.c_video_input; }
  vital::algo::metadata_filter_sptr
  metadata_filter() const { return parent.c_metadata_filter; }

  bool filter_uses_image = true;
};

// ----------------------------------------------------------------------------
kv::image_container_scptr
video_input_metadata_filter::priv
::current_image_for_transform() const
{
  return ( this->filter_uses_image
           ? this->video_input()->frame_image()
           : nullptr );
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_metadata_filter::priv
::transform_frame_metadata(
  kv::metadata_vector const& in,
  kv::image_container_scptr const& image ) const
{
  if( !metadata_filter() )
  {
    return in;
  }

  return metadata_filter()->filter( in, image );
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_metadata_filter::priv
::transform_current_frame_metadata() const
{
  return this->transform_frame_metadata(
    this->video_input()->frame_metadata(),
    this->current_image_for_transform() );
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, m_d );
  attach_logger( "arrows.core.video_input_metadata_filter" );
}

// ----------------------------------------------------------------------------
video_input_metadata_filter
::~video_input_metadata_filter()
{}

// ----------------------------------------------------------------------------

void
video_input_metadata_filter
::set_configuration_internal( [[maybe_unused]] vital::config_block_sptr config )
{
  if( m_d->metadata_filter() )
  {
    auto const& caps =
      m_d->metadata_filter()->get_implementation_capabilities();
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
    vital::check_nested_algo_configuration< vital::algo::video_input >(
    "video_input", config ) &&
    vital::check_nested_algo_configuration< vital::algo::metadata_filter >(
    "metadata_filter", config );
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::open( std::string name )
{
  if( !m_d->video_input() )
  {
    VITAL_THROW(
      kv::algorithm_configuration_exception,
      interface_name(), impl_name(), "invalid video_input." );
  }
  m_d->video_input()->open( name );

  auto const& vi_caps = m_d->video_input()->get_implementation_capabilities();

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
  copy_capability( vi::IS_SEEKABLE_BY_FRAME );
  copy_capability( vi::HAS_RAW_IMAGE );
  copy_capability( vi::HAS_RAW_METADATA );
  copy_capability( vi::HAS_UNINTERPRETED_DATA );
  set_capability( vi::IS_SEEKABLE_BY_TIME, false );
}

// ----------------------------------------------------------------------------
void
video_input_metadata_filter
::close()
{
  if( m_d->video_input() )
  {
    m_d->video_input()->close();
  }
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::next_frame( vital::time_usec_t timeout )
{
  if( !m_d->video_input() )
  {
    return false;
  }

  return m_d->video_input()->next_frame( timeout );
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::seek_frame(
  kv::timestamp::frame_t frame_number,
  vital::time_usec_t timeout )
{
  if( !m_d->video_input() )
  {
    return false;
  }

  return m_d->video_input()->seek_frame( frame_number, timeout );
}

// ----------------------------------------------------------------------------
bool
video_input_metadata_filter
::seek_time(
  [[maybe_unused]] kv::timestamp::time_t time_usec,
  [[maybe_unused]] kv::time_usec_t timeout )
{
  // TODO: Unimplemented
  return false;
}

// ----------------------------------------------------------------------------
kv::image_container_sptr
video_input_metadata_filter
::frame_image()
{
  if( !m_d->video_input() )
  {
    return nullptr;
  }

  return m_d->video_input()->frame_image();
}

// ----------------------------------------------------------------------------
kv::metadata_vector
video_input_metadata_filter
::frame_metadata()
{
  if( !m_d->video_input() )
  {
    return {};
  }

  return m_d->transform_current_frame_metadata();
}

// ----------------------------------------------------------------------------
kv::video_raw_metadata_sptr
video_input_metadata_filter
::raw_frame_metadata()
{
  if( !m_d->video_input() )
  {
    return nullptr;
  }

  return m_d->video_input()->raw_frame_metadata();
}

// ----------------------------------------------------------------------------
kv::video_raw_image_sptr
video_input_metadata_filter
::raw_frame_image()
{
  if( !m_d->video_input() )
  {
    return nullptr;
  }
  return m_d->video_input()->raw_frame_image();
}

// ----------------------------------------------------------------------------
kv::video_uninterpreted_data_sptr
video_input_metadata_filter
::uninterpreted_frame_data()
{
  if( !m_d->video_input() )
  {
    return nullptr;
  }

  return m_d->video_input()->uninterpreted_frame_data();
}

// ----------------------------------------------------------------------------
kwiver::vital::video_settings_sptr
video_input_metadata_filter
::implementation_settings() const
{
  return m_d->video_input()->implementation_settings();
}

// ----------------------------------------------------------------------------
#define FORWARD_OR( name, fallback )                          \
auto video_input_metadata_filter::name() const                \
                   ->decltype( m_d->video_input()->name() ) { \
  if( m_d->video_input() )                                    \
  {                                                           \
    return m_d->video_input()->name();                        \
  }                                                           \
  return fallback; }

FORWARD_OR( end_of_video, true )
FORWARD_OR( good, false )
FORWARD_OR( num_frames, 0 )
FORWARD_OR( frame_timestamp, {} )

} // namespace core

} // namespace arrows

} // namespace kwiver
