// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV resection_camera algorithm implementation

#include "resection_camera.h"

#include "camera_intrinsics.h"

#include <arrows/mvg/camera_options.h>

#include <vital/range/iota.h>

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/eigen.hpp>

namespace kvr = kwiver::vital::range;

namespace kwiver {

namespace arrows {

namespace ocv {

// ----------------------------------------------------------------------------
using vectorf = std::vector< float >;

struct resection_camera::priv : public mvg::camera_options
{

  priv() : m_logger{ vital::get_logger( "arrows.ocv.resection_camera" ) }
  {
  }

  vital::logger_handle_t m_logger;
  // leave enogh of margin for inliers
  double reproj_accuracy = 16.0;
  // maximum number of iterations for camera calibration
  int max_iterations = 32;
  // focal length scales to optimize f*scale over
  vectorf focal_scales { 1 };
};

std::ostream & operator<<(std::ostream & s, vectorf const & v)
{
  for (unsigned i=0, n=v.size(); i<n; ++i)
  {
    if (i>0) s << ' ';
    s << v[i];
  }
  return s;
}

std::istream & operator>>(std::istream & s, vectorf & v)
{
  while (!s.eof())
  {
    float a = 0;
	s >> a;
	v.push_back(a);
  }
  return s;
}

// ----------------------------------------------------------------------------
resection_camera
::resection_camera()
  : d_{ new priv }
{
}

// ----------------------------------------------------------------------------
resection_camera::~resection_camera()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
resection_camera
::get_configuration() const
{
  vital::config_block_sptr config =
    vital::algo::resection_camera::get_configuration();
  d_->get_configuration( config );
  config->set_value( "reproj_accuracy", d_->reproj_accuracy,
                     "desired re-projection positive accuracy for inlier points" );
  config->set_value( "max_iterations", d_->max_iterations,
                     "maximum number of iterations to run optimization [1, INT_MAX]" );
  std::stringstream ss;
  ss << d_->focal_scales;
  config->set_value( "focal_scales", ss.str(),
                     "focal length scales to optimize f*scale over" );
  return config;
}

// ----------------------------------------------------------------------------
void
resection_camera
::set_configuration( vital::config_block_sptr config )
{
  d_->set_configuration( config );
  d_->reproj_accuracy = config->get_value< double >( "reproj_accuracy",
                                                     d_->reproj_accuracy );
  d_->max_iterations = config->get_value< int >( "max_iterations",
                                                 d_->max_iterations );
  std::stringstream ss( config->get_value< std::string > ( "focal_scales", "1" ) );
  d_->focal_scales.clear();
  ss >> d_->focal_scales;
}

// ----------------------------------------------------------------------------
bool
resection_camera
::check_configuration( vital::config_block_sptr config ) const
{
  auto const reproj_accuracy =
    config->get_value< double >( "reproj_accuracy", d_->reproj_accuracy );
  if( reproj_accuracy <= 0.0 )
  {
    LOG_ERROR( d_->m_logger,
               "reproj_accuracy parameter is " << reproj_accuracy <<
               ", but needs to be positive." );
    return false;
  }

  auto const max_iterations =
    config->get_value< int >( "max_iterations", d_->max_iterations );
  if( max_iterations < 1 )
  {
    LOG_ERROR( d_->m_logger,
               "max iterations is " << max_iterations <<
               ", needs to be greater than zero." );
    return false;
  }

  std::stringstream ss( config->get_value< std::string > ( "focal_scales", "1" ) );
  vectorf focal_scales;
  ss >> focal_scales;
  auto m = std::min_element( focal_scales.begin(), focal_scales.end() );
  if( m == focal_scales.end() || *m <= 0 )
  {
    LOG_ERROR( d_->m_logger,
               "focal_scales: " << focal_scales <<
               ", needs to be greater than zero." );
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
vital::camera_perspective_sptr
resection_camera
::resection(
  std::vector< kwiver::vital::vector_2d > const& image_points,
  std::vector< kwiver::vital::vector_3d > const& world_points,
  vital::camera_intrinsics_sptr cal,
  std::vector< bool >* inliers ) const
{
  if( cal == nullptr )
  {
    LOG_ERROR( d_->m_logger, "camera calibration guess should not be null" );
    return nullptr;
  }

  auto const point_count = image_points.size();
  constexpr size_t min_count = 3;
  if( point_count < min_count )
  {
    LOG_ERROR( d_->m_logger,
               "camera resection needs at least " << min_count << " points, "
               "but only " << point_count << " were provided" );
    return nullptr;
  }

  auto const wpoint_count = world_points.size();
  if( point_count != wpoint_count )
  {
    LOG_WARN( d_->m_logger,
              "counts of 3D points (" << wpoint_count << ") and "
              "their projections (" << point_count << ") do not match" );
  }

  std::vector< cv::Point2f > cv_image_points;
  cv_image_points.reserve( point_count );
  for( auto const& p : image_points )
  {
    cv_image_points.emplace_back( p.x(), p.y() );
  }

  std::vector< cv::Point3f > cv_world_points;
  cv_world_points.reserve( point_count );
  for( const auto& p : world_points )
  {
    cv_world_points.emplace_back( p.x(), p.y(), p.z() );
  }

  cv::Mat inliers_mat;
  using vmat = std::vector< cv::Mat >;

  vmat vrvec, vtvec;
  auto const world_points_vec =
    std::vector< std::vector< cv::Point3f > >{ cv_world_points };
  auto const image_points_vec =
    std::vector< std::vector< cv::Point2f > >{ cv_image_points };
  auto const image_size =
    cv::Size{ static_cast< int >( cal->image_width() ),
              static_cast< int >( cal->image_height() ) };
  auto dist_coeffs = get_ocv_dist_coeffs( cal );
  int flags = cv::CALIB_USE_INTRINSIC_GUESS;
  if( !d_->optimize_focal_length )
  {
    flags |= cv::CALIB_FIX_FOCAL_LENGTH;
  }
  if( !d_->optimize_aspect_ratio )
  {
    flags |= cv::CALIB_FIX_ASPECT_RATIO;
  }
  if( !d_->optimize_principal_point )
  {
    flags |= cv::CALIB_FIX_PRINCIPAL_POINT;
  }
  if( !d_->optimize_dist_k1 )
  {
    flags |= cv::CALIB_FIX_K1;
  }
  if( !d_->optimize_dist_k2 )
  {
    flags |= cv::CALIB_FIX_K2;
  }
  if( !d_->optimize_dist_k3 )
  {
    flags |= cv::CALIB_FIX_K3;
  }
  if( !d_->optimize_dist_p1_p2 )
  {
    flags |= cv::CALIB_ZERO_TANGENT_DIST;
  }
  if( d_->optimize_dist_k4_k5_k6 )
  {
    flags |= cv::CALIB_RATIONAL_MODEL;
  }
  else
  {
    flags |= cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5 | cv::CALIB_FIX_K6;
  }

  vital::matrix_3x3d K = cal->as_matrix();
  cv::TermCriteria term_criteria{
    cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
    d_->max_iterations, DBL_EPSILON };
  using MatD = cv::Mat_< double >;

  MatD cv_K;
  eigen2cv( K, cv_K );

  auto const dc0 = dist_coeffs;
  // focal scale search parameter for optimization
  auto focal_scale = 1.0;
  // minimize re-projection error over multiple focal scales
  auto err = std::numeric_limits< double >::infinity();
  for( auto const scale : d_->focal_scales )
  {
    auto dc = dc0;
    vmat rv, tv;
    MatD cvK;
    eigen2cv( K, cvK );
    cvK( 0, 0 ) *= scale;
    cvK( 1, 1 ) *= scale;

    auto const e = cv::calibrateCamera(
      world_points_vec, image_points_vec,
      image_size, cvK, dc, rv, tv,
      flags, term_criteria );
    if( e < err && fabs(e-err) > DBL_EPSILON )
    {
      cv_K = cvK;
      dist_coeffs = dc;
      vrvec = rv;
      vtvec = tv;
      focal_scale = scale;
      err = e;
    }
  }
  LOG_DEBUG( d_->m_logger, "re-projection error=" << err <<
             ", focal scale=" << focal_scale );

  auto const reproj_error = d_->reproj_accuracy;
  if( err > reproj_error )
  {
    LOG_WARN( d_->m_logger, "estimated re-projection error " <<
              err << " exceeds expected re-projection error " <<
              reproj_error );
  }

  cv::Mat rvec = vrvec[ 0 ];
  cv::Mat tvec = vtvec[ 0 ];

  if( inliers )
  {
    std::vector< cv::Point2f > projected_points;
    projectPoints( cv_world_points, rvec, tvec, cv_K,
                   dist_coeffs, projected_points );

    inliers->resize( point_count );
    for( auto const i : kvr::iota( point_count ) )
    {
      auto const delta = norm( projected_points[ i ] - cv_image_points[ i ] );
      ( *inliers )[ i ] = ( delta < reproj_error );
    }
  }

  auto res_cam = std::make_shared< vital::simple_camera_perspective >();
  Eigen::Vector3d rvec_eig, tvec_eig;
  auto const dc_size = dist_coeffs.size();

  Eigen::VectorXd dist_eig( dist_coeffs.size() );
  for( auto const i : kvr::iota( dc_size ) )
  {
    dist_eig[ static_cast< int >( i ) ] = dist_coeffs[ i ];
  }
  cv::cv2eigen( rvec, rvec_eig );
  cv::cv2eigen( tvec, tvec_eig );
  cv::cv2eigen( cv_K, K );

  vital::rotation_d rot{ rvec_eig };
  res_cam->set_rotation( rot );
  res_cam->set_translation( tvec_eig );
  cal = std::make_shared< vital::simple_camera_intrinsics >( K, dist_eig );
  res_cam->set_intrinsics( cal );

  if( !res_cam->center().allFinite() )
  {
    LOG_DEBUG( d_->m_logger, "rvec " << rvec.at< double >( 0 ) << " " <<
               rvec.at< double >( 1 ) << " " << rvec.at< double >( 2 ) );
    LOG_DEBUG( d_->m_logger, "tvec " << tvec.at< double >( 0 ) << " " <<
               tvec.at< double >( 1 ) << " " << tvec.at< double >( 2 ) );
    LOG_DEBUG( d_->m_logger,
               "rotation angle " << res_cam->rotation().angle() );
    LOG_WARN( d_->m_logger, "non-finite camera center found" );
    return nullptr;
  }
  return res_cam;
}

} // namespace ocv

} // namespace arrows

} // namespace kwiver
