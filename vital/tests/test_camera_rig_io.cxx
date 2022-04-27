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

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG(1, g_data_dir);
  g_data_dir += "/";

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class camera_rig_io : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(camera_rig_io, krtd_test)
{
  kv::path_t const DN = g_data_dir;
  kv::path_t const CNBase = "cam";
  kv::path_t const FNBase = DN + CNBase;
  kv::path_list_t cam_names;
  unsigned N = 3;
// form a camera rig
  kv::camera_rig_sptr rig0( new kv::camera_rig );

  auto const dim = 512, dim2 = dim/2;
  kv::vector_2d const principal_point( dim2, dim2 );
  auto const aspect_ratio = 1.0, skew = 0.0;

  for ( unsigned i=1; i<=N; ++i )
  {
    unsigned j=i-1;
    kv::vector_3d center( j, .2*j, .2*j );
    kv::rotation_d rotation;
    kv::vector_4d dist( .01*i, .2, .3, .4 );
    auto const focal_length = .1*i;
    kv::camera_intrinsics_sptr intrinsics(
      new kv::simple_camera_intrinsics(
          focal_length,
          principal_point,
          aspect_ratio,
          skew,
          dist,
          dim, dim
      )
    );
    kv::simple_camera_perspective_sptr cam(
      new kv::simple_camera_perspective( center, rotation, intrinsics )
    );
  // camera name
    std::string const ext = ".krtd";
    std::stringstream ss;
    ss << FNBase << i << ext;
    auto const & CN = ss.str();
    rig0->add( CN, cam );
    cam_names.push_back( CN );
  }
  kv::write_camera_rig( rig0 );

// read camera rig and check against the original
  auto rig = kv::read_camera_rig( cam_names );
  ASSERT_TRUE( rig.get() );
  auto const & cams = rig->cameras();
  auto cnt = cams.size();
  EXPECT_EQ( cnt, N );

// make sure the cameras are the same in both rigs
  for ( auto const & c: cams )
  {
    auto const & CN = c.first;
    auto const cam0 =
      dynamic_cast<kv::camera_perspective const *>( rig0->camera(CN).get() );
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
  for ( std::string const & CN: cam_names )
  {
    EXPECT_EQ( 0, std::remove(CN.c_str()) );
  }
}

// ----------------------------------------------------------------------------
void io_stereo_test(std::string const & ext)
{
  kv::path_t const DN = g_data_dir;
  kv::path_t const CNBase = "cam";
  kv::path_t const FNBase = DN + CNBase;

// form a stereo rig
  unsigned const N = 2;
  auto const dim = 512, dim2 = dim/2;
  kv::vector_2d const principal_point( dim2, dim2 );
  auto const aspect_ratio = 1.0, skew = 0.0;

  kv::camera_collection cams0;
  for ( unsigned i=1; i<=N; ++i )
  {
    unsigned j = i-1;
    kv::vector_3d center( j, .2*j, .2*j );
    kv::rotation_d rotation;
    kv::vector_4d dist( .01*i, .2, .3, .4 );
    auto const focal_length = .1*i;
    kv::camera_intrinsics_sptr intrinsics(
      new kv::simple_camera_intrinsics(
          focal_length,
          principal_point,
          aspect_ratio,
          skew,
          dist,
          dim, dim
      )
    );
    kv::simple_camera_perspective_sptr cam(
      new kv::simple_camera_perspective( center, rotation, intrinsics )
    );
    auto const CN = i==1 ? "left" : "right";
    cams0[CN] = cam;
  }

  auto rig0 = std::make_shared<kv::camera_rig_stereo>(
      cams0["left"], cams0["right"]
  );

  auto const & FN = FNBase+ext;
  kv::write_stereo_rig( rig0, FN );

// read camera rig and check against the original
  auto rig = kv::read_stereo_rig( FN );
  ASSERT_TRUE( rig.get() );
  auto const & cams = rig->cameras();
  auto cnt = cams.size();
  EXPECT_EQ( cnt, N );

// make sure the rigs are the same
  for ( auto const & c: cams )
  {
    auto const & CN = c.first;
    auto const cam0 =
      dynamic_cast<kv::camera_perspective const *>( rig0->camera(CN).get() );
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
  EXPECT_EQ( 0, std::remove(FN.c_str()) );
}

// ----------------------------------------------------------------------------
TEST_F(camera_rig_io, stereo_json_test)
{
  io_stereo_test(".json");
}

// ----------------------------------------------------------------------------
TEST_F(camera_rig_io, stereo_yaml_test)
{
  io_stereo_test(".yaml");
}
