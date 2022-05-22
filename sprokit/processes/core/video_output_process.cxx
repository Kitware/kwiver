// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "video_output_process.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/types/image_container.h>
#include <vital/types/image.h>

#include <vital/algo/video_output.h>
#include <vital/exceptions.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>
#include <sprokit/pipeline/datum.h>

#ifdef WITH_FFMPEG
#include <arrows/ffmpeg/ffmpeg_video_settings.h>
#endif

#include <algorithm>
#include <string>

namespace algo = kwiver::vital::algo;

namespace kwiver {

// (config-key, value-type, default-value, description )
create_config_trait( video_filename, std::string, "",
  "Name of output video file." );

create_config_trait( exit_on_invalid, bool, "false",
  "If a frame in the middle of a sequence is invalid, do not "
  "exit and throw an error, continue processing data. If the "
  "first frame cannot be read, always exit regardless of this "
  "setting." );

create_config_trait( maximum_length, double, "-1.0",
  "Maximum output video length (in seconds) if this length is "
  "exceeded, multiple video files less than this amount will be "
  "output with a timestamp start extension." );

create_algorithm_name_config_trait( video_writer );

// -----------------------------------------------------------------------------
// Private implementation class
class video_output_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values
  std::string                            m_video_filename;
  bool                                   m_exit_on_invalid;

  kwiver::vital::algo::video_output_sptr m_video_writer;
  kwiver::vital::algorithm_capabilities  m_video_traits;
  double                                 m_maximum_length;

  double                                 m_frame_rate;
  bool                                   m_is_first_frame;
  double                                 m_clip_start_time;
  kwiver::vital::image_container_sptr    m_last_frame;
  kwiver::vital::metadata_vector         m_last_metadata;

}; // end priv class


// =============================================================================
video_output_process
::video_output_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new video_output_process::priv )
{
  make_ports();
  make_config();
}

video_output_process
::~video_output_process()
{
  if( d->m_video_writer )
  {
    d->m_video_writer->close();
  }
}


// -----------------------------------------------------------------------------
void video_output_process
::_configure()
{
  scoped_configure_instrumentation();

  // Examine the configuration
  d->m_video_filename = config_value_using_trait( video_filename );
  d->m_exit_on_invalid = config_value_using_trait( exit_on_invalid );
  d->m_maximum_length = config_value_using_trait( maximum_length );

  vital::config_block_sptr algo_config = get_config();

  if( !algo::video_output::check_nested_algo_configuration_using_trait(
         video_writer, algo_config ) )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
                 "Configuration check failed." );
  }

  // instantiate requested/configured algo type
  algo::video_output::set_nested_algo_configuration_using_trait(
    video_writer,
    algo_config,
    d->m_video_writer );

  if( !d->m_video_writer )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
                 "Unable to create video_writer." );
  }
}


// -----------------------------------------------------------------------------
// Post connection initialization
void video_output_process
::_init()
{
  scoped_init_instrumentation();

  d->m_is_first_frame = true;
  d->m_clip_start_time = -1.0;
}


// -----------------------------------------------------------------------------
void video_output_process
::_step()
{
  bool reset = false;

  vital::image_container_sptr frame = grab_from_port_using_trait( image );
  vital::timestamp ts = grab_from_port_using_trait( timestamp );

  if( !frame )
  {
    if( d->m_exit_on_invalid )
    {
      VITAL_THROW( vital::image_exception, "Invalid image received" );
    }
    else
    {
      frame = d->m_last_frame;
    }
  }
  else
  {
    d->m_last_frame = frame;
  }

  if( d->m_is_first_frame && has_input_port_edge_using_trait( frame_rate ) )
  {
    d->m_frame_rate = grab_from_port_using_trait( frame_rate );
  }

  if( d->m_maximum_length > 0.0 )
  {
    double current_seconds = ts.get_time_seconds();
    double clip_start_time = d->m_maximum_length *
      static_cast< int >( current_seconds / d->m_maximum_length );

    if( clip_start_time != d->m_clip_start_time )
    {
      d->m_clip_start_time = clip_start_time;
      reset = true;
    }
  }

  if( d->m_is_first_frame || reset )
  {
    // instantiate a video reader
#ifdef WITH_FFMPEG
    arrows::ffmpeg::ffmpeg_video_settings default_settings;
    default_settings.frame_rate = av_d2q( d->m_frame_rate, 1e9 );
    default_settings.parameters->width = frame->width();
    default_settings.parameters->height = frame->height();
#else
    vital::video_settings default_settings;
#endif
    std::string filename = d->m_video_filename;

    if( reset )
    {
      std::size_t pos = filename.find_last_of( "." );
      std::string stem = filename.substr( 0, pos );
      std::string ext = ( pos == std::string::npos ? "mp4" : filename.substr( pos ) );

      unsigned seconds_int = static_cast< unsigned >( d->m_clip_start_time );
      std::string h = std::to_string( seconds_int / 3600 );
      std::string m = std::to_string( ( seconds_int / 60 ) % 60 );
      std::string s = std::to_string( seconds_int % 60 );

      std::string time_str = "_" +
        std::string( 2 - std::min( std::size_t( 2 ), h.length() ), '0' ) + h + "h_" +
        std::string( 2 - std::min( std::size_t( 2 ), m.length() ), '0' ) + m + "m_" +
        std::string( 2 - std::min( std::size_t( 2 ), s.length() ), '0' ) + s + "s";

      filename = stem + time_str + ext;
    }

    d->m_video_writer->open( filename, &default_settings ); // throws
    d->m_is_first_frame = false;
  }

  d->m_video_writer->add_image( frame, ts );

  if( has_input_port_edge_using_trait( metadata ) )
  {
    for( auto meta : grab_from_port_using_trait( metadata ) )
    {
      if( meta )
      {
        d->m_video_writer->add_metadata( *meta );
      }
    }
  }
}


// -----------------------------------------------------------------------------
void video_output_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;

  // We are outputting a shared ref to the output image, therefore we
  // should mark it as shared.
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_input_port_using_trait( image, required );
  declare_input_port_using_trait( timestamp, required );
  declare_input_port_using_trait( metadata, optional );
  declare_input_port_using_trait( frame_rate, optional );
}


// -----------------------------------------------------------------------------
void video_output_process
::make_config()
{
  declare_config_using_trait( video_filename );
  declare_config_using_trait( exit_on_invalid );
  declare_config_using_trait( video_writer );
}


// =============================================================================
video_output_process::priv
::priv()
  : m_exit_on_invalid( true ),
    m_is_first_frame( true ),
    m_clip_start_time( -1.0 )
{
}

video_output_process::priv
::~priv()
{
}

} // end namespace
