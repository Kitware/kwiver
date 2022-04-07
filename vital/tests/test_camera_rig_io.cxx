// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief core camera_rig_io tests

#include <vital/io/camera_rig_io.h>
#include <vital/vital_types.h>
#include <vital/exceptions.h>
#include <vital/types/camera_perspective.h>

#include <tests/test_eigen.h>
#include <tests/test_gtest.h>

#include <iostream>
#include <sstream>

namespace kv = kwiver::vital;

kv::path_t g_data_dir;

static std::string const krtd = "vital_data/test_camera_io-";

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class camera_rig_io : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(camera_rig_io, KRTD_format_read)
{
  kv::path_t const test_read_file = data_dir + krtd;
  auto const N = 3;
  kv::path_list_t cam_files;
  for ( auto i=1; i<=N; ++i )
  {
    std::stringstream ss;
    ss << test_read_file << i << ".krtd";
    cam_files.push_back(ss.str());
  }
  kv::camera_rig_sptr rig = kv::read_camera_rig( cam_files );
  EXPECT_NE( rig.get(), nullptr );

  auto const & cams = rig->cameras();
  auto cnt = cams.size();
  EXPECT_EQ( cnt, N );
  Eigen::Matrix<double,3,3> expected_intrinsics;
  expected_intrinsics << 1, 2, 3,
                         0, 5, 6,
                         0, 0, 1;
  Eigen::Matrix<double,3,3> expected_rotation;
  expected_rotation << 1, 0, 0,
                       0, 1, 0,
                       0, 0, 1;
  for (auto const & kvp: cams)
  {
    auto const & cam = dynamic_cast<kv::camera_perspective const *>(kvp.second.get());
    Eigen::Matrix<double,3,3> K( cam->intrinsics()->as_matrix() );
    EXPECT_MATRIX_EQ( expected_intrinsics, K );
    Eigen::Matrix<double,3,3> R( cam->rotation().matrix() );
    EXPECT_MATRIX_EQ( expected_rotation, R );
    Eigen::Matrix<double,3,1> T( cam->translation() );
    auto n = kvp.first[kvp.first.length()-6]-'0';
    Eigen::Matrix<double,3,1> expected_translation;
    expected_translation << 1, 2, 3;
    for ( auto i=0; i<3; ++i )
    {
      auto & e = expected_translation[i];
      e = 10*e + n;
    }
    EXPECT_MATRIX_EQ( expected_translation, T );
    std::vector<double> expected_distortion = {1, 2, 3, 4, 5};
    std::vector<double> D = cam->intrinsics()->dist_coeffs() ;
    EXPECT_EQ( expected_distortion, D );
  }
}

// ----------------------------------------------------------------------------
TEST_F(camera_rig_io, output_format_test)
{
/* TODO: remove/modify
  kv::simple_camera_perspective cam;
  std::cerr << "Default constructed camera\n" << cam << std::endl;
  std::cerr << "cam.get_center()     : " << kv::vector_3d(cam.get_center()).transpose() << std::endl;
  std::cerr << "cam.get_rotation()   : " << cam.get_rotation() << std::endl;
  std::cerr << "cam.get_translation(): " << cam.translation() << std::endl;

  // We're expecting -0's as this is what Eigen likes to output when a zero
  // vector is negated.
  std::stringstream ss;
  ss << cam;
  EXPECT_EQ(
    "1 0 0\n"
    "0 1 0\n"
    "0 0 1\n"
    "\n"
    "1 0 0\n"
    "0 1 0\n"
    "0 0 1\n"
    "\n"
    "-0 -0 -0\n"
    "\n"
    "0\n",
    ss.str() )
    << "Camera output string test";
*/
}
