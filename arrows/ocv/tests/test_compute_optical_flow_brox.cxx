/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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

#include <arrows/ocv/compute_optical_flow_brox.h>
#include <arrows/ocv/image_container.h>
#include <vital/plugin_loader/plugin_manager.h>
#include <test_gtest.h>
#include <gtest/gtest.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

kwiver::vital::path_t g_data_dir;

using namespace kwiver::vital;
using namespace kwiver::arrows;
using ocv::compute_optical_flow_brox;

static constexpr double tolerance = 1e-6;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG(1, g_data_dir);

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
// Test that the plugin is created properly
TEST(compute_optical_flow_brox, create)
{
  plugin_manager::instance().load_all_plugins();
  EXPECT_NE( nullptr, compute_optical_flow_brox::create("ocv") );
}

// ---------------------------------------------------------------------------
// Check if the compute is generating images that are similar to a pre-computed image
// Note: This test ensures consistency rather than veracity of the output
TEST(compute_optical_flow_brox, compute)
{
  compute_optical_flow_brox cmp_of;
  cv::Mat img1 = cv::imread( g_data_dir + "/" + "frame001.png" );
  cv::Mat img2 = cv::imread( g_data_dir + "/" + "frame002.png" );
  cv::Mat gt = cv::imread(  g_data_dir + "/" + "gt_flow.png" );
  EXPECT_NE(img1.data, nullptr);
  EXPECT_NE(img2.data, nullptr); 
  EXPECT_NE(gt.data, nullptr);
  auto img1_sptr = 
    std::make_shared< ocv::image_container >(img1, ocv::image_container::ColorMode::RGB_COLOR);
  auto img2_sptr =
    std::make_shared< ocv::image_container >(img2, ocv::image_container::ColorMode::RGB_COLOR);

  auto op_img = cmp_of.compute(img1_sptr, img2_sptr);
  cv::Mat flow_image = ocv::image_container_to_ocv_matrix( *op_img );
  cv::Mat diff_image;
  cv::absdiff(gt, flow_image, diff_image);
  EXPECT_LE(cv::sum(cv::sum(diff_image))[0], tolerance);
}
