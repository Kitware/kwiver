// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test PDAL pointcloud output

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/pdal/pointcloud_io.h>
#include <arrows/proj/geo_conv.h>
#include <vital/io/landmark_map_io.h>

namespace kv = kwiver::vital;

kv::path_t g_data_dir;

// ----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class pointcloud_io : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(pointcloud_io, create) {
  auto pcio_ptr = new kwiver::arrows::pdal::pointcloud_io();
  ASSERT_NE(pcio_ptr, nullptr);
}

TEST_F(pointcloud_io, save_geo_origin) {
    auto const geo_origin_path = data_dir + "/geo_origin.txt";
    auto const landmarks_path = data_dir + "/landmarks.ply";
    auto const tmp_path =
        kwiver::testing::temp_file_name( "test-pdal-output-", ".las" );

    static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
    kv::set_geo_conv(&geo_conv);
    kv::landmark_map_sptr landmark_map =
        kv::read_ply_file(landmarks_path);

    auto io = new kwiver::arrows::pdal::pointcloud_io();
    io->save_(tmp_path, geo_origin_path, landmark_map);
}
