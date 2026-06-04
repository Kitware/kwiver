// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/kpf/algo/detected_object_set_input_kpf.h>
#include <arrows/kpf/algo/detected_object_set_output_kpf.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vital/algo/algorithm.txx>

#include <gtest/gtest.h>

#include <algorithm>

using namespace kwiver::vital;
using namespace kwiver::arrows::kpf;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( detected_object_set_input_kpf, create )
{
  using namespace kwiver::vital;

  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm<
      algo::detected_object_set_input >( "kpf_input" ) );
}

// ----------------------------------------------------------------------------
TEST ( detected_object_set_output_kpf, create )
{
  using namespace kwiver::vital;

  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm<
      algo::detected_object_set_output >( "kpf_output" ) );
}
