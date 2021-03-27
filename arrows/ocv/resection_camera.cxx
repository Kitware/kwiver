// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief OCV resection_camera algorithm implementation
 */

#include "resection_camera.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/eigen.hpp>
#include <cmath>

#include "camera_intrinsics.h"

using namespace std;
using namespace cv;
using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv
{

/// private implementation
struct resection_camera::priv
{
  priv()
    : confidence_threshold(0.99)
    , max_iterations(10000)
    , m_logger( vital::get_logger( "arrows.ocv.resection_camera" ))
  {
  }

  double confidence_threshold;
  int max_iterations;

  vital::logger_handle_t m_logger;
};

resection_camera::resection_camera()
: d_(new priv)
{
}

resection_camera::~resection_camera()
{
}

/// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
resection_camera::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config =
      vital::algo::resection_camera::get_configuration();
  config->set_value("confidence_threshold", d_->confidence_threshold,
    "Confidence that estimated matrix is correct, range (0.0, 1.0]");
  config->set_value("max_iterations", d_->max_iterations,
    "maximum number of iterations to run PnP [1, INT_MAX]");
  return config;
}

/// Set this algorithm's properties via a config block
void
resection_camera::set_configuration(vital::config_block_sptr config)
{
  d_->confidence_threshold =
    config->get_value<double>("confidence_threshold", d_->confidence_threshold);
  d_->max_iterations =
    config->get_value<int>("max_iterations", d_->max_iterations);
}

/// Check that the algorithm's configuration vital::config_block is valid
bool
resection_camera::check_configuration(vital::config_block_sptr config) const
{
  bool good_conf = true;
  double confidence_threshold =
    config->get_value<double>("confidence_threshold", d_->confidence_threshold);

  if( confidence_threshold <= 0.0 || confidence_threshold > 1.0 )
  {
    LOG_ERROR(d_->m_logger, "confidence_threshold parameter is "
                            << confidence_threshold
                            << ", needs to be in (0.0, 1.0].");
    good_conf = false;
  }

  int max_iterations = config->get_value<int>("max_iterations", d_->max_iterations);

  if (max_iterations < 1)
  {
    LOG_ERROR(d_->m_logger, "max iterations is " << max_iterations
      << ", needs to be greater than zero.");
    good_conf = false;
  }
  return good_conf;
}

kwiver::vital::camera_perspective_sptr
resection_camera::resection(
	vector<kwiver::vital::vector_2d> const & pts2d,
    vector<kwiver::vital::vector_3d> const & pts3d,
    kwiver::vital::camera_intrinsics_sptr cal,
    vector<bool>& inliers
) const
{
  if (cal==nullptr)
  {
    LOG_ERROR(d_->m_logger, "camera calibration guess cannot be null");
    return vital::camera_perspective_sptr();
  }
  const unsigned
    minCnt = 3,
    pts2cnt = pts2d.size(),
    pts3cnt = pts3d.size();
  if (pts2cnt < minCnt || pts3cnt < minCnt)
  {
    LOG_ERROR(d_->m_logger, "not enough points to resection camera");
    return vital::camera_perspective_sptr();
  }
  if (pts2cnt != pts3cnt)
    LOG_WARN(d_->m_logger, "counts of 3D points and projections do not match");

  vector<Point2f> projs;
  vector<Point3f> Xs;
  for(const auto & p : pts2d)
    projs.push_back(Point2f(static_cast<float>(p.x()), static_cast<float>(p.y())));
  for(const auto & X : pts3d)
    Xs.push_back(Point3f(static_cast<float>(X.x()),
			 static_cast<float>(X.y()),
			 static_cast<float>(X.z())));

  matrix_3x3d K = cal->as_matrix();
  Mat cv_K; eigen2cv(K, cv_K);
  auto dist_coeffs = get_ocv_dist_coeffs(cal);
  Mat inliers_mat;
  vector<Mat> vrvec, vtvec;

  vector<vector<Point3f>> objPts = {Xs};
  vector<vector<Point2f>> imgPts = {projs};
  Size imgSize(cal->image_width(), cal->image_height());
  int flags = CALIB_USE_INTRINSIC_GUESS;
  const auto err = calibrateCamera(objPts, imgPts, imgSize, cv_K, dist_coeffs, vrvec, vtvec, flags);
  const auto reproj_error = 4.;
  if (err>reproj_error)
    LOG_WARN(d_->m_logger, "estimated re-projection error " << err << " > " << reproj_error << " expected re-projection error");

  Mat rvec=vrvec[0], tvec=vtvec[0];
  vector<Point2f> prjPts; projectPoints(Xs, rvec, tvec, cv_K, dist_coeffs, prjPts);
  auto cnt = Xs.size();
  inliers.resize(cnt);
  while (cnt--) inliers[cnt] = norm(prjPts[cnt]-projs[cnt])<reproj_error;

  auto res_cam = make_shared<simple_camera_perspective>();
  Eigen::Vector3d rvec_eig, tvec_eig;
  auto len = dist_coeffs.size();
  Eigen::VectorXd dist_eig(len);
  while (len--) dist_eig[len]=dist_coeffs[len];
  cv2eigen(rvec, rvec_eig);
  cv2eigen(tvec, tvec_eig);
  cv2eigen(cv_K, K);
  rotation_d rot(rvec_eig);
  res_cam->set_rotation(rot);
  res_cam->set_translation(tvec_eig);
  cal.reset(new simple_camera_intrinsics(K, dist_eig));
  res_cam->set_intrinsics(cal);

  if (!isfinite(res_cam->center().x()))
  {
    LOG_DEBUG(d_->m_logger, "rvec " << rvec.at<double>(0) << " " <<
              rvec.at<double>(1) << " " << rvec.at<double>(2));
    LOG_DEBUG(d_->m_logger, "tvec " << tvec.at<double>(0) << " " <<
              tvec.at<double>(1) << " " << tvec.at<double>(2));
    LOG_DEBUG(d_->m_logger, "rotation angle " << res_cam->rotation().angle());
    LOG_WARN(d_->m_logger, "non-finite camera center found");
    return vital::camera_perspective_sptr();
  }
  return dynamic_pointer_cast<vital::camera_perspective>(res_cam);
}

kwiver::vital::camera_perspective_sptr
resection_camera::resection(kwiver::vital::frame_id_t const & frame,
          kwiver::vital::landmark_map_sptr landmarks,
          kwiver::vital::feature_track_set_sptr tracks,
          kwiver::vital::camera_intrinsics_sptr cal
) const
{
  return vital::algo::resection_camera::resection(frame,landmarks,tracks,cal);
}


} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
