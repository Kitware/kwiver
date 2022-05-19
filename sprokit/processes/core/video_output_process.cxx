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

create_algorithm_name_config_trait( video_writer );

// -----------------------------------------------------------------------------
// Private implementation class
class video_output_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values
  std::string                           m_video_filename;
  bool                                  m_exit_on_invalid;

  kwiver::vital::algo::video_output_sptr m_video_writer;
  kwiver::vital::algorithm_capabilities m_video_traits;
  kwiver::vital::image_container_sptr   m_last_frame;
  bool                                  m_first_frame;

  kwiver::vital::metadata_vector        m_last_metadata;

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

  // instantiate a video reader
  vital::video_settings default_settings;
  d->m_video_writer->open( d->m_video_filename, &default_settings ); // throws

  //d->m_video_traits = d->m_video_writer->get_implementation_capabilities();
}


// -----------------------------------------------------------------------------
void video_output_process
::_step()
{
  vital::image_container_sptr frame = grab_from_port_using_trait( image );
  vital::timestamp ts = grab_from_port_using_trait( timestamp );

  if( !frame )
  {
    if( d->m_exit_on_invalid )
    {
      VITAL_THROW( sprokit::invalid_configuration_exception, name(),
                   "Invalid image received" );
    }
    else
    {
      frame = d->m_last_frame;
    }
  }
  else
  {
    d->m_last_frame = frame;
    d->m_first_frame = false;
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
    m_first_frame( true )
{
}

video_output_process::priv
::~priv()
{
}

} // end namespace
