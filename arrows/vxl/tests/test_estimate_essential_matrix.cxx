/*ckwg +29
 * Copyright 2014-2017 by Kitware, Inc.
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

#include <test_eigen.h>
#include <test_scene.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <arrows/core/projected_track_set.h>
#include <arrows/core/epipolar_geometry.h>
#include <arrows/vxl/estimate_essential_matrix.h>

#include <Eigen/LU>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(estimate_essential_matrix, create)
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, algo::estimate_essential_matrix::create("vxl") );
}

// ----------------------------------------------------------------------------
// Print epipolar distance of pairs of points given a fundamental matrix
void print_epipolar_distances(const kwiver::vital::matrix_3x3d& F,
                              const std::vector<kwiver::vital::vector_2d> right_pts,
                              const std::vector<kwiver::vital::vector_2d> left_pts)
{
  using namespace kwiver::arrows;
  matrix_3x3d Ft = F.transpose();
  for(unsigned i=0; i<right_pts.size(); ++i)
  {
    const vector_2d& pr = right_pts[i];
    const vector_2d& pl = left_pts[i];
    vector_3d vr(pr.x(), pr.y(), 1.0);
    vector_3d vl(pl.x(), pl.y(), 1.0);
    vector_3d lr = F * vr;
    vector_3d ll = Ft * vl;
    double sr = 1.0 / sqrt(lr.x()*lr.x() + lr.y()*lr.y());
    double sl = 1.0 / sqrt(ll.x()*ll.x() + ll.y()*ll.y());
    // sum of point to epipolar line distance in both images
    double d = vr.dot(ll);
    std::cout <<" dist right = "<<d*sr<<"  dist left = "<<d*sl << std::endl;
  }
}

// ----------------------------------------------------------------------------
// Test essential matrix estimation with ideal points
TEST(estimate_essential_matrix, ideal_points)
{
  using namespace kwiver::arrows;
  vxl::estimate_essential_matrix est_e;

  // create landmarks at the random locations
  landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  track_set_sptr tracks = projected_tracks(landmarks, cameras);

  const frame_id_t frame1 = 0;
  const frame_id_t frame2 = 10;

  camera_map::map_camera_t cams = cameras->cameras();
  camera_sptr cam1 = cams[frame1];
  camera_sptr cam2 = cams[frame2];
  camera_intrinsics_sptr cal1 = cam1->intrinsics();
  camera_intrinsics_sptr cal2 = cam2->intrinsics();

  // compute the true essential matrix from the cameras
  essential_matrix_sptr true_E = essential_matrix_from_cameras(*cam1, *cam2);

  // extract coresponding image points
  std::vector<track_sptr> trks = tracks->tracks();
  std::vector<vector_2d> pts1, pts2;
  for(unsigned int i=0; i<trks.size(); ++i)
  {
    auto fts1 = std::dynamic_pointer_cast<feature_track_state>(*trks[i]->find(frame1));
    auto fts2 = std::dynamic_pointer_cast<feature_track_state>(*trks[i]->find(frame2));
    pts1.push_back(fts1->feature->loc());
    pts2.push_back(fts2->feature->loc());
  }

  // print the epipolar distances using this essential matrix
  fundamental_matrix_sptr F = essential_matrix_to_fundamental(*true_E, *cal1, *cal2);
  print_epipolar_distances(F->matrix(), pts1, pts2);

  // compute the essential matrix from the corresponding points
  std::vector<bool> inliers;
  auto estimated_E = est_e.estimate(pts1, pts2, cal1, cal2, inliers, 1.5);

  // compare true and computed essential matrices
  std::cout << "true E = " << *true_E << std::endl;
  std::cout << "Estimated E = " << *estimated_E << std::endl;
  EXPECT_MATRIX_SIMILAR( true_E->matrix(), estimated_E->matrix(), 1e-8 );

  std::cout << "num inliers " << inliers.size() << std::endl;
  EXPECT_EQ( pts1.size(), inliers.size() )
    << "All points should be inliers";
}

// ----------------------------------------------------------------------------
// Test essential matrix estimation with noisy points
TEST(estimate_essential_matrix, noisy_points)
{
  using namespace kwiver::arrows;
  vxl::estimate_essential_matrix est_e;

  // create landmarks at the random locations
  landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  feature_track_set_sptr tracks = projected_tracks(landmarks, cameras);

  // add random noise to track image locations
  tracks = kwiver::testing::noisy_tracks(tracks, 0.5);

  const frame_id_t frame1 = 0;
  const frame_id_t frame2 = 10;

  camera_map::map_camera_t cams = cameras->cameras();
  camera_sptr cam1 = cams[frame1];
  camera_sptr cam2 = cams[frame2];
  camera_intrinsics_sptr cal1 = cam1->intrinsics();
  camera_intrinsics_sptr cal2 = cam2->intrinsics();

  // compute the true essential matrix from the cameras
  essential_matrix_sptr true_E = essential_matrix_from_cameras(*cam1, *cam2);

  // extract coresponding image points
  std::vector<track_sptr> trks = tracks->tracks();
  std::vector<vector_2d> pts1, pts2;
  for(unsigned int i=0; i<trks.size(); ++i)
  {
    auto fts1 = std::dynamic_pointer_cast<feature_track_state>(*trks[i]->find(frame1));
    auto fts2 = std::dynamic_pointer_cast<feature_track_state>(*trks[i]->find(frame2));
    pts1.push_back(fts1->feature->loc());
    pts2.push_back(fts2->feature->loc());
  }

  // print the epipolar distances using this essential matrix
  fundamental_matrix_sptr F = essential_matrix_to_fundamental(*true_E, *cal1, *cal2);
  print_epipolar_distances(F->matrix(), pts1, pts2);

  // compute the essential matrix from the corresponding points
  std::vector<bool> inliers;
  auto estimated_E = est_e.estimate(pts1, pts2, cal1, cal2, inliers, 1.5);

  // compare true and computed essential matrices
  std::cout << "true E = "<< *true_E << std::endl;
  std::cout << "Estimated E = "<< *estimated_E << std::endl;
  EXPECT_MATRIX_SIMILAR( true_E->matrix(), estimated_E->matrix(), 1e-2 );

  std::cout << "num inliers " << inliers.size() << std::endl;
  EXPECT_GT( inliers.size(), pts1.size() / 3 )
    << "Not enough inliers";
}
