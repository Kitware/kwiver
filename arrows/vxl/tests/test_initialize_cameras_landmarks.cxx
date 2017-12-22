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

#include <test_gtest.h>
#include <test_eigen.h>
#include <test_scene.h>

#include <arrows/vxl/estimate_essential_matrix.h>
#include <arrows/vxl/estimate_similarity_transform.h>

#include <arrows/core/projected_track_set.h>
#include <arrows/core/metrics.h>
#include <arrows/core/transform.h>
#include <arrows/core/initialize_cameras_landmarks.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <vital/types/similarity.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST(initialize_cameras_landmarks, create)
{
  EXPECT_NE( nullptr, algo::initialize_cameras_landmarks::create("core") );
}

// ----------------------------------------------------------------------------
// Helper function to configure the algorithm
static void
configure_algo(kwiver::arrows::core::initialize_cameras_landmarks& algo,
               const kwiver::vital::camera_intrinsics_sptr K)
{
  using namespace kwiver::arrows;
  kwiver::vital::config_block_sptr cfg = algo.get_configuration();
  cfg->set_value("verbose", "true");
  cfg->set_value("base_camera:focal_length", K->focal_length());
  cfg->set_value("base_camera:principal_point", K->principal_point());
  cfg->set_value("base_camera:aspect_ratio", K->aspect_ratio());
  cfg->set_value("base_camera:skew", K->skew());
  cfg->set_value("essential_mat_estimator:type", "vxl");
  cfg->set_value("essential_mat_estimator:vxl:num_ransac_samples", 10);
  cfg->set_value("camera_optimizer:type", "vxl");
  cfg->set_value("lm_triangulator:type", "core");
  algo.set_configuration(cfg);

  if(!algo.check_configuration(cfg))
  {
    std::cout << "Error: configuration is not correct" << std::endl;
    return;
  }
}

// ----------------------------------------------------------------------------
static void
evaluate_initialization(const kwiver::vital::camera_map_sptr true_cams,
                        const kwiver::vital::landmark_map_sptr true_landmarks,
                        const kwiver::vital::camera_map_sptr est_cams,
                        const kwiver::vital::landmark_map_sptr est_landmarks,
                        double tol)
{
  using namespace kwiver;

  typedef vital::algo::estimate_similarity_transform_sptr est_sim_sptr;
  est_sim_sptr est_sim(new arrows::vxl::estimate_similarity_transform());
  vital::similarity_d global_sim = est_sim->estimate_transform(est_cams, true_cams);
  std::cout << "similarity = "<<global_sim<<std::endl;


  vital::camera_map::map_camera_t orig_cams = true_cams->cameras();
  vital::camera_map::map_camera_t new_cams = est_cams->cameras();
  for (auto const& p : orig_cams)
  {
    vital::camera_sptr new_cam_t = arrows::transform(new_cams[p.first], global_sim);
    vital::rotation_d dR = new_cam_t->rotation().inverse() * p.second->rotation();
    EXPECT_NEAR(0.0, dR.angle(), tol) << "Rotation difference magnitude";

    double dt = (p.second->center() - new_cam_t->center()).norm();
    EXPECT_NEAR(0.0, dt, tol) << "Camera center difference";
  }

  vital::landmark_map::map_landmark_t orig_lms = true_landmarks->landmarks();
  vital::landmark_map::map_landmark_t new_lms = est_landmarks->landmarks();
  for (auto const& p : orig_lms)
  {
    vital::landmark_sptr new_lm_tr = arrows::transform(new_lms[p.first], global_sim);

    double dt = (p.second->loc() - new_lm_tr->loc()).norm();
    EXPECT_NEAR(0.0, dt, tol) << "Landmark location difference";
  }
}

// ----------------------------------------------------------------------------
// Test initialization with ideal points
TEST(initialize_cameras_landmarks, ideal_points)
{
  using namespace kwiver;
  arrows::core::initialize_cameras_landmarks init;

  // create landmarks at the random locations
  vital::landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  feature_track_set_sptr tracks = arrows::projected_tracks(landmarks, cameras);

  vital::camera_intrinsics_sptr K = cameras->cameras()[0]->intrinsics();
  configure_algo(init, K);

  vital::camera_map_sptr new_cameras;
  vital::landmark_map_sptr new_landmarks;
  init.initialize(new_cameras, new_landmarks, tracks);

  evaluate_initialization(cameras, landmarks,
                          new_cameras, new_landmarks,
                          1e-6);
}

// ----------------------------------------------------------------------------
// Test initialization with ideal points
TEST(initialize_cameras_landmarks, ideal_points_from_last)
{
  using namespace kwiver;
  arrows::core::initialize_cameras_landmarks init;

  // create landmarks at the random locations
  vital::landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  vital::camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  vital::feature_track_set_sptr tracks = arrows::projected_tracks(landmarks, cameras);

  vital::config_block_sptr cfg = init.get_configuration();
  cfg->set_value("init_from_last", "true");
  init.set_configuration(cfg);
  vital::camera_intrinsics_sptr K = cameras->cameras()[0]->intrinsics();
  configure_algo(init, K);

  vital::camera_map_sptr new_cameras;
  vital::landmark_map_sptr new_landmarks;
  init.initialize(new_cameras, new_landmarks, tracks);

  evaluate_initialization(cameras, landmarks,
                          new_cameras, new_landmarks,
                          1e-6);
}

// ----------------------------------------------------------------------------
// Test initialization with noisy points
TEST(initialize_cameras_landmarks, noisy_points)
{
  using namespace kwiver;
  arrows::core::initialize_cameras_landmarks init;

  // create landmarks at the random locations
  vital::landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  vital::camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  vital::feature_track_set_sptr tracks = arrows::projected_tracks(landmarks, cameras);

  // add random noise to track image locations
  tracks = kwiver::testing::noisy_tracks(tracks, 0.3);

  camera_intrinsics_sptr K = cameras->cameras()[0]->intrinsics();
  configure_algo(init, K);

  vital::camera_map_sptr new_cameras;
  vital::landmark_map_sptr new_landmarks;
  init.initialize(new_cameras, new_landmarks, tracks);

  evaluate_initialization(cameras, landmarks,
                          new_cameras, new_landmarks,
                          0.2);
}

// ----------------------------------------------------------------------------
// Test initialization with noisy points
TEST(initialize_cameras_landmarks, noisy_points_from_last)
{
  using namespace kwiver;
  arrows::core::initialize_cameras_landmarks init;

  // create landmarks at the random locations
  vital::landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  vital::camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  vital::feature_track_set_sptr tracks = arrows::projected_tracks(landmarks, cameras);

  // add random noise to track image locations
  tracks = kwiver::testing::noisy_tracks(tracks, 0.3);

  vital::config_block_sptr cfg = init.get_configuration();
  cfg->set_value("init_from_last", "true");
  init.set_configuration(cfg);
  vital::camera_intrinsics_sptr K = cameras->cameras()[0]->intrinsics();
  configure_algo(init, K);

  vital::camera_map_sptr new_cameras;
  landmark_map_sptr new_landmarks;
  init.initialize(new_cameras, new_landmarks, tracks);

  evaluate_initialization(cameras, landmarks,
                          new_cameras, new_landmarks,
                          0.2);
}

// ----------------------------------------------------------------------------
// Test initialization with subsets of cameras and landmarks
TEST(initialize_cameras_landmarks, subset_init)
{
  using namespace kwiver;
  arrows::core::initialize_cameras_landmarks init;

  // create landmarks at the random locations
  vital::landmark_map_sptr landmarks = kwiver::testing::init_landmarks(100);
  landmarks = kwiver::testing::noisy_landmarks(landmarks, 1.0);

  // create a camera sequence (elliptical path)
  vital::camera_map_sptr cameras = kwiver::testing::camera_seq();

  // create tracks from the projections
  vital::feature_track_set_sptr tracks = arrows::projected_tracks(landmarks, cameras);

  vital::camera_intrinsics_sptr K = cameras->cameras()[0]->intrinsics();
  configure_algo(init, K);

  // mark every 3rd camera for initialization
  vital::camera_map::map_camera_t cams_to_init;
  for(unsigned int i=0; i<cameras->size(); ++i)
  {
    if( i%3 == 0 )
    {
      cams_to_init[i] = camera_sptr();
    }
  }
  vital::camera_map_sptr new_cameras(new simple_camera_map(cams_to_init));

  // mark every 5rd landmark for initialization
  vital::landmark_map::map_landmark_t lms_to_init;
  for(unsigned int i=0; i<landmarks->size(); ++i)
  {
    if( i%5 == 0 )
    {
      lms_to_init[i] = vital::landmark_sptr();
    }
  }
  vital::landmark_map_sptr new_landmarks(new vital::simple_landmark_map(lms_to_init));

  init.initialize(new_cameras, new_landmarks, tracks);

  // test that we only initialized the requested objects
  for (auto const& p : new_cameras->cameras())
  {
    auto const& camera_id = p.first;
    auto const& camera_ptr = p.second;

    EXPECT_NE( nullptr, camera_ptr );
    EXPECT_EQ( 0, camera_id % 3 );
  }
  for (auto const& p : new_landmarks->landmarks())
  {
    auto const& landmark_id = p.first;
    auto const& landmark_ptr = p.second;

    EXPECT_NE( nullptr, landmark_ptr );
    EXPECT_EQ( 0, landmark_id % 5 );
  }

  // calling this again should do nothing
  vital::camera_map_sptr before_cameras = new_cameras;
  vital::landmark_map_sptr before_landmarks = new_landmarks;
  init.initialize(new_cameras, new_landmarks, tracks);
  EXPECT_EQ( before_cameras, new_cameras )
    << "Initialization should be no-op";
  EXPECT_EQ( before_landmarks, new_landmarks )
    << "Initialization should be no-op";

  // add the rest of the cameras
  cams_to_init = new_cameras->cameras();
  for(unsigned int i=0; i<cameras->size(); ++i)
  {
    if( cams_to_init.find(i) == cams_to_init.end() )
    {
      cams_to_init[i] = camera_sptr();
    }
  }
  new_cameras = vital::camera_map_sptr(new vital::simple_camera_map(cams_to_init));

  // add the rest of the landmarks
  lms_to_init = new_landmarks->landmarks();
  for(unsigned int i=0; i<landmarks->size(); ++i)
  {
    if( lms_to_init.find(i) == lms_to_init.end() )
    {
      lms_to_init[i] = vital::landmark_sptr();
    }
  }
  new_landmarks = vital::landmark_map_sptr(new simple_landmark_map(lms_to_init));

  // initialize the rest
  init.initialize(new_cameras, new_landmarks, tracks);

  evaluate_initialization(cameras, landmarks,
                          new_cameras, new_landmarks,
                          1e-6);
}
