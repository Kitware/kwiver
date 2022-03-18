// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test PDAL pointcloud output

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/pdal/algo/pointcloud_io.h>
#include <arrows/proj/geo_conv.h>
#include <vital/io/landmark_map_io.h>
#include <vital/plugin_loader/plugin_manager.h>

#include <fstream>

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
TEST_F(pointcloud_io, create)
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(nullptr, kv::algo::pointcloud_io::create("pdal"));
}

TEST_F(pointcloud_io, save)
{
  auto const geo_origin_path = data_dir + "/geo_origin.txt";
  auto const landmarks_path = data_dir + "/landmarks.ply";
  auto const tmp_path = data_dir + "/pointcloud.las";
  std::ofstream ofs(tmp_path);
  ofs.close();

  // This will delete the temporary file even if an exception is thrown
  struct _tmp_file_deleter
  {
    ~_tmp_file_deleter()
    {
      std::remove( tmp_path.c_str() );
    }
    std::string tmp_path;
  }
  tmp_file_deleter{ tmp_path };

  kv::landmark_map_sptr landmark_map =
    kv::read_ply_file(landmarks_path);

  std::vector<kv::vector_3d> points;
  std::vector<kv::rgb_color> colors;
  points.reserve(landmark_map->size());
  colors.reserve(landmark_map->size());
  for (auto lm : landmark_map->landmarks())
  {
    points.push_back(lm.second->loc());
    colors.push_back(lm.second->color());
  }

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  kv::set_geo_conv(&geo_conv);

  auto lgcs = kv::local_geo_cs();
  read_local_geo_cs_from_file(lgcs, geo_origin_path);

  auto pc_io = kwiver::arrows::pdal::pointcloud_io();
  pc_io.set_local_geo_cs(lgcs);
  pc_io.save(tmp_path, points, colors);
}

TEST_F(pointcloud_io, save_landmarks)
{
  auto const geo_origin_path = data_dir + "/geo_origin.txt";
  auto const landmarks_path = data_dir + "/landmarks.ply";
  auto const tmp_path = data_dir + "/pointcloud.las";
  std::ofstream ofs(tmp_path);
  ofs.close();

  // This will delete the temporary file even if an exception is thrown
  struct _tmp_file_deleter
  {
    ~_tmp_file_deleter()
    {
      std::remove( tmp_path.c_str() );
    }
    std::string tmp_path;
  }
  tmp_file_deleter{ tmp_path };

  kv::landmark_map_sptr landmark_map =
    kv::read_ply_file(landmarks_path);

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  kv::set_geo_conv(&geo_conv);

  auto lgcs = kv::local_geo_cs();
  read_local_geo_cs_from_file(lgcs, geo_origin_path);

  auto pc_io = kwiver::arrows::pdal::pointcloud_io();
  pc_io.set_local_geo_cs(lgcs);
  pc_io.save(tmp_path, landmark_map);
}
