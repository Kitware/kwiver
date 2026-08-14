// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of Ceres logging utilities.

#include <arrows/ceres/logging.h>

#include <vital/logger/logger.h>

#include <ceres/ceres.h>

#include <atomic>

namespace kwiver {

namespace arrows {

namespace ceres {

namespace {

// ----------------------------------------------------------------------------
class log_sink : public ::google::LogSink
{
public:
  log_sink() : logger{ vital::get_logger( "arrows.ceres.internal" ) }
  {}

  void
  send(
    ::google::LogSeverity severity,
    [[maybe_unused]] char const* full_filename,
    [[maybe_unused]] char const* base_filename,
    [[maybe_unused]] int line,
    [[maybe_unused]] tm const* tm_time,
    char const* message,
    size_t message_len ) override
  {
    switch( severity )
    {
      case ::google::GLOG_FATAL:
      case ::google::GLOG_ERROR:
        LOG_ERROR( logger, std::string( message, message_len ) );
        break;
      case ::google::GLOG_WARNING:
        LOG_WARN( logger, std::string( message, message_len ) );
        break;
      case ::google::GLOG_INFO:
        LOG_INFO( logger, std::string( message, message_len ) );
        break;
      default:
        break;
    }
  }

  vital::logger_handle_t logger;
};

} // namespace <anonymous>

// ----------------------------------------------------------------------------
void
init_logging()
{
  static std::atomic_bool has_initialized = false;

  if( has_initialized.exchange( true ) )
  {
    return;
  }

  ::google::InitGoogleLogging( "kwiver" );
  FLAGS_v = 1;

  static log_sink sink;
  ::google::AddLogSink( &sink );
}

} // namespace ceres

} // namespace arrows

} // namespace kwiver
