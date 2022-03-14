// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test geo_MGRS functionality

#include <tests/test_gtest.h>

#include <vital/exceptions/io.h>
#include <vital/types/geodesy.h>
#include <vital/types/local_geo_cs.h>
#include <arrows/proj/geo_conv.h>

#include <sstream>
#include <fstream>

namespace kv = kwiver::vital;
kv::path_t g_data_dir;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class local_geo_cs : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(local_geo_cs, read_local_geo_cs_from_file)
{
  auto const geo_origin_valid = data_dir + "/geo_origin_aphill.txt";
  auto const geo_origin_invalid = data_dir + "/geo_origin_empty.txt";

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  kv::set_geo_conv(&geo_conv);
  int crs = kv::SRID::lat_lon_WGS84;
  bool valid = false;

  // Test valid file
  kv::local_geo_cs lgcs;
  valid = read_local_geo_cs_from_file(lgcs, geo_origin_valid);
  EXPECT_TRUE(valid);
  // Expect the local geo cs to be valid
  kv::vector_3d origin = lgcs.origin().location();
  EXPECT_NEAR(origin[0], -77.3578172263, 1e-8);
  EXPECT_NEAR(origin[1], 38.1903504278, 1e-8);
  EXPECT_NEAR(origin[2], -68.0169758322, 1e-8);

  // Test invalid file
  lgcs.set_origin(kv::geo_point(kv::vector_3d(0, 0, 0), crs));
  valid = kv::read_local_geo_cs_from_file(lgcs, geo_origin_invalid);
  EXPECT_FALSE(valid);
  // Expect the origin to be unchanged
  origin = lgcs.origin().location();
  EXPECT_NEAR(origin[0], 0.0, 1e-8);
  EXPECT_NEAR(origin[1], 0.0, 1e-8);
  EXPECT_NEAR(origin[2], 0.0, 1e-8);
}

TEST_F(local_geo_cs, write_local_geo_cs_to_file)
{
  auto const tmp_path_valid = data_dir + "/geo_origin_write.txt";

  // This will delete the temporary file even if an exception is thrown
  struct _tmp_file_deleter {
    ~_tmp_file_deleter() {
      std::remove( tmp_path.c_str() );
    }

    std::string tmp_path;
  } tmp_file_deleter{ tmp_path_valid };

  static auto geo_conv = kwiver::arrows::proj::geo_conversion{};
  kv::set_geo_conv(&geo_conv);
  int crs = kv::SRID::lat_lon_WGS84;
  bool valid = false;

  // Test valid file
  kv::local_geo_cs lgcs;
  lgcs.set_origin(kv::geo_point(kv::vector_3d(1, 1, 1), crs));
  valid = kv::write_local_geo_cs_to_file(lgcs, tmp_path_valid);
  EXPECT_TRUE(valid);

  // Test that the file was written
  std::ifstream ifs(tmp_path_valid);
  double lat = 0, lon = 0, alt = 0;
  ifs >> lat >> lon >> alt;
  EXPECT_TRUE(ifs.good());

  // Test that the file contains the correct data
  EXPECT_NEAR(lat, 1.0, 1e-8);
  EXPECT_NEAR(lon, 1.0, 1e-8);
  EXPECT_NEAR(alt, 1.0, 1e-8);
}
