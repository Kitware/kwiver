/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 * \brief Test jenkins
 */

#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <sstream>

#include <gtest/gtest.h>
#include <test_gtest.h>


#include <track_oracle/core/track_oracle_core.h>
#include <track_oracle/core/track_base.h>
#include <track_oracle/file_formats/file_format_manager.h>

namespace to = ::kwiver::track_oracle;

using std::string;
using std::vector;
using std::thread;
using std::ostringstream;

namespace { //anon

string g_data_dir;

}; // ...anon



// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );

#if GTEST_IS_THREADSAFE
  GET_ARG(1, g_data_dir);
  // ... do stuff
#endif
  return RUN_ALL_TESTS();
}

TEST( track_oracle, gtest_threadsafe_2 )
{
#if GTEST_IS_THREADSAFE
  EXPECT_TRUE( true ) << "GTest is threadsafe";
  std::cerr << "GTest is threadsafe\n";
#else
  EXPECT_TRUE( false ) << "GTest is not threadsafe";
  std::cerr << "GTest is NOT threadsafe\n";
#endif
}

TEST( track_oracle, travis )
{

  string track_file = g_data_dir+"/generic_tracks.kw18";
  to::track_handle_list_type tracks;
  bool rc = to::file_format_manager::read( track_file, tracks );
  EXPECT_TRUE( rc ) << " reading from '" << track_file << "'";
}
