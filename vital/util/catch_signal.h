// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities for catching process-level signals from the OS.

#ifndef KWIVER_VITAL_UTIL_CATCH_SIGNAL_H_
#define KWIVER_VITAL_UTIL_CATCH_SIGNAL_H_

#include <vital/util/vital_util_export.h>

#include <functional>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Runs \p fn when a keyboard interrupt (Ctrl+C) has been detected.
///
/// The function \p fn replaces any previous \p fn passed to this function;
/// this is a process-wide setting. This function is thread-safe.
///
/// \param fn
///   Function to run when the process receives the keyboard interrupt signal.
///   If \c nullptr, the system default action will be performed instead.
VITAL_UTIL_EXPORT void catch_sigint( std::function< void() > fn );

} // namespace vital

} // namespace kwiver

#endif
