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

#include <arrows/ocv/transfer_with_depth_map.h>
#include <arrows/ocv/image_container.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace kwiver::vital;

path_t g_data_dir;

static std::string src_cam_file_name = "src_camera.krtd";
static std::string dest_cam_file_name = "dest_camera.krtd";
static std::string src_depth_map_file_name = "src_depth_map.tif";

// ----------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class transfer_with_depth_map : public ::testing::Test
{
  TEST_ARG(data_dir);
};

// ----------------------------------------------------------------------------
TEST_F(transfer_with_depth_map, backproject_to_depth_map)
{
  path_t src_cam_file_path = data_dir + "/" + src_cam_file_name;
  path_t dest_cam_file_path = data_dir + "/" + dest_cam_file_name;
  path_t src_depth_map_file_path = data_dir + "/" + src_depth_map_file_name;

  auto const src_cam_sptr = read_krtd_file(src_cam_file_path);
  auto const dest_cam_sptr = read_krtd_file(dest_cam_file_path);

  cv::Mat img = cv::imread(src_depth_map_file_path.c_str(), -1);
  // auto const depth_map_sptr = kwiver::arrows::ocv::image_io::load_(src_depth_map_file_path);
  auto img_ptr = image_container_sptr(new kwiver::arrows::ocv::image_container(img, kwiver::arrows::ocv::image_container::OTHER_COLOR));

  auto const transfer_sptr = new kwiver::arrows::ocv::transfer_with_depth_map
    (src_cam_sptr, dest_cam_sptr, img_ptr);

  auto img_point = vector_2d(0, 0);

  vector_3d world_point =
    transfer_sptr->backproject_to_depth_map(src_cam_sptr, img_ptr, img_point);

  EXPECT_EQ(world_point, vector_3d(0.0, 0.0, 0.0));
}
