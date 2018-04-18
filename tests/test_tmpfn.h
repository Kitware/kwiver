/*ckwg +29
 * Copyright 2017-2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * \brief Supplemental macro definitions for test cases
 */

#ifndef KWIVER_TEST_TEST_TMPFN_H_
#define KWIVER_TEST_TEST_TMPFN_H_

#include <kwiversys/SystemTools.hxx>
#include <kwiversys/SystemInformation.hxx>

#include <string>
#include <sstream>
#include <cstdlib>
#include <chrono>

namespace kwiver {
namespace testing {

using ST = kwiversys::SystemTools;
using namespace std::chrono;

// ----------------------------------------------------------------------------
/** @brief Generate a unique file name in the current working directory.
 *
 * @param prefix Prefix for generated file name.
 * @param suffix Suffix for generated file name.
 */
std::string
temp_file_name( char const* prefix, char const* suffix )
{
  std::string tfn;
  kwiversys::SystemInformation sysinfo;

  // This is a fair amount of work to get a unique seed for the RNG.
  // Practically speaking, this may not be necessary since the RNG is
  // used repeatedly if the proposed file already exists. So it will
  // also work if the default random sequence is used and the
  // progression of file name is always the same (as long as there is
  // not a race condition).
  auto ms = std::chrono::high_resolution_clock::now();
  auto value = ms.time_since_epoch();
  unsigned int seed = value.count() * (10000 + sysinfo.GetProcessId() );

  srand( seed );

  do
  {
    std::stringstream sstr;
    sstr << prefix << rand() << suffix;
    tfn = sstr.str();
  } while ( ST::FileExists( tfn, false ) );

  return tfn;
}

} // end namespace testing
} // end namespace kwiver

#endif
