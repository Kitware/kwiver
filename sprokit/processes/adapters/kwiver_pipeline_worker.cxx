// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Pipeline worker executable for subprocess embedded pipelines.
 *
 * This executable is spawned by subprocess_embedded_pipeline to run
 * the actual pipeline in a separate process. It communicates with the
 * parent process via stdin/stdout pipes using a binary protocol.
 */

#include "embedded_pipeline.h"
#include "adapter_data_set.h"

#include <vital/any.h>
#include <vital/config/config_block.h>
#include <vital/logger/logger.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <sprokit/pipeline/datum.h>

#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <cstdint>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace {

// Message type identifiers - must match subprocess_embedded_pipeline.cxx
constexpr uint32_t MSG_PIPELINE_CONFIG = 1;
constexpr uint32_t MSG_DATA_SET = 2;
constexpr uint32_t MSG_END_OF_INPUT = 3;
constexpr uint32_t MSG_PORT_INFO = 4;
constexpr uint32_t MSG_ERROR = 5;
constexpr uint32_t MSG_READY = 6;

kwiver::vital::logger_handle_t g_logger;

// Serialize a string with length prefix
void write_string( std::ostream& os, const std::string& str )
{
  uint32_t len = static_cast< uint32_t >( str.size() );
  os.write( reinterpret_cast< const char* >( &len ), sizeof( len ) );
  os.write( str.data(), len );
}

// Read a length-prefixed string
std::string read_string( std::istream& is )
{
  uint32_t len;
  is.read( reinterpret_cast< char* >( &len ), sizeof( len ) );
  if ( !is )
  {
    throw std::runtime_error( "Failed to read string length" );
  }
  std::string str( len, '\0' );
  is.read( &str[0], len );
  if ( !is )
  {
    throw std::runtime_error( "Failed to read string data" );
  }
  return str;
}

// Write a message type identifier
void write_msg_type( std::ostream& os, uint32_t type )
{
  os.write( reinterpret_cast< const char* >( &type ), sizeof( type ) );
}

// Read a message type identifier
uint32_t read_msg_type( std::istream& is )
{
  uint32_t type;
  is.read( reinterpret_cast< char* >( &type ), sizeof( type ) );
  if ( !is )
  {
    return 0; // EOF or error
  }
  return type;
}

// Serialize an adapter_data_set to a string
std::string serialize_data_set( kwiver::adapter::adapter_data_set_t ads )
{
  std::ostringstream oss;

  // Write data set type
  uint32_t type = static_cast< uint32_t >( ads->type() );
  oss.write( reinterpret_cast< const char* >( &type ), sizeof( type ) );

  // Write number of elements
  uint32_t count = static_cast< uint32_t >( ads->size() );
  oss.write( reinterpret_cast< const char* >( &count ), sizeof( count ) );

  // Write each element
  for ( auto const& elem : *ads )
  {
    // Write port name
    write_string( oss, elem.first );

    // Write datum type
    auto datum = elem.second;
    uint32_t datum_type = static_cast< uint32_t >( datum->type() );
    oss.write( reinterpret_cast< const char* >( &datum_type ), sizeof( datum_type ) );

    // For data datums, serialize the actual data
    if ( datum->type() == sprokit::datum::data )
    {
      try
      {
        auto any_data = datum->get_datum< kwiver::vital::any >();
        std::string type_name = any_data.type().name();
        write_string( oss, type_name );
        // Placeholder for actual data - real implementation needs type registry
        write_string( oss, "" );
      }
      catch ( ... )
      {
        write_string( oss, "" );
        write_string( oss, "" );
      }
    }
  }

  return oss.str();
}

// Deserialize a string to an adapter_data_set
kwiver::adapter::adapter_data_set_t deserialize_data_set( const std::string& data )
{
  std::istringstream iss( data );

  // Read data set type
  uint32_t type;
  iss.read( reinterpret_cast< char* >( &type ), sizeof( type ) );

  auto ads = kwiver::adapter::adapter_data_set::create(
    static_cast< kwiver::adapter::adapter_data_set::data_set_type >( type ) );

  // Read number of elements
  uint32_t count;
  iss.read( reinterpret_cast< char* >( &count ), sizeof( count ) );

  // Read each element
  for ( uint32_t i = 0; i < count; ++i )
  {
    std::string port_name = read_string( iss );

    uint32_t datum_type;
    iss.read( reinterpret_cast< char* >( &datum_type ), sizeof( datum_type ) );

    if ( datum_type == static_cast< uint32_t >( sprokit::datum::data ) )
    {
      std::string type_info = read_string( iss );
      std::string value_data = read_string( iss );

      auto datum = sprokit::datum::empty_datum();
      ads->add_datum( port_name, datum );
    }
    else
    {
      sprokit::datum_t datum;
      switch ( static_cast< sprokit::datum::type >( datum_type ) )
      {
        case sprokit::datum::empty:
          datum = sprokit::datum::empty_datum();
          break;
        case sprokit::datum::flush:
          datum = sprokit::datum::flush_datum();
          break;
        case sprokit::datum::complete:
          datum = sprokit::datum::complete_datum();
          break;
        case sprokit::datum::error:
          datum = sprokit::datum::error_datum( "deserialized error" );
          break;
        default:
          datum = sprokit::datum::empty_datum();
          break;
      }
      ads->add_datum( port_name, datum );
    }
  }

  return ads;
}

void send_error( const std::string& msg )
{
  write_msg_type( std::cout, MSG_ERROR );
  write_string( std::cout, msg );
  std::cout.flush();
}

void send_ready()
{
  write_msg_type( std::cout, MSG_READY );
  std::cout.flush();
}

void send_port_info( const sprokit::process::ports_t& input_ports,
                     const sprokit::process::ports_t& output_ports )
{
  write_msg_type( std::cout, MSG_PORT_INFO );

  // Write input ports
  uint32_t count = static_cast< uint32_t >( input_ports.size() );
  std::cout.write( reinterpret_cast< const char* >( &count ), sizeof( count ) );
  for ( const auto& port : input_ports )
  {
    write_string( std::cout, port );
  }

  // Write output ports
  count = static_cast< uint32_t >( output_ports.size() );
  std::cout.write( reinterpret_cast< const char* >( &count ), sizeof( count ) );
  for ( const auto& port : output_ports )
  {
    write_string( std::cout, port );
  }

  std::cout.flush();
}

void send_data_set( kwiver::adapter::adapter_data_set_t ads )
{
  if ( ads->is_end_of_data() )
  {
    write_msg_type( std::cout, MSG_END_OF_INPUT );
  }
  else
  {
    write_msg_type( std::cout, MSG_DATA_SET );
    std::string serialized = serialize_data_set( ads );
    write_string( std::cout, serialized );
  }
  std::cout.flush();
}

} // anonymous namespace

int main( int argc, char* argv[] )
{
  // Set binary mode for stdin/stdout on Windows
#ifdef _WIN32
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );
#endif

  g_logger = kwiver::vital::get_logger( "kwiver_pipeline_worker" );
  LOG_DEBUG( g_logger, "Pipeline worker starting" );

  try
  {
    // Load all plugins
    kwiver::vital::plugin_manager::instance().load_all_plugins();

    // Read pipeline configuration from stdin
    uint32_t msg_type = read_msg_type( std::cin );
    if ( msg_type != MSG_PIPELINE_CONFIG )
    {
      send_error( "Expected pipeline configuration message" );
      return 1;
    }

    std::string pipeline_config = read_string( std::cin );
    std::string def_dir = read_string( std::cin );

    LOG_DEBUG( g_logger, "Received pipeline config, def_dir=" << def_dir );

    // Build and start the pipeline
    kwiver::embedded_pipeline pipeline;

    std::istringstream config_stream( pipeline_config );
    pipeline.build_pipeline( config_stream, def_dir );

    // Send port information back to parent
    sprokit::process::ports_t input_ports;
    sprokit::process::ports_t output_ports;

    if ( pipeline.input_adapter_connected() )
    {
      input_ports = pipeline.input_port_names();
    }
    if ( pipeline.output_adapter_connected() )
    {
      output_ports = pipeline.output_port_names();
    }

    send_port_info( input_ports, output_ports );

    // Start the pipeline
    pipeline.start();

    LOG_DEBUG( g_logger, "Pipeline started" );

    // Send ready signal
    send_ready();

    // Main loop: read data from stdin, send to pipeline, receive output, send to stdout
    std::atomic< bool > input_done{ false };
    std::atomic< bool > output_done{ false };

    // Thread to receive pipeline output and send to stdout
    std::thread output_thread( [&]() {
      try
      {
        while ( !output_done && pipeline.output_adapter_connected() )
        {
          auto ads = pipeline.receive();
          send_data_set( ads );

          if ( ads->is_end_of_data() )
          {
            output_done = true;
            break;
          }
        }
      }
      catch ( std::exception const& e )
      {
        LOG_ERROR( g_logger, "Output thread error: " << e.what() );
        send_error( e.what() );
      }
    } );

    // Main thread: read from stdin and send to pipeline
    try
    {
      while ( !input_done )
      {
        msg_type = read_msg_type( std::cin );

        if ( msg_type == 0 )
        {
          // EOF on stdin
          LOG_DEBUG( g_logger, "EOF on stdin" );
          if ( pipeline.input_adapter_connected() )
          {
            pipeline.send_end_of_input();
          }
          input_done = true;
          break;
        }
        else if ( msg_type == MSG_DATA_SET )
        {
          std::string serialized = read_string( std::cin );
          auto ads = deserialize_data_set( serialized );

          if ( pipeline.input_adapter_connected() )
          {
            pipeline.send( ads );
          }
        }
        else if ( msg_type == MSG_END_OF_INPUT )
        {
          LOG_DEBUG( g_logger, "Received end of input" );
          if ( pipeline.input_adapter_connected() )
          {
            pipeline.send_end_of_input();
          }
          input_done = true;
          break;
        }
        else
        {
          LOG_WARN( g_logger, "Unknown message type: " << msg_type );
        }
      }
    }
    catch ( std::exception const& e )
    {
      LOG_ERROR( g_logger, "Input processing error: " << e.what() );
      send_error( e.what() );
    }

    // Wait for output thread to finish
    if ( output_thread.joinable() )
    {
      output_thread.join();
    }

    // Wait for pipeline to complete
    pipeline.wait();

    LOG_DEBUG( g_logger, "Pipeline worker finished successfully" );
    return 0;
  }
  catch ( std::exception const& e )
  {
    LOG_ERROR( g_logger, "Fatal error: " << e.what() );
    send_error( e.what() );
    return 1;
  }
  catch ( ... )
  {
    LOG_ERROR( g_logger, "Unknown fatal error" );
    send_error( "Unknown error" );
    return 1;
  }
}
