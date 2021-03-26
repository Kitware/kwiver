// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief OCV resection_camera algorithm implementation
 */

#include "resection_camera.h"
#include "camera_intrinsics.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/eigen.hpp>
#include <cmath>


using namespace std;
using namespace cv;
using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

// ----------------------------------------------------------------------------

struct resection_camera::priv
{
  double reproj_accuracy = 4.;
  int max_iterations = 32;
  vital::logger_handle_t m_logger;

  priv() : m_logger{ vital::get_logger( "arrows.ocv.resection_camera" ) }
  {
  }
};

// ----------------------------------------------------------------------------

resection_camera
::resection_camera()
  : d_{ new priv }
{
}

resection_camera::~resection_camera()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
resection_camera
::get_configuration() const
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

/// Set this algorithm's properties via a config block.
void
resection_camera
::set_configuration( vital::config_block_sptr config )
{
  d_->reproj_accuracy = config->get_value< double >( "reproj_accuracy",
                                                     d_->reproj_accuracy );
  d_->max_iterations = config->get_value< int >( "max_iterations",
                                                 d_->max_iterations );
}

/// Check that the algorithm's configuration vital::config_block is valid.
bool
resection_camera
::check_configuration( vital::config_block_sptr config ) const
{
  bool good_conf = true;
  double reproj_accuracy = config->get_value< double >( "reproj_accuracy",
                                                        d_->reproj_accuracy );
  if( reproj_accuracy <= 0.0 )
  {
    LOG_ERROR( d_->m_logger,
               "reproj_accuracy parameter is " << reproj_accuracy <<
          ", but needs to be positive." );
    good_conf = false;
  }

  int max_iterations = config->get_value<int>("max_iterations", d_->max_iterations);

  if (max_iterations < 1)
  {
    LOG_ERROR( d_->m_logger,
               "max iterations is " << max_iterations <<
          ", needs to be greater than zero." );
    good_conf = false;
  }
  return good_conf;
}

kwiver::vital::camera_perspective_sptr
resection_camera::resection(
	std::vector<kwiver::vital::vector_2d> const & pts2d,
    std::vector<kwiver::vital::vector_3d> const & pts3d,
    kwiver::vital::camera_intrinsics_sptr cal,
    std::vector<bool>& inliers
) const
{
  if( cal == nullptr )
  {
    LOG_ERROR(d_->m_logger, "camera calibration guess cannot be null");
    return vital::camera_perspective_sptr();
  }

  constexpr size_t min_count = 3;
  auto const pts2_count = pts2d.size();
  auto const pts3_count = pts3d.size();
  if( pts2_count < min_count || pts3_count < min_count )
  {
    LOG_ERROR(d_->m_logger, "not enough points to resection camera");
    return vital::camera_perspective_sptr();
  }
  if (pts2cnt != pts3cnt)
    LOG_WARN(d_->m_logger, "counts of 3D points and projections do not match");

  std::vector<cv::Point2f> projs;
  std::vector<cv::Point3f> Xs;
  for(const auto & p : pts2d)
    projs.push_back(cv::Point2f(static_cast<float>(p.x()), static_cast<float>(p.y())));
  for(const auto & X : pts3d)
    Xs.push_back(cv::Point3f(static_cast<float>(X.x()),
                             static_cast<float>(X.y()),
                             static_cast<float>(X.z())));

  vital::matrix_3x3d K = cal->as_matrix();
  cv::Mat cv_K; cv::eigen2cv(K, cv_K);
  std::vector<double> dist_coeffs = get_ocv_dist_coeffs(cal);

  // TODO: initialize for cv::calibrateCamera, e.g. image size, which requires an approximate cal to exist
  const double reproj_error = 4.;
  cv::Mat inliers_mat, rvec, tvec;
  bool success = cv::solvePnPRansac(Xs, projs,
    cv_K, dist_coeffs, rvec, tvec, false,
    d_->max_iterations, reproj_error,
    d_->confidence_threshold, inliers_mat,
    cv::SOLVEPNP_EPNP);

  double inlier_ratio = (inliers_mat.rows / (double)Xs.size());

  if (!success || tvec.rows == 0 || rvec.rows == 0)
  {
    LOG_DEBUG(d_->m_logger, "no PnP solution after " << d_->max_iterations
              << " iterations with confidence " << d_->confidence_threshold
              << " and best inlier ratio " << inlier_ratio );
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
  cnt = dist_coeffs.size();
  Eigen::VectorXd dist_eig(cnt);
  while (cnt--) dist_eig[cnt]=dist_coeffs[cnt];
  cv2eigen(rvec, rvec_eig);
  cv2eigen(tvec, tvec_eig);
  cv2eigen(cv_K, K);
  rotation_d rot(rvec_eig);
  res_cam->set_rotation(rot);
  res_cam->set_translation(tvec_eig);
  res_cam->set_intrinsics(cal); // TODO: set from calibration estimates

  if (!isfinite(res_cam->center().x()))
  {
    dist_eig[ cnt ] = dist_coeffs[ cnt ];
  }
  cv2eigen( rvec, rvec_eig );
  cv2eigen( tvec, tvec_eig );
  cv2eigen( cv_K, K );

  vital::rotation_d rot( rvec_eig );
  res_cam->set_rotation( rot );
  res_cam->set_translation( tvec_eig );
  cal.reset( new vital::simple_camera_intrinsics( K, dist_eig ) );
  res_cam->set_intrinsics( cal );

  auto ctr = res_cam->center();
  if( !isfinite( ctr.x() ) || !isfinite( ctr.y() ) || !isfinite( ctr.z() ) )
  {
    LOG_DEBUG( d_->m_logger, "rvec " << rvec.at< double >( 0 ) << " " <<
               rvec.at< double >( 1 ) << " " << rvec.at< double >( 2 ) );
    LOG_DEBUG( d_->m_logger, "tvec " << tvec.at< double >( 0 ) << " " <<
               tvec.at< double >( 1 ) << " " << tvec.at< double >( 2 ) );
    LOG_DEBUG( d_->m_logger,
               "rotation angle " << res_cam->rotation().angle() );
    LOG_WARN( d_->m_logger, "non-finite camera center found" );
    return vital::camera_perspective_sptr();
  }
  return std::dynamic_pointer_cast<vital::camera_perspective>(res_cam);
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
