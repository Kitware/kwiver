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
#include <cstdio>

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
TEST_F(camera_rig_io, output_format_test)
{
  kv::camera_rig_sptr rig0( new kv::camera_rig );
  auto const N = 3;
  kv::path_t const DN = g_data_dir;
  kv::path_t const FNBase = DN + "cam-";
  kv::path_list_t cam_files;
  for ( auto i=1; i<=N; ++i )
  {
    std::stringstream ss;
    ss << FNBase << i << ".krtd";
    kv::vector_3d center( i, .2*i, .2*i );
    kv::rotation_d rotation;
    kv::vector_4d dist( .01*i, .2, .3, .4 );
    kv::camera_intrinsics_sptr intrinsics(
      new kv::simple_camera_intrinsics(
          i, // focal_length,
          kv::vector_2d( 256, 256 ), // principal_point,
          1., // aspect_ratio=1.0,
          0., // skew=0.0,
          dist, // vector_t dist_coeffs=vector_t(),
          512, // image_width=0,
          512 // image_height=0
      )
    );
    kv::simple_camera_perspective_sptr cam(
        new kv::simple_camera_perspective( center, rotation, intrinsics )
    );
    auto const & FN = ss.str();
    rig0->add(FN, cam);
    cam_files.push_back(FN);
  }
  kv::write_camera_rig( rig0 );
// read camera rig and check against the original
  auto rig = kv::read_camera_rig( cam_files );
  EXPECT_NE( rig.get(), nullptr );
  auto const & cams = rig->cameras();
  auto cnt = cams.size();
  EXPECT_EQ( cnt, N );
// make sure the cameras are the same in both rigs
  for ( auto const & c: cams )
  {
    auto const & FN = c.first;
    auto const cam0 =
        dynamic_cast<kv::camera_perspective const *>( rig0->camera(FN).get() );
    auto const cam =
        dynamic_cast<kv::camera_perspective const *>( c.second.get() );

    Eigen::Matrix<double,3,3> K0( cam0->intrinsics()->as_matrix() );
    Eigen::Matrix<double,3,3> K( cam->intrinsics()->as_matrix() );
    EXPECT_MATRIX_EQ( K0, K );

    Eigen::Matrix<double,3,3> R0( cam0->rotation().matrix() );
    Eigen::Matrix<double,3,3> R( cam->rotation().matrix() );
    EXPECT_MATRIX_EQ( R0, R );

    Eigen::Matrix<double,3,1> T0( cam0->translation() );
    Eigen::Matrix<double,3,1> T( cam->translation() );
    EXPECT_MATRIX_EQ( T0, T );

    std::vector<double> D0 = cam0->intrinsics()->dist_coeffs();
    std::vector<double> D = cam->intrinsics()->dist_coeffs();
    EXPECT_EQ( D0, D );
  }
// cleanup
  for ( std::string const & FN: cam_files )
  {
    EXPECT_EQ( 0, std::remove(FN.c_str()) );
  }
}
