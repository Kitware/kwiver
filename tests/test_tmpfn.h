// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 *
 * \brief Supplemental macro definitions for test cases
 */

#ifndef KWIVER_TEST_TEST_TMPFN_H_
#define KWIVER_TEST_TEST_TMPFN_H_

#include <fcntl.h>

#include <stdexcept>
#include <string>

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#define tempnam(d, p) _tempnam(d, p)
#define O_EXCL _O_EXCL
#define O_CREAT _O_CREAT
#define open _open
#define close _close
#define pmode _S_IREAD | _S_IWRITE
#else
#include <unistd.h>
#define pmode 0600
#endif

namespace kwiver {
namespace testing {

// ----------------------------------------------------------------------------
/** @brief Generate a unique file name in the current working directory.
 *
 * @param prefix Prefix for generated file name.
 * @param suffix Suffix for generated file name.
 */
std::string
temp_file_name( char const* prefix, char const* suffix )
{
  std::string name;
  int fd = -1;
  while( fd < 0 )
  {
    auto const base_name = tempnam( ".", prefix );
    name = std::string( base_name ) + suffix;
    free( base_name );
    fd = open( name.c_str(), O_EXCL | O_CREAT, pmode );
  }

  if( close( fd ) )
  {
    throw std::runtime_error( "Could not close temporary file" );
  }

  return name;
}

} // end namespace testing
} // end namespace kwiver

#endif
