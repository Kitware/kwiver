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

static std::string geo_origin_file = "pointcloud_data/geo_origin.txt";
static std::string landmarks_file = "pointcloud_data/landmarks.ply";

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
TEST_F(pointcloud_io, save_geo_origin) {
    auto const geo_origin_path = data_dir + "/" + geo_origin_file;
    auto const landmarks_path = data_dir + "/" + landmarks_file;
    auto const tmp_path =
        kwiver::testing::temp_file_name( "test-pdal-output-", ".las" );

    static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
    kv::set_geo_conv(&geo_conv);
    kv::landmark_map_sptr landmark_map =
        kv::read_ply_file(landmarks_path);

    auto lgcs = kwiver::vital::local_geo_cs();
    read_local_geo_cs_from_file(lgcs, geo_origin_path);

    kwiver::arrows::pdal::save_point_cloud_las(tmp_path, lgcs, landmark_map);
}
