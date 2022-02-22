// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of template process.
 */

#include "matlab_process.h"

#include <sprokit/processes/kwiver_type_traits.h>
#include <vital/types/timestamp.h>

#include <arrows/matlab/matlab_engine.h>
#include <arrows/matlab/matlab_util.h>
#include <arrows/ocv/image_container.h>
#include <kwiversys/SystemTools.hxx>

#include <opencv2/core/core.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <string>
#include <sstream>
#include <fstream>

namespace kwiver {
namespace matlab {

// ----------------------------------------------------------------
/**
 * \class matlab_process
 *
 * \brief Matlab interface template
 *
 * \iports
 *
 * \iport{timestamp} time stamp for incoming images.
 *
 * \iport{image} Input image to be processed.
 *
 * \oports
 *
 * \oport{image} Resulting image
 *
 * \configs
 *
 * \config{header} Header text. (string)
 *
 * \config{footer} Footer text (string)
 */

// config items
// <name>, <type>, <default string>, <description string>
create_config_trait( program_file, std::string, "", "Name of matlab process to interface to." );

//----------------------------------------------------------------
// Private implementation class
class matlab_process::priv
{
public:
  priv( matlab_process* parent );
  ~priv();

  void check_result();
  void eval( const std::string& expr );

  matlab_process* m_parent;

  // Configuration values
  std::string m_program_file;

  // MatLab support. The engine is allocated at the latest time.
  std::shared_ptr< kwiver::arrows::matlab::matlab_engine > m_matlab_engine;

}; // end priv class

// ================================================================
matlab_process
::matlab_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new matlab_process::priv( this ) )
{
  make_ports(); // create process ports
  make_config(); // declare process configuration
}

matlab_process
::~matlab_process()
{
}

// ----------------------------------------------------------------
/**
 * @brief Configure process
 *
 * This method is called prior to connecting ports to allow the
 * process to configure itself.
 */
void
matlab_process
::_configure()
{
  scoped_configure_instrumentation();

  // Need to delay creating engine because it is heavyweight
  d->m_matlab_engine = std::make_shared< kwiver::arrows::matlab::matlab_engine >();

  d->m_program_file = config_value_using_trait( program_file );

  // Create path to program file so we can do addpath('path');
  std::string full_path = kwiversys::SystemTools::CollapseFullPath( d->m_program_file );
  full_path = kwiversys::SystemTools::GetFilenamePath( full_path );

  d->eval( "addpath('" + full_path + "')" );

  // Get config values for this algorithm by extracting the subblock
  auto algo_config = this->get_config()->subblock( "matlab_config" );

  // Iterate over all values in this config block and pass the values
  // to the matlab as variable assignments.
  auto keys = algo_config->available_values();
  for( auto k : keys )
  {
    std::stringstream config_command;
    config_command <<  k << "=" << algo_config->get_value<std::string>( k ) << ";";
    LOG_DEBUG( logger(), "Sending config value: " << config_command.str() );

    d->eval( config_command.str() );
  }// end foreach

  // Call matlab function to complete the config
  d->eval( "configure_process()" );
}

// ----------------------------------------------------------------
void
matlab_process
::_step()
{
  kwiver::vital::timestamp frame_time;

  // See if optional input port has been connected.
  // Get input only if connected.
  if ( has_input_port_edge_using_trait( timestamp ) )
  {
    frame_time = grab_from_port_using_trait( timestamp );
  }

  kwiver::vital::image_container_sptr img = grab_from_port_using_trait( image );

  kwiver::vital::image_container_sptr out_image;
  {
    scoped_step_instrumentation();

    LOG_DEBUG( logger(), "Processing frame " << frame_time );

    // Convert inputs to matlab format and send to matlab engine
    // Need to establish an interface to the matlab function.
    // Could use parameters or well known names.

    // Sending an image to matlab
    kwiver::arrows::matlab::MxArraySptr mx_image = kwiver::arrows::matlab::convert_mx_image( img );
    d->m_matlab_engine->put_variable( "in_image", mx_image );

    // Call matlab step function
    d->eval( "step( in_image );" );

    kwiver::arrows::matlab::MxArraySptr mx_out_image = d->m_matlab_engine->get_variable( "out_image" );
    out_image = kwiver::arrows::matlab::convert_mx_image( mx_out_image );
  }

  push_to_port_using_trait( image, out_image );
}

// ------------------------------------------------------------------
void
matlab_process
::_init()
{
}

// ----------------------------------------------------------------
void
matlab_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t required;

  sprokit::process::port_flags_t optional;
  required.insert( flag_required );

  sprokit::process::port_flags_t shared;
  shared.insert( flag_output_shared );

  // -- input --
  declare_input_port_using_trait( timestamp, optional );
  declare_input_port_using_trait( image, required );

  // -- output --
  declare_output_port_using_trait( image, shared );
}

// ----------------------------------------------------------------
void
matlab_process
::make_config()
{
  declare_config_using_trait( program_file );
}

// ================================================================
matlab_process::priv
::priv( matlab_process* parent)
  : m_parent( parent )
{
}

matlab_process::priv
::~priv()
{
}

// ------------------------------------------------------------------
void
matlab_process::priv
::check_result()
{
  const std::string& results( m_matlab_engine->output() );
  if ( results.size() > 0 )
  {
    LOG_INFO( m_parent->logger(), "Matlab output: " << results );
  }
}

// ------------------------------------------------------------------
void
matlab_process::priv
::eval( const std::string& expr )
{
  LOG_DEBUG( m_parent->logger(), "Matlab eval: " << expr );
  m_matlab_engine->eval( expr );
  check_result();
}

} } // end namespace
