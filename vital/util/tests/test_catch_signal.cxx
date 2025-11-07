// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test process signal catching utilities.

#include <vital/util/catch_signal.h>

#include <gtest/gtest.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <signal.h>
#include <unistd.h>
#endif

#include <atomic>

using namespace kwiver::vital;

namespace {

// ----------------------------------------------------------------------------
// On Windows, we can only generate Ctrl-C at the process group (usually
// console / terminal) level, not at the process level. If these tests are
// running as part of another command in the same console (e.g. ctest), that
// process will also receive Ctrl-C, which is undesirable. Therefore we have to
// allocate a separate console for this process before we create any interrupts.
void
initialize()
{
#ifdef _WIN32
  FreeConsole();
  AllocConsole();
#endif
}

// ----------------------------------------------------------------------------
void
raise_sigint()
{
#ifdef _WIN32
  GenerateConsoleCtrlEvent( CTRL_C_EVENT, GetCurrentProcessId() );
#endif

#ifdef __unix__
  kill( getpid(), SIGINT );
#endif
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( catch_signal, uncaught_sigint )
{
  initialize();

  EXPECT_DEATH_IF_SUPPORTED( raise_sigint(), "" );
}

// ----------------------------------------------------------------------------
TEST ( catch_signal, caught_sigint )
{
  initialize();

  // Add one handler
  std::atomic_bool flag1 = false;
  catch_sigint( [ &flag1 ](){ flag1 = true; } );
  raise_sigint();
  EXPECT_TRUE( flag1.load() );

  // Replace it with another handler
  std::atomic_bool flag2 = false;
  flag1 = false;
  catch_sigint( [ &flag2 ](){ flag2 = true; } );
  raise_sigint();
  EXPECT_FALSE( flag1.load() );
  EXPECT_TRUE( flag2.load() );

  // Replace that with no handler
  catch_sigint( nullptr );
  EXPECT_DEATH_IF_SUPPORTED( raise_sigint(), "" );
}
