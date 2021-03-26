// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/math_constants.h>
#include <arrows/mvg/projected_track_set.h>
#include <test_scene.h>
#include <test_eigen.h>

using namespace kwiver::vital;
using namespace kwiver::arrows::mvg;
using namespace std;

// ----------------------------------------------------------------------------
// Test camera resection with ideal points.
TEST(resection_camera, ideal_points)
{
  // landmarks at random locations
  landmark_map_sptr landmarks = kwiver::testing::init_landmarks(128);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // tracks from projections
  track_set_sptr tracks = projected_tracks(landmarks, cameras);

  const frame_id_t frmID = 0;

  auto cams = cameras->cameras();
  auto cam = dynamic_pointer_cast<camera_perspective>(cams[frmID]);

  // corresponding image points
  vector<vector_2d> pts_projs;
  vector<vector_3d> pts_3d;
  for (auto const& track : tracks->tracks())
  {
    auto lm_id = track->id();
    auto lm_it = landmarks->landmarks().find(lm_id);
    pts_3d.push_back(lm_it->second->loc());
    auto const fts = dynamic_pointer_cast<feature_track_state>(*track->find(frmID));
    pts_projs.push_back(fts->feature->loc());
  }

  // camera pose from the points and their projections
  vector<bool> inliers;
  resection_camera res_cam;
  auto est_cam = res_cam.resection(pts_projs, pts_3d, cam->intrinsics(), inliers);

  // true and computed camera poses
  const auto
    &camR = cam->rotation(),
    &estR = est_cam->rotation();
  cout << "cam R:\n" << camR.matrix() << endl
  << "est R:\n" << estR.matrix() << endl
  << "cam C = " << cam->center().transpose() << endl
  << "est C = " << est_cam->center().transpose() << endl;

  auto R_err = camR.inverse()*estR;
  cout << "rotation error = " << R_err.angle()*180/pi << " degrees" << endl;

  EXPECT_LT(R_err.angle(), ideal_rotation_tolerance);
  EXPECT_MATRIX_SIMILAR(cam->center(), est_cam->center(), ideal_center_tolerance);

  auto inlier_cnt = count(inliers.begin(), inliers.end(), true);
  cout << "inlier count = " << inlier_cnt << endl;
  EXPECT_EQ(pts_projs.size(), inlier_cnt) << "all points should be inliers";
}

// ----------------------------------------------------------------------------
// Test camera resection with noisy points.
TEST(resection_camera, noisy_points)
{
  // landmarks at random locations
  landmark_map_sptr landmarks = kwiver::testing::init_landmarks(128);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // tracks from the projections
  auto tracks = dynamic_pointer_cast<feature_track_set>(projected_tracks(landmarks, cameras));

  // random noise to track image locations
  tracks = kwiver::testing::noisy_tracks(tracks, 0.5);

  const frame_id_t frmID = 1;

  // corresponding image points
  vector<vector_2d> pts_projs;
  vector<vector_3d> pts_3d;
  for (auto const & track : tracks->tracks())
  {
    auto lm_id = track->id();
    auto lm_it = landmarks->landmarks().find(lm_id);
    pts_3d.push_back(lm_it->second->loc());
    auto const fts = dynamic_pointer_cast<feature_track_state>(*track->find(frmID));
    pts_projs.push_back(fts->feature->loc());
  }

  // camera pose from the points and their projections
  resection_camera res_cam;
  auto cams = cameras->cameras();
  auto cam = dynamic_pointer_cast<camera_perspective>(cams[frmID]);
  vector<bool> inliers;
  auto est_cam = res_cam.resection(pts_projs, pts_3d, cam->intrinsics(), inliers);
  EXPECT_NE(est_cam, nullptr) << "resection camera failed";

  // true and computed camera poses
  const auto
    &camR = cam->rotation(),
    &estR = est_cam->rotation();
  cout << "cam R:\n" << camR.matrix() << endl
  << "est R:\n" << estR.matrix() << endl
  << "cam C = " << cam->center().transpose() << endl
  << "est C = " << est_cam->center().transpose() << endl;

  auto R_err = camR.inverse()*estR;
  cout << "rotation error = " << R_err.angle()*180/pi << " degrees" << endl;

  EXPECT_LT(R_err.angle(), noisy_rotation_tolerance);
  EXPECT_MATRIX_SIMILAR(cam->center(), est_cam->center(), noisy_center_tolerance);

  auto inlier_cnt = count(inliers.begin(), inliers.end(), true);
  cout << "inlier count = " << inlier_cnt << endl;
  EXPECT_GT(inlier_cnt, pts_projs.size()/2) << "not enough inliers";
}

// ----------------------------------------------------------------------------
// Test camera resection with outliers.
TEST(resection_camera, outlier_points)
{
  // landmarks at random locations
  landmark_map_sptr landmarks = kwiver::testing::init_landmarks(128);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // tracks from projections
  auto tracks = dynamic_pointer_cast<feature_track_set>( projected_tracks(landmarks, cameras));

  // random noise to track image locations
  tracks = kwiver::testing::noisy_tracks(tracks, 0.5);

  const frame_id_t frmID = 2;

  auto cams = cameras->cameras();
  auto cam = dynamic_pointer_cast<camera_perspective>(cams[frmID]);

  // corresponding image points
  unsigned i = 0;
  vector<vector_2d> pts_projs;
  vector<vector_3d> pts_3d;
  for (auto const& track : tracks->tracks())
  {
    auto lm_id = track->id();
    auto lm_it = landmarks->landmarks().find(lm_id);
    pts_3d.push_back(lm_it->second->loc());
    if (++i % 3)
    {
      auto const fts = dynamic_pointer_cast<feature_track_state>(*track->find(frmID));
      pts_projs.push_back(fts->feature->loc());
    }
    else // outlier
      pts_projs.push_back(kwiver::testing::random_point2d(1000.));
  }

  // camera pose from the points and their projections
  vector<bool> inliers;
  resection_camera res_cam;
  auto est_cam = res_cam.resection(pts_projs, pts_3d, cam->intrinsics(), inliers);

  // true and computed camera poses
  const auto
    &camR = cam->rotation(),
    &estR = est_cam->rotation();
  cout << "cam R:\n" << camR.matrix() << endl
  << "est R:\n" << estR.matrix() << endl
  << "cam C = " << cam->center().transpose() << endl
  << "est C = " << est_cam->center().transpose() << endl;

  auto R_err = camR.inverse()*estR;
  cout << "rotation error = " << R_err.angle()*180/pi << " degrees" << endl;

  // compare true and computed camera poses
  EXPECT_LT(R_err.angle(), outlier_rotation_tolerance);
  EXPECT_MATRIX_SIMILAR(cam->center(), est_cam->center(), outlier_center_tolerance);

  auto inlier_cnt = count(inliers.begin(), inliers.end(), true);
  cout << "inlier count = " << inlier_cnt << endl;
  EXPECT_GT(inlier_cnt, pts_projs.size()/3) << "not enough inliers";
}
