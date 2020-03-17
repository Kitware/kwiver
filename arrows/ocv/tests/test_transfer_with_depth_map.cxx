/*ckwg +29
 * Copyright 2020 by Kitware, Inc.
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

#include <test_gtest.h>

#include <tuple>

#include <arrows/ocv/transfer_with_depth_map.h>
#include <arrows/ocv/image_container.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace kwiver::vital;

path_t g_data_dir;

static std::string src_cam_file_name = "src_camera.krtd";
static std::string dest_cam_file_name = "dest_camera.krtd";

// ----------------------------------------------------------------------------
int main( int argc, char* argv[] ) {
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class transfer_with_depth_map : public ::testing::Test {
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(transfer_with_depth_map, backproject_to_depth_map) {
  path_t src_cam_file_path = data_dir + "/" + src_cam_file_name;
  path_t dest_cam_file_path = data_dir + "/" + dest_cam_file_name;

  auto const src_cam_sptr = read_krtd_file(src_cam_file_path);
  auto const dest_cam_sptr = read_krtd_file(dest_cam_file_path);

  // Stub out our depth map
  auto img = cv::Mat_<float>(1080, 1920);
  img.at<float>(278, 645) = 144.04840087890625;

  auto img_ptr = std::make_shared<kwiver::arrows::ocv::image_container>(img, kwiver::arrows::ocv::image_container::OTHER_COLOR);

  auto const transfer_sptr = new kwiver::arrows::ocv::transfer_with_depth_map
    (src_cam_sptr, dest_cam_sptr, img_ptr);

  auto img_point = vector_2d(645.739280245023, 278.8466692189893);

  vector_3d world_point =
    transfer_sptr->backproject_to_depth_map(src_cam_sptr, img_ptr, img_point);

  EXPECT_NEAR(world_point(0), 22.58335929, 1e-6);
  EXPECT_NEAR(world_point(1), -60.0864538, 1e-6);
  EXPECT_NEAR(world_point(2), 1.49882075, 1e-6);
}

// ----------------------------------------------------------------------------
TEST_F(transfer_with_depth_map, backproject_wrt_height) {
  path_t src_cam_file_path = data_dir + "/" + src_cam_file_name;
  path_t dest_cam_file_path = data_dir + "/" + dest_cam_file_name;

  auto const src_cam_sptr = read_krtd_file(src_cam_file_path);
  auto const dest_cam_sptr = read_krtd_file(dest_cam_file_path);

  // Stub out our depth map
  auto img = cv::Mat_<float>(1080, 1920);
  img.at<float>(318, 1065) = 125.21247100830078;

  auto img_ptr = std::make_shared<kwiver::arrows::ocv::image_container>(img, kwiver::arrows::ocv::image_container::OTHER_COLOR);

  auto const transfer_sptr = new kwiver::arrows::ocv::transfer_with_depth_map
    (src_cam_sptr, dest_cam_sptr, img_ptr);

  auto img_point_bottom = vector_2d(1065.0, 318.0);
  auto img_point_top = vector_2d(1074.0, 157.0);

  vector_3d world_point_top;
  std::tie (std::ignore, world_point_top) =
    transfer_sptr->backproject_wrt_height(src_cam_sptr, img_ptr, img_point_bottom, img_point_top);

  EXPECT_NEAR(world_point_top(0), -3.651212895611903, 1e-6);
  EXPECT_NEAR(world_point_top(1), -40.096500055335781, 1e-6);
  EXPECT_NEAR(world_point_top(2), 10.571217535299395, 1e-6);
}
