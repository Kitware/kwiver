// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "vital/logger/kwiver_logger_factory.h"
#include <log4cxx/logger.h>
#include <memory>

#include <vital/logger_plugins/vital_log4cxx_logger_export.h>

namespace kwiver {

namespace vital {

namespace logger_ns {

// ----------------------------------------------------------------

/** @class log4cxx_logger
 * @brief Log4cxx logger implementation.
 *
 * This class is an adapter that bridges from our standard logging API
 * to the log4cxx API.
 */
class log4cxx_logger
  : public kwiver_logger
{
public:
  /**
   * @brief Create a new log4cxx logger object
   *
   * @param fact Pointer to logger factory class.
   * @param name Name of logging category.
   */
  log4cxx_logger( kwiver_logger_factory* fact, std::string const& name )
    : kwiver_logger( fact, name ),
      m_loggerImpl( ::log4cxx::Logger::getLogger( name ) )
  {}

  // Check to see if level is enabled
  virtual bool
  is_fatal_enabled() const { return this->m_loggerImpl->isFatalEnabled(); }
  virtual bool
  is_error_enabled() const { return this->m_loggerImpl->isErrorEnabled(); }
  virtual bool
  is_warn_enabled() const { return this->m_loggerImpl->isWarnEnabled(); }
  virtual bool
  is_info_enabled() const { return this->m_loggerImpl->isInfoEnabled(); }
  virtual bool
  is_debug_enabled() const { return this->m_loggerImpl->isDebugEnabled(); }
  virtual bool
  is_trace_enabled() const { return this->m_loggerImpl->isTraceEnabled(); }

  // ------------------------------------------------------------------
  virtual void
  set_level( log_level_t level )
  {
    log4cxx::LevelPtr lvl;

    switch( level )
    {
      case LEVEL_TRACE:
        lvl = ::log4cxx::Level::getTrace();
        break;

      case LEVEL_DEBUG:
        lvl = ::log4cxx::Level::getDebug();
        break;

      case LEVEL_INFO:
        lvl = ::log4cxx::Level::getInfo();
        break;

      case LEVEL_WARN:
        lvl = ::log4cxx::Level::getWarn();
        break;

      case LEVEL_ERROR:
        lvl = ::log4cxx::Level::getError();
        break;

      case LEVEL_FATAL:
        lvl = ::log4cxx::Level::getFatal();
        break;

      default:
        // log or throw
        break;
    } // end switch

    this->m_loggerImpl->setLevel( lvl );
  }

  // ------------------------------------------------------------------
  virtual log_level_t
  get_level() const
  {
    log4cxx::LevelPtr lvl = this->m_loggerImpl->getLevel();

    if( lvl == ::log4cxx::Level::getTrace() ) { return LEVEL_TRACE; }
    if( lvl == ::log4cxx::Level::getDebug() ) { return LEVEL_DEBUG; }
    if( lvl == ::log4cxx::Level::getInfo() ) { return LEVEL_INFO; }
    if( lvl == ::log4cxx::Level::getWarn() ) { return LEVEL_WARN; }
    if( lvl == ::log4cxx::Level::getError() ) { return LEVEL_ERROR; }
    if( lvl == ::log4cxx::Level::getFatal() ) { return LEVEL_FATAL; }
    return LEVEL_NONE;
  }

  // ------------------------------------------------------------------
  virtual void
  log_fatal( std::string const& msg )
  {
    this->m_loggerImpl->fatal( msg );
    do_callback( LEVEL_FATAL, msg, location_info() );
  }

  virtual void
  log_fatal( std::string const&                      msg,
             kwiver::vital::logger_ns::location_info const& location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    this->m_loggerImpl->fatal( msg, cxx_location );
    do_callback( LEVEL_FATAL, msg, location );
  }

  // ------------------------------------------------------------------
  virtual void
  log_error( std::string const& msg )
  {
    this->m_loggerImpl->error( msg );
    do_callback( LEVEL_ERROR, msg, location_info() );
  }

  virtual void
  log_error( std::string const&                      msg,
             kwiver::vital::logger_ns::location_info const& location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    this->m_loggerImpl->error( msg, cxx_location );
    do_callback( LEVEL_ERROR, msg, location );
  }

  // ------------------------------------------------------------------
  virtual void
  log_warn( std::string const& msg )
  {
    this->m_loggerImpl->warn( msg );
    do_callback( LEVEL_WARN, msg, location_info() );
  }

  virtual void
  log_warn( std::string const&                       msg,
            kwiver::vital::logger_ns::location_info const&  location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    this->m_loggerImpl->warn( msg, cxx_location );
    do_callback( LEVEL_WARN, msg, location );
  }

  // ------------------------------------------------------------------
  virtual void
  log_info( std::string const& msg )
  {
    this->m_loggerImpl->info( msg );
    do_callback( LEVEL_INFO, msg, location_info() );
  }

  virtual void
  log_info( std::string const&                       msg,
            kwiver::vital::logger_ns::location_info const&  location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    this->m_loggerImpl->info( msg, cxx_location );
    do_callback( LEVEL_INFO, msg, location );
  }

  // ------------------------------------------------------------------
  virtual void
  log_debug( std::string const& msg )
  {
    this->m_loggerImpl->debug( msg );
    do_callback( LEVEL_DEBUG, msg, location_info() );
  }

  virtual void
  log_debug( std::string const&                      msg,
             kwiver::vital::logger_ns::location_info const& location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    this->m_loggerImpl->debug( msg, cxx_location );
    do_callback( LEVEL_DEBUG, msg, location );
  }

  // ------------------------------------------------------------------
  virtual void
  log_trace( std::string const& msg )
  {
    this->m_loggerImpl->trace( msg );
    do_callback( LEVEL_TRACE, msg, location_info() );
  }

  virtual void
  log_trace( std::string const&                      msg,
             kwiver::vital::logger_ns::location_info const& location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    this->m_loggerImpl->trace( msg, cxx_location );
    do_callback( LEVEL_TRACE, msg, location );
  }

  // ------------------------------------------------------------------
  virtual void
  log_message( log_level_t level, std::string const& msg )
  {
    log4cxx::LevelPtr lvl;

    switch( level )
    {
      case LEVEL_TRACE: lvl = ::log4cxx::Level::getTrace();
        break;
      case LEVEL_DEBUG: lvl = ::log4cxx::Level::getDebug();
        break;
      case LEVEL_INFO: lvl = ::log4cxx::Level::getInfo();
        break;
      case LEVEL_WARN: lvl = ::log4cxx::Level::getWarn();
        break;
      case LEVEL_ERROR: lvl = ::log4cxx::Level::getError();
        break;
      case LEVEL_FATAL: lvl = ::log4cxx::Level::getFatal();
        break;
      default:
        break;
    } // end switch

    this->m_loggerImpl->log( lvl, msg );
  }

  virtual void
  log_message( log_level_t level, std::string const& msg,
               kwiver::vital::logger_ns::location_info const& location )
  {
    log4cxx::spi::LocationInfo cxx_location( location.get_file_name_ptr(),
                                             location.get_method_name_ptr(),
                                             location.get_line_number() );

    log4cxx::LevelPtr lvl;

    switch( level )
    {
      case LEVEL_TRACE: lvl = ::log4cxx::Level::getTrace();
        break;
      case LEVEL_DEBUG: lvl = ::log4cxx::Level::getDebug();
        break;
      case LEVEL_INFO: lvl = ::log4cxx::Level::getInfo();
        break;
      case LEVEL_WARN: lvl = ::log4cxx::Level::getWarn();
        break;
      case LEVEL_ERROR: lvl = ::log4cxx::Level::getError();
        break;
      case LEVEL_FATAL: lvl = ::log4cxx::Level::getFatal();
        break;
      default:
        break;
    } // end switch

    this->m_loggerImpl->log( lvl, msg, cxx_location );
  }

  // -- extended interface --
  log4cxx::LoggerPtr
  get_logger_impl()
  {
    return m_loggerImpl;
  }

protected:
  log4cxx::LoggerPtr m_loggerImpl;
}; // end class

// ==================================================================

/** Factory for underlying log4cxx logger.
 *
 * This class represents the factory for the log4cxx logging service.
 * A logger object is created or reused for the specified name.
 */
class log4cxx_factory
  : public kwiver_logger_factory
{
public:
  log4cxx_factory()
    : kwiver_logger_factory( "log4cxx factory" )
  {
    // There may actually be some initialization to be done.
    // Currently following the default init process.
  }

  virtual ~log4cxx_factory() = default;

  virtual logger_handle_t
  get_logger( std::string const& name )
  {
    return logger_handle_t( new log4cxx_logger( this, name ) );
  }
}; // end class log4cxx_factory

} // namespace logger_ns

} // namespace vital

} // namespace kwiver

// ==================================================================

/*
 * Shared object bootstrap function
 */
extern "C" VITAL_LOG4CXX_LOGGER_EXPORT void* kwiver_logger_factory();

void*
kwiver_logger_factory()
{
  kwiver::vital::logger_ns::log4cxx_factory* ptr =
    new kwiver::vital::logger_ns::log4cxx_factory();
  return ptr;
}
