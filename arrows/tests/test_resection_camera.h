// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <algorithm>

#include <test_eigen.h>
#include <test_scene.h>

#include <arrows/mvg/projected_track_set.h>

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
  camera_intrinsics_sptr cal = cam->intrinsics();

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
  auto est_cam = res_cam.resection(pts_projs, pts_3d, inliers, cam->intrinsics());

  // true and computed camera poses
  const auto
    &camR = cam->rotation(),
    &estR = est_cam->rotation();
  cout << "cam R:\n" << camR.matrix() << endl
  << "est R:\n" << estR.matrix() << endl
  << "cam C = " << cam->center().transpose() << endl
  << "est C = " << est_cam->center().transpose() << endl;

  auto R_err = camR.inverse()*estR;
  cout << "rotation error = " << R_err.angle()*180./pi << " degrees" << endl;

  EXPECT_LT(R_err.angle(), ideal_rotation_tolerance);
  EXPECT_MATRIX_SIMILAR(cam->center(), est_cam->center(), ideal_center_tolerance);

  auto inlier_cnt = count(inliers.begin(), inliers.end(), true);
  cout << "inlier count = " << inlier_cnt << endl;
  EXPECT_EQ(pts_projs.size(), inlier_cnt) << "all points should be inliers";
}

// ----------------------------------------------------------------------------
// Test camera resection with noisy points
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

  auto cams = cameras->cameras();
  auto cam = dynamic_pointer_cast<camera_perspective>(cams[frmID]);
  camera_intrinsics_sptr cal = cam->intrinsics();

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
  auto est_cam = res_cam.resection(pts_projs, pts_3d, inliers, cam->intrinsics());

  // true and computed camera poses
  const auto
    &camR = cam->rotation(),
    &estR = est_cam->rotation();
  cout << "cam R:\n" << camR.matrix() << endl
  << "est R:\n" << estR.matrix() << endl
  << "cam C = " << cam->center().transpose() << endl
  << "est C = " << est_cam->center().transpose() << endl;

  auto R_err = camR.inverse()*estR;
  cout << "rotation error = " << R_err.angle()*180./pi << " degrees" << endl;

  EXPECT_LT(R_err.angle(), noisy_rotation_tolerance);
  EXPECT_MATRIX_SIMILAR(cam->center(), est_cam->center(), noisy_center_tolerance);

  auto inlier_cnt = count(inliers.begin(), inliers.end(), true);
  cout << "inlier count = " << inlier_cnt << endl;
  EXPECT_GT(inlier_cnt, pts_projs.size()/2) << "not enough inliers";
}

// ----------------------------------------------------------------------------
// Test camera resection with noisy points, missing tracks and initial calibration guess.
TEST(resection_camera, noisy_points_cal)
{
  // landmarks at random locations
  landmark_map_sptr landmarks = kwiver::testing::init_landmarks(128);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  auto tracks = dynamic_pointer_cast<feature_track_set>(
    projected_tracks(landmarks, cameras));

  // add random noise to track image locations
  tracks = kwiver::testing::noisy_tracks(tracks, 0.5);

  const frame_id_t frmID = 10;

  camera_map::map_camera_t cams = cameras->cameras();
  auto cam = dynamic_pointer_cast<camera_perspective>(cams[frmID]);
  camera_intrinsics_sptr cal = cam->intrinsics();

  // extract corresponding image points
  unsigned int i = 0;
  vector<vector_2d> pts_projs;
  vector<vector_3d> pts_3d;
  for (auto const& track : tracks->tracks())
  {
    auto lm_id = track->id();
    auto lm_it = landmarks->landmarks().find(lm_id);
    pts_3d.push_back(lm_it->second->loc());

    if (++i % 3 == 0)
    {
      pts_projs.push_back(kwiver::testing::random_point2d(1000.0));
    }
    else
    {
      auto const fts =
        dynamic_pointer_cast<feature_track_state>(*track->find(frmID));
      pts_projs.push_back(fts->feature->loc());
    }
  }

  // compute the camera pose from the points and their projections
  vector<bool> inliers;
  auto est_cam = res_cam.resection(pts_projs, pts_3d, inliers, cam->intrinsics());

  // compare true and computed camera poses
  cout << "true R = " << cam->rotation().matrix() << endl;
  cout << "Estimated R = " << est_cam->rotation().matrix() << endl;
  cout << "true C = " << cam->center() << endl;
  cout << "Estimated C = " << est_cam->center() << endl;

  auto R_err = cam->rotation().inverse()*est_cam->rotation();
  cout << "Rotation error = " << R_err.angle()*180.0 / pi << " degrees" << endl;

  // resection camera using 3D landmarks, 2D feature tracks and a camera intrinsics guess
  resection_camera res_cam;
  auto cams = cameras->cameras();
  auto cam = dynamic_pointer_cast<camera_perspective>(cams[frmID]);
  auto est_cam = res_cam.resection(frmID, landmarks, tracks, cam->intrinsics());
  EXPECT_NE(est_cam, nullptr) << "null resection camera on noisy points, missing tracks with initial cal guess";

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

  unsigned int inlier_cnt = count(inliers.begin(), inliers.end(), true);
  cout << "num inliers " << inlier_cnt << endl;
  EXPECT_GT(inlier_cnt, pts_projs.size() / 3)
    << "Not enough inliers";
}
