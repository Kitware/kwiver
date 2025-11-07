// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities for catching process-level signals from the OS.

#include <vital/util/catch_signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <signal.h>
#endif

#include <mutex>

namespace kwiver {

namespace vital {

namespace {

std::mutex _fn_mutex;
std::function< void() > _fn;

#ifdef __unix__
// ----------------------------------------------------------------------------
void
signal_handler( int )
{
  std::unique_lock lock( _fn_mutex );
  auto fn = _fn;
  lock.unlock();
  fn();
}
#endif

#ifdef _WIN32
// ----------------------------------------------------------------------------
BOOL WINAPI
signal_handler( DWORD sig )
{
  if( sig == CTRL_C_EVENT )
  {
    std::unique_lock lock( _fn_mutex );
    auto fn = _fn;
    lock.unlock();
    fn();
    return TRUE;
  }

  return FALSE;
}
#endif

} // namespace <anonymous>

// ----------------------------------------------------------------------------
void
catch_sigint( std::function< void() > fn )
{
  std::lock_guard lock( _fn_mutex );
  _fn = std::move( fn );

#ifdef _WIN32
  if( _fn )
  {
    SetConsoleCtrlHandler( &signal_handler, TRUE );
  }
  else
  {
    SetConsoleCtrlHandler( &signal_handler, FALSE );
  }
#endif

#ifdef __unix__
  if( _fn )
  {
    struct sigaction handler = {};
    handler.sa_handler = signal_handler;
    sigaction( SIGINT, &handler, nullptr );
  }
  else
  {
    signal( SIGINT, SIG_DFL );
  }
#endif
}

} // namespace vital

} // namespace kwiver
