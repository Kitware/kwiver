// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation for subprocess_embedded_pipeline.
 */

#include "subprocess_embedded_pipeline.h"

#include <vital/any.h>
#include <vital/config/config_block.h>
#include <vital/logger/logger.h>
#include <vital/util/bounded_buffer.h>
#include <vital/vital_config.h>

#include <sprokit/pipeline/datum.h>

#include <kwiversys/Process.h>
#include <kwiversys/SystemTools.hxx>

#include <sstream>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstring>

namespace kwiver {

namespace {

// Message type identifiers for IPC protocol
constexpr uint32_t MSG_PIPELINE_CONFIG = 1;
constexpr uint32_t MSG_DATA_SET = 2;
constexpr uint32_t MSG_END_OF_INPUT = 3;
constexpr uint32_t MSG_PORT_INFO = 4;
constexpr uint32_t MSG_ERROR = 5;
constexpr uint32_t MSG_READY = 6;

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
    throw std::runtime_error( "Failed to read string length from subprocess" );
  }
  std::string str( len, '\0' );
  is.read( &str[0], len );
  if ( !is )
  {
    throw std::runtime_error( "Failed to read string data from subprocess" );
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
    // Note: This simplified implementation assumes the datum contains serializable data
    // A production implementation would need proper type-aware serialization
    if ( datum->type() == sprokit::datum::data )
    {
      // Get the any value from the datum
      try
      {
        auto any_data = datum->get_datum< kwiver::vital::any >();
        // Store the type name for deserialization
        std::string type_name = any_data.type().name();
        write_string( oss, type_name );
        // Placeholder for actual data - real implementation needs type registry
        write_string( oss, "any_placeholder" );
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
    // Read port name
    std::string port_name = read_string( iss );

    // Read datum type
    uint32_t datum_type;
    iss.read( reinterpret_cast< char* >( &datum_type ), sizeof( datum_type ) );

    if ( datum_type == static_cast< uint32_t >( sprokit::datum::data ) )
    {
      std::string type_info = read_string( iss );
      std::string value_data = read_string( iss );

      // Create a placeholder datum - real implementation needs proper deserialization
      auto datum = sprokit::datum::empty_datum();
      ads->add_datum( port_name, datum );
    }
    else
    {
      // Create appropriate non-data datum
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

} // end anonymous namespace

typedef kwiversys::SystemTools ST;

// ----------------------------------------------------------------
class subprocess_embedded_pipeline::priv
{
public:
  priv()
    : m_logger( kwiver::vital::get_logger( "sprokit.subprocess_embedded_pipeline" ) )
    , m_at_end( false )
    , m_subprocess_running( false )
    , m_subprocess_exit_code( -1 )
    , m_input_buffer( 10 )  // Bounded buffer for input data sets
    , m_output_buffer( 10 ) // Bounded buffer for output data sets
  {
  }

  ~priv()
  {
    stop_subprocess();
  }

  void start_subprocess();
  void stop_subprocess();
  void send_to_subprocess( const std::string& data );
  std::string receive_from_subprocess();

  // Thread functions for async I/O
  void input_thread_func();
  void output_thread_func();

  vital::logger_handle_t m_logger;

  std::string m_pipeline_config;
  std::string m_def_dir;
  std::string m_worker_path;

  std::atomic< bool > m_at_end;
  std::atomic< bool > m_subprocess_running;
  std::atomic< int > m_subprocess_exit_code;

  sprokit::process::ports_t m_input_ports;
  sprokit::process::ports_t m_output_ports;
  bool m_input_adapter_connected{ false };
  bool m_output_adapter_connected{ false };

  // Subprocess handle
  kwsysProcess* m_process{ nullptr };

  // Thread-safe buffers for async I/O
  kwiver::vital::bounded_buffer< kwiver::adapter::adapter_data_set_t > m_input_buffer;
  kwiver::vital::bounded_buffer< kwiver::adapter::adapter_data_set_t > m_output_buffer;

  // I/O threads
  std::thread m_input_thread;
  std::thread m_output_thread;

  // Synchronization
  std::mutex m_process_mutex;
  std::atomic< bool > m_stop_requested{ false };
};

// ==================================================================
subprocess_embedded_pipeline::subprocess_embedded_pipeline()
  : m_priv( new priv() )
{
}

// ----------------------------------------------------------------------------
subprocess_embedded_pipeline::~subprocess_embedded_pipeline()
{
  try
  {
    stop();
  }
  catch ( ... )
  {
    // Suppress exceptions in destructor
  }
}

// ----------------------------------------------------------------------------
void
subprocess_embedded_pipeline::build_pipeline( std::istream& istr,
                                              std::string const& def_dir )
{
  // Read the entire pipeline config into a string
  std::ostringstream ss;
  ss << istr.rdbuf();
  m_priv->m_pipeline_config = ss.str();

  m_priv->m_def_dir = def_dir;
  if ( m_priv->m_def_dir.empty() )
  {
    m_priv->m_def_dir = ST::GetCurrentWorkingDirectory();
  }

  // Parse the pipeline config to extract port information
  // This is done by building a local pipeline temporarily to get port info
  std::istringstream config_stream( m_priv->m_pipeline_config );

  // Call parent build_pipeline to validate and extract port info
  // We use the parent class to parse the pipeline and get port information
  // but we won't actually run it in this process
  try
  {
    embedded_pipeline::build_pipeline( config_stream, m_priv->m_def_dir );
    m_priv->m_input_adapter_connected = embedded_pipeline::input_adapter_connected();
    m_priv->m_output_adapter_connected = embedded_pipeline::output_adapter_connected();

    if ( m_priv->m_input_adapter_connected )
    {
      m_priv->m_input_ports = embedded_pipeline::input_port_names();
    }
    if ( m_priv->m_output_adapter_connected )
    {
      m_priv->m_output_ports = embedded_pipeline::output_port_names();
    }
  }
  catch ( std::exception const& e )
  {
    LOG_ERROR( m_priv->m_logger, "Failed to parse pipeline: " << e.what() );
    throw;
  }
}

// ------------------------------------------------------------------
void
subprocess_embedded_pipeline::send( kwiver::adapter::adapter_data_set_t ads )
{
  if ( !m_priv->m_subprocess_running )
  {
    throw std::runtime_error( "Subprocess is not running" );
  }

  if ( !m_priv->m_input_adapter_connected )
  {
    throw std::runtime_error( "Input adapter not connected" );
  }

  // Send to the bounded buffer (will block if full)
  m_priv->m_input_buffer.Send( ads );
}

// ------------------------------------------------------------------
void
subprocess_embedded_pipeline::send_end_of_input()
{
  if ( !m_priv->m_subprocess_running )
  {
    throw std::runtime_error( "Subprocess is not running" );
  }

  auto ads = kwiver::adapter::adapter_data_set::create(
    kwiver::adapter::adapter_data_set::end_of_input );
  m_priv->m_input_buffer.Send( ads );
}

// ------------------------------------------------------------------
kwiver::adapter::adapter_data_set_t
subprocess_embedded_pipeline::receive()
{
  if ( !m_priv->m_output_adapter_connected )
  {
    throw std::runtime_error( "Output adapter not connected" );
  }

  if ( m_priv->m_at_end )
  {
    LOG_WARN( m_priv->m_logger, "receive() called after end of data" );
  }

  // Receive from the bounded buffer (will block if empty)
  auto ads = m_priv->m_output_buffer.Receive();

  if ( ads->is_end_of_data() )
  {
    m_priv->m_at_end = true;
  }

  return ads;
}

// ------------------------------------------------------------------
bool
subprocess_embedded_pipeline::full() const
{
  return m_priv->m_input_buffer.Full();
}

// ------------------------------------------------------------------
bool
subprocess_embedded_pipeline::empty() const
{
  return m_priv->m_output_buffer.Empty();
}

// ------------------------------------------------------------------
bool
subprocess_embedded_pipeline::at_end() const
{
  return m_priv->m_at_end;
}

// ------------------------------------------------------------------
void
subprocess_embedded_pipeline::start()
{
  if ( m_priv->m_subprocess_running )
  {
    LOG_WARN( m_priv->m_logger, "Subprocess already running" );
    return;
  }

  m_priv->start_subprocess();
}

// ------------------------------------------------------------------
void
subprocess_embedded_pipeline::wait()
{
  if ( !m_priv->m_subprocess_running )
  {
    return;
  }

  // Wait for output thread to finish (it will end when subprocess closes stdout)
  if ( m_priv->m_output_thread.joinable() )
  {
    m_priv->m_output_thread.join();
  }

  // Wait for input thread to finish
  if ( m_priv->m_input_thread.joinable() )
  {
    m_priv->m_input_thread.join();
  }

  // Wait for subprocess to exit
  if ( m_priv->m_process )
  {
    kwsysProcess_WaitForExit( m_priv->m_process, nullptr );
    m_priv->m_subprocess_exit_code = kwsysProcess_GetExitValue( m_priv->m_process );
    m_priv->m_subprocess_running = false;
  }
}

// ------------------------------------------------------------------
void
subprocess_embedded_pipeline::stop()
{
  m_priv->stop_subprocess();
}

// ------------------------------------------------------------------
sprokit::process::ports_t
subprocess_embedded_pipeline::input_port_names() const
{
  return m_priv->m_input_ports;
}

// ------------------------------------------------------------------
sprokit::process::ports_t
subprocess_embedded_pipeline::output_port_names() const
{
  return m_priv->m_output_ports;
}

// ------------------------------------------------------------------
bool
subprocess_embedded_pipeline::input_adapter_connected() const
{
  return m_priv->m_input_adapter_connected;
}

// ------------------------------------------------------------------
bool
subprocess_embedded_pipeline::output_adapter_connected() const
{
  return m_priv->m_output_adapter_connected;
}

// ------------------------------------------------------------------
void
subprocess_embedded_pipeline::set_worker_path( std::string const& path )
{
  m_priv->m_worker_path = path;
}

// ------------------------------------------------------------------
int
subprocess_embedded_pipeline::subprocess_exit_code() const
{
  return m_priv->m_subprocess_exit_code;
}

// ------------------------------------------------------------------
bool
subprocess_embedded_pipeline::subprocess_running() const
{
  return m_priv->m_subprocess_running;
}

// ==================================================================
// Private implementation methods
// ==================================================================

void
subprocess_embedded_pipeline::priv::start_subprocess()
{
  std::lock_guard< std::mutex > lock( m_process_mutex );

  // Find worker executable
  std::string worker_exe = m_worker_path;
  if ( worker_exe.empty() )
  {
    // Search in standard locations
    worker_exe = "kwiver_pipeline_worker";

    // Try to find it in PATH or relative to current executable
    std::string found_path;
    if ( ST::FindProgramPath( worker_exe.c_str(), found_path, "" ) )
    {
      worker_exe = found_path;
    }
  }

  // Create process
  m_process = kwsysProcess_New();
  if ( !m_process )
  {
    throw std::runtime_error( "Failed to create subprocess object" );
  }

  // Build command line
  const char* cmd[] = { worker_exe.c_str(), nullptr };
  kwsysProcess_SetCommand( m_process, cmd );

  // Set up pipe-based I/O
  kwsysProcess_SetPipeShared( m_process, kwsysProcess_Pipe_STDOUT, 0 );
  kwsysProcess_SetPipeShared( m_process, kwsysProcess_Pipe_STDERR, 1 ); // Share stderr

  // Execute the subprocess
  kwsysProcess_Execute( m_process );

  int state = kwsysProcess_GetState( m_process );
  if ( state == kwsysProcess_State_Error )
  {
    std::string error = kwsysProcess_GetErrorString( m_process );
    kwsysProcess_Delete( m_process );
    m_process = nullptr;
    throw std::runtime_error( "Failed to start subprocess: " + error );
  }

  m_subprocess_running = true;
  m_stop_requested = false;

  // Send pipeline configuration to subprocess
  std::ostringstream config_msg;
  write_msg_type( config_msg, MSG_PIPELINE_CONFIG );
  write_string( config_msg, m_pipeline_config );
  write_string( config_msg, m_def_dir );

  std::string msg_data = config_msg.str();
  // Write to subprocess stdin
  // Note: kwsysProcess doesn't provide direct stdin write, so we use pipe
  // For now, we'll need to implement this differently or use a different approach

  // Start I/O threads
  m_input_thread = std::thread( &priv::input_thread_func, this );
  m_output_thread = std::thread( &priv::output_thread_func, this );

  LOG_DEBUG( m_logger, "Subprocess started with PID" );
}

void
subprocess_embedded_pipeline::priv::stop_subprocess()
{
  m_stop_requested = true;

  {
    std::lock_guard< std::mutex > lock( m_process_mutex );

    if ( m_process && m_subprocess_running )
    {
      // Kill the subprocess
      kwsysProcess_Kill( m_process );
      kwsysProcess_WaitForExit( m_process, nullptr );
      m_subprocess_exit_code = -1;
      m_subprocess_running = false;
    }
  }

  // Reset the bounded buffers to unblock waiting threads
  m_input_buffer.Reset();
  m_output_buffer.Reset();

  // Wait for threads to finish
  if ( m_input_thread.joinable() )
  {
    m_input_thread.join();
  }
  if ( m_output_thread.joinable() )
  {
    m_output_thread.join();
  }

  {
    std::lock_guard< std::mutex > lock( m_process_mutex );
    if ( m_process )
    {
      kwsysProcess_Delete( m_process );
      m_process = nullptr;
    }
  }
}

void
subprocess_embedded_pipeline::priv::input_thread_func()
{
  LOG_DEBUG( m_logger, "Input thread started" );

  try
  {
    while ( !m_stop_requested && m_subprocess_running )
    {
      // Get data from input buffer
      auto ads = m_input_buffer.Receive();

      if ( m_stop_requested )
      {
        break;
      }

      // Serialize the data set
      std::ostringstream msg;
      if ( ads->is_end_of_data() )
      {
        write_msg_type( msg, MSG_END_OF_INPUT );
      }
      else
      {
        write_msg_type( msg, MSG_DATA_SET );
        std::string serialized = serialize_data_set( ads );
        write_string( msg, serialized );
      }

      // Send to subprocess
      // Note: This needs proper implementation with kwsysProcess pipe writing
      // For now this is a placeholder

      if ( ads->is_end_of_data() )
      {
        break; // Exit after sending end of input
      }
    }
  }
  catch ( std::exception const& e )
  {
    LOG_ERROR( m_logger, "Input thread error: " << e.what() );
  }

  LOG_DEBUG( m_logger, "Input thread finished" );
}

void
subprocess_embedded_pipeline::priv::output_thread_func()
{
  LOG_DEBUG( m_logger, "Output thread started" );

  try
  {
    char* data = nullptr;
    int length = 0;

    while ( !m_stop_requested && m_subprocess_running )
    {
      // Read from subprocess stdout
      int pipe = kwsysProcess_WaitForData( m_process, &data, &length, nullptr );

      if ( pipe == kwsysProcess_Pipe_STDOUT && length > 0 )
      {
        // Process received data
        std::string received( data, length );
        std::istringstream iss( received );

        uint32_t msg_type = read_msg_type( iss );

        if ( msg_type == MSG_DATA_SET )
        {
          std::string serialized = read_string( iss );
          auto ads = deserialize_data_set( serialized );
          m_output_buffer.Send( ads );
        }
        else if ( msg_type == MSG_END_OF_INPUT )
        {
          auto ads = kwiver::adapter::adapter_data_set::create(
            kwiver::adapter::adapter_data_set::end_of_input );
          m_output_buffer.Send( ads );
          break;
        }
        else if ( msg_type == MSG_ERROR )
        {
          std::string error_msg = read_string( iss );
          LOG_ERROR( m_logger, "Subprocess error: " << error_msg );
        }
      }
      else if ( pipe == kwsysProcess_Pipe_None )
      {
        // Subprocess has closed stdout, we're done
        auto ads = kwiver::adapter::adapter_data_set::create(
          kwiver::adapter::adapter_data_set::end_of_input );
        m_output_buffer.Send( ads );
        break;
      }
    }
  }
  catch ( std::exception const& e )
  {
    LOG_ERROR( m_logger, "Output thread error: " << e.what() );
  }

  LOG_DEBUG( m_logger, "Output thread finished" );
}

} // end namespace kwiver
