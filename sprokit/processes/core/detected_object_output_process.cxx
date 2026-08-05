// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation for detected_object_set_output process
 */

#include "detected_object_output_process.h"

#include <vital/vital_types.h>
#include <vital/exceptions.h>
#include <vital/types/timestamp.h>
#include <vital/algo/detected_object_set_output.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver {

// (config-key, value-type, default-value, description )
create_config_trait( file_name, std::string, "", "Name of the detection set file to write." );
create_algorithm_name_config_trait( writer );

/**
 * \class detected_object_output_process
 *
 * \brief Writes detected objects to a file.
 *
 * \process This process writes the detected objecs in the set to a
 * file. The actual renderingwriting is done by the selected \b
 * detected_object_set_output algorithm implementation.
 *
 * \iports
 *
 * \iport{image_file_name} Optional name of an image file to associate
 * with the set of detections.
 *
 * \iport{detected_object_set} Set ob objects to pass to writer
 * algorithm.
 *
 * \configs
 *
 * \config{file_name} Name of the file that the detections are written.
 *
 * \config{writer} Name of the configuration subblock that selects
 * and configures the writing algorithm
 */

//----------------------------------------------------------------
// Private implementation class
class detected_object_output_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values
  std::string m_file_name;
  bool m_write_time_as_uid{ false };

  /// Frame time as HH:MM:SS.ssssss, empty if it cannot be formatted.
  std::string format_time( kwiver::vital::timestamp const& ts ) const;

  algo::detected_object_set_output_sptr m_writer;
}; // end priv class

// ================================================================

detected_object_output_process
::detected_object_output_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new detected_object_output_process::priv )
{
  make_ports();
  make_config();
}

detected_object_output_process
::~detected_object_output_process()
{
}

// ----------------------------------------------------------------
void detected_object_output_process
::_configure()
{
  scoped_configure_instrumentation();

  // Get process config entries
  d->m_file_name = config_value_using_trait( file_name );
  d->m_write_time_as_uid = config_value_using_trait( write_time_as_uid );
  if ( d->m_file_name.empty() )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
             "Required file name not specified." );
  }

  // Get algo conrig entries
  kwiver::vital::config_block_sptr algo_config = get_config(); // config for process

  // validate configuration
  if ( ! check_nested_algo_configuration_using_trait(
         writer,
         algo_config, d->m_writer ) )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(), "Configuration check failed." );
  }

  // instantiate image reader and converter based on config type
  set_nested_algo_configuration_using_trait(
    writer,
    algo_config,
    d->m_writer);
  if ( ! d->m_writer )
  {
    VITAL_THROW( sprokit::invalid_configuration_exception, name(),
             "Unable to create writer." );
  }
}

// ----------------------------------------------------------------
void detected_object_output_process
::_init()
{
  scoped_init_instrumentation();

  d->m_writer->open( d->m_file_name ); // throws
}

// ----------------------------------------------------------------
void detected_object_output_process
::_step()
{
  std::string identifier;

  // image name is optional
  if ( has_input_port_edge_using_trait( image_file_name ) )
  {
    identifier = grab_from_port_using_trait( image_file_name );
  }

  // timestamp is optional, but must always be consumed when connected,
  // otherwise the edge backs up and stalls the pipeline
  if ( has_input_port_edge_using_trait( timestamp ) )
  {
    auto const ts = grab_from_port_using_trait( timestamp );

    // Video has no per-frame file name, so fall back to the frame time there
    // and leave image lists writing their file names
    if ( ( d->m_write_time_as_uid || identifier.empty() ) &&
         ts.has_valid_time() )
    {
      identifier = d->format_time( ts );
    }
  }

  kwiver::vital::detected_object_set_sptr input = grab_from_port_using_trait( detected_object_set );

  {
    scoped_step_instrumentation();

    d->m_writer->write_set( input, identifier );
  }
}

// ----------------------------------------------------------------
void detected_object_output_process
::_finalize()
{
  d->m_writer->complete();
}

// ----------------------------------------------------------------
void detected_object_output_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  declare_input_port_using_trait( image_file_name, optional );
  declare_input_port_using_trait( timestamp, optional );
  declare_input_port_using_trait( detected_object_set, required );
}

// ----------------------------------------------------------------
void detected_object_output_process
::make_config()
{
  declare_config_using_trait( file_name );
  declare_config_using_trait( write_time_as_uid );
  declare_config_using_trait( writer );
}

// ================================================================
detected_object_output_process::priv
::priv()
{
}

detected_object_output_process::priv
::~priv()
{
}

} // end namespace

// ----------------------------------------------------------------
std::string
detected_object_output_process::priv
::format_time( kwiver::vital::timestamp const& ts ) const
{
  const kwiver::vital::time_usec_t usec( 1000000 );
  const std::time_t time_s =
    static_cast< std::time_t >( ts.get_time_usec() / usec );
  const unsigned time_us =
    static_cast< unsigned >( ts.get_time_usec() % usec );

  char buffer[10];
  struct tm* tmp = gmtime( &time_s );

  if( !tmp || !strftime( buffer, sizeof( buffer ), "%H:%M:%S", tmp ) )
  {
    return std::string();
  }

  std::string time_us_str = std::to_string( time_us );
  while( time_us_str.size() < 6 )
  {
    time_us_str = "0" + time_us_str;
  }

  return std::string( buffer ) + "." + time_us_str;
}
