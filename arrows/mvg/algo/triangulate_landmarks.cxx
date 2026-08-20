// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation of MVG triangulate landmarks algorithm
 */

#include "triangulate_landmarks.h"

#include <ctime>
#include <random>
#include <set>

#include <arrows/mvg/metrics.h>
#include <arrows/mvg/triangulate.h>

#include <vital/math_constants.h>
#include <vital/vital_config.h>

namespace kwiver {

namespace arrows {

namespace mvg {

// Private implementation class
class triangulate_landmarks::priv
{
public:
  // Constructor
  priv( triangulate_landmarks& parent )
    : parent( parent )
  {}

  triangulate_landmarks& parent;

  vital::vector_3d
  ransac_triangulation(
    const std::vector< vital::simple_camera_perspective >& lm_cams,
    const std::vector< vital::vector_2d >& lm_image_pts,
    int& best_inlier_count,
    vital::vector_3d const* guess ) const;

  bool
  triangulate(
    const std::vector< vital::simple_camera_perspective >& lm_cams,
    const std::vector< vital::vector_2d >& lm_image_pts,
    vital::vector_3d& pt3d ) const;

  // Configuration values
  bool
  c_homogeneous() const { return parent.c_homogeneous; }
  bool
  c_ransac() const { return parent.c_ransac; }
  float
  c_min_angle_deg() const { return parent.c_min_angle_deg; }
  float
  c_inlier_threshold_pixels() const { return parent.c_inlier_threshold_pixels; }

  float
  c_frac_track_inliers_to_keep_triangulated_point() const
  {
    return parent.c_frac_track_inliers_to_keep_triangulated_point;
  }

  int
  c_max_ransac_samples() const { return parent.c_max_ransac_samples; }
  double
  c_conf_thresh() const { return parent.c_conf_thresh; }
};

void
triangulate_landmarks
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.mvg.triangulate_landmarks" );
}

// Destructor
triangulate_landmarks
::~triangulate_landmarks()
{}

// Check that the algorithm's currently configuration is valid
bool
triangulate_landmarks
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

bool
triangulate_landmarks::priv
::triangulate(
  const std::vector< vital::simple_camera_perspective >& lm_cams,
  const std::vector< vital::vector_2d >& lm_image_pts,
  vital::vector_3d& pt3d ) const
{
  if( c_homogeneous() )
  {
    vital::vector_4d pt4d = triangulate_homog( lm_cams, lm_image_pts );
    if( std::abs( pt4d[ 3 ] ) < 1e-6 )
    {
      pt3d.setZero();
      return false;
    }
    pt3d = pt4d.segment( 0, 3 ) / pt4d[ 3 ];
  }
  else
  {
    pt3d = triangulate_inhomog( lm_cams, lm_image_pts );
  }

  return true;
}

/// Triangulate the landmark with RANSAC robust estimation
vital::vector_3d
triangulate_landmarks::priv
::ransac_triangulation(
  const std::vector< vital::simple_camera_perspective >& lm_cams,
  const std::vector< vital::vector_2d >& lm_image_pts,
  int& best_inlier_count,
  vital::vector_3d const* guess ) const
{
  double conf = 0;
  std::vector< vital::simple_camera_perspective > cam_sample( 2 );
  std::vector< vital::vector_2d > proj_sample( 2 );
  vital::vector_3d best_pt3d;
  best_inlier_count = 0;

  double best_inlier_ratio = 0;

  std::random_device rd;
  std::mt19937_64 gen( rd() );
  std::time_t time_res = std::time( nullptr );
  gen.seed( time_res );

  std::uniform_int_distribution<> dis( 0, int( lm_cams.size() - 1 ) );

  best_pt3d.setZero();

  if( lm_cams.size() < 2 )
  {
    return best_pt3d;
  }

  int s_idx[ 2 ];
  vital::landmark_d lm;
  vital::feature_d f;

  for( int num_samples = 1;
       num_samples <= c_max_ransac_samples() && conf < c_conf_thresh();
       ++num_samples )
  {
    // pick two random points
    int inlier_count = 0;
    s_idx[ 0 ] = dis( gen );
    s_idx[ 1 ] = s_idx[ 0 ];
    while( s_idx[ 0 ] == s_idx[ 1 ] )
    {
      s_idx[ 1 ] = dis( gen );
    }

    cam_sample[ 0 ] = lm_cams[ s_idx[ 0 ] ];
    cam_sample[ 1 ] = lm_cams[ s_idx[ 1 ] ];
    proj_sample[ 0 ] = lm_image_pts[ s_idx[ 0 ] ];
    proj_sample[ 1 ] = lm_image_pts[ s_idx[ 1 ] ];

    vital::vector_3d pt3d;
    if( guess != nullptr && num_samples == 1 )
    {
      pt3d = *guess;
    }
    else
    {
      if( !triangulate( cam_sample, proj_sample, pt3d ) )
      {
        continue;
      }
    }

    lm.set_loc( pt3d );
    // count inliers
    for( size_t idx = 0; idx < lm_cams.size(); ++idx )
    {
      auto depth = lm_cams[ idx ].depth( lm.loc() );
      if( depth <= 0 )
      {
        continue;
      }
      f.set_loc( lm_image_pts[ idx ] );

      double reproj_err_sq = reprojection_error_sqr( lm_cams[ idx ], lm, f );
      if( reproj_err_sq <
          c_inlier_threshold_pixels() * c_inlier_threshold_pixels() )
      {
        ++inlier_count;
      }
    }

    if( inlier_count > best_inlier_count )
    {
      best_inlier_count = inlier_count;
      best_pt3d = pt3d;
      best_inlier_ratio = ( double ) best_inlier_count /
                          ( double ) lm_cams.size();
    }

    conf = 1.0 - pow(
      1.0 - pow( best_inlier_ratio, 2.0 ),
      double( num_samples ) );
    if( lm_cams.size() == 2 )
    {
      break;  // 2 choose 2 only happens one way
    }
  }

  return best_pt3d;
}

void
triangulate_landmarks
::triangulate(
  vital::camera_map_sptr cameras,
  vital::feature_track_set_sptr tracks,
  vital::landmark_map_sptr& landmarks ) const
{
  vital::track_map_t track_map;
  auto tks = tracks->tracks();
  for( auto const& t : tks )
  {
    track_map[ t->id() ] = t;
  }
  triangulate( cameras, track_map, landmarks );
}

// Triangulate the landmark locations given sets of cameras and tracks
void
triangulate_landmarks
::triangulate(
  vital::camera_map_sptr cameras,
  vital::track_map_t track_map,
  vital::landmark_map_sptr& landmarks ) const
{
  using namespace kwiver;
  if( !cameras || !landmarks )
  {
    // TODO throw an exception for missing input data
    return;
  }

  typedef vital::camera_map::map_camera_t map_camera_t;
  typedef vital::landmark_map::map_landmark_t map_landmark_t;

  // extract data from containers
  map_camera_t cams = cameras->cameras();
  map_landmark_t lms = landmarks->landmarks();

  // the set of landmark ids which failed to triangulate
  std::set< vital::landmark_id_t > failed_landmarks;
  std::set< vital::landmark_id_t > failed_outlier, failed_angle;

  // minimum triangulation angle
  double thresh_triang_cos_ang = cos( vital::deg_to_rad * c_min_angle_deg );

  std::vector< vital::simple_camera_perspective > lm_cams;
  std::vector< vital::simple_camera_rpc > lm_cams_rpc;
  std::vector< vital::vector_2d > lm_image_pts;
  std::vector< vital::feature_track_state_sptr > lm_features;

  map_landmark_t triangulated_lms;
  for( const map_landmark_t::value_type& p : lms )
  {
    lm_cams.clear();
    lm_cams_rpc.clear();
    lm_image_pts.clear();
    lm_features.clear();

    // extract the cameras and image points for this landmarks
    size_t lm_observations = 0;

    // get the corresponding track
    vital::track_map_t::const_iterator t_itr = track_map.find( p.first );
    if( t_itr == track_map.end() )
    {
      // there is no track for the provided landmark
      failed_landmarks.insert( p.first );
      continue;
    }

    const vital::track& t = *t_itr->second;

    for( vital::track::history_const_itr tsi = t.begin(); tsi != t.end();
         ++tsi )
    {
      auto fts = std::static_pointer_cast< vital::feature_track_state >( *tsi );
      if( !fts && !fts->feature )
      {
        // there is no valid feature for this track state
        continue;
      }

      map_camera_t::const_iterator c_itr = cams.find( ( *tsi )->frame() );
      if( c_itr == cams.end() )
      {
        // there is no camera for this track state.
        continue;
      }

      auto cam_ptr =
        std::dynamic_pointer_cast< vital::camera_perspective >( c_itr->second );
      if( cam_ptr )
      {
        lm_cams.push_back( vital::simple_camera_perspective( *cam_ptr ) );
      }

      auto rpc_ptr =
        std::dynamic_pointer_cast< vital::camera_rpc >( c_itr->second );
      if( rpc_ptr )
      {
        lm_cams_rpc.push_back( vital::simple_camera_rpc( *rpc_ptr ) );
      }
      if( cam_ptr || rpc_ptr )
      {
        lm_image_pts.push_back( fts->feature->loc() );
        lm_features.push_back( fts );
        ++lm_observations;
      }
    }

    // if we found at least two views of this landmark, triangulate
    if( lm_cams.size() > 1 )
    {
      int inlier_count = 0;
      vital::vector_3d pt3d;
      if( c_ransac )
      {
        vital::vector_3d lm_cur_pt3d = p.second->loc();
        auto triang_guess = &lm_cur_pt3d;
        if( lm_cur_pt3d.x() == 0 && lm_cur_pt3d.y() == 0 &&
            lm_cur_pt3d.z() == 0 )
        {
          triang_guess = nullptr;
        }

        pt3d = d_->ransac_triangulation(
          lm_cams, lm_image_pts, inlier_count,
          triang_guess );
        if( inlier_count <
            lm_image_pts.size() *
            c_frac_track_inliers_to_keep_triangulated_point )
        {
          failed_landmarks.insert( p.first );
          failed_outlier.insert( p.first );
          continue;
        }
      }
      else
      {
        if( !d_->triangulate( lm_cams, lm_image_pts, pt3d ) )
        {
          failed_landmarks.insert( p.first );
          continue;
        }

        // test if the point is behind any of the cameras
        bool behind = false;
        for( auto const& lm_cam : lm_cams )
        {
          auto depth = lm_cam.depth( pt3d );
          if( depth <= 0 )
          {
            behind = true;
            break;
          }
        }
        if( behind )
        {
          for( auto lm_feat : lm_features )
          {
            lm_feat->inlier = false;
          }
          failed_landmarks.insert( p.first );
          continue;
        }
      }

      // set inlier/outlier states for the measurements
      for( size_t idx = 0; idx < lm_cams.size(); ++idx )
      {
        vital::landmark_d lm;
        lm.set_loc( pt3d );

        double reproj_err_sq = reprojection_error_sqr(
          lm_cams[ idx ], lm,
          *lm_features[ idx ]->feature );
        if( reproj_err_sq <
            c_inlier_threshold_pixels * c_inlier_threshold_pixels )
        {
          lm_features[ idx ]->inlier = true;
        }
        else
        {
          lm_features[ idx ]->inlier = false;
        }
      }
      if( !pt3d.allFinite() )
      {
        for( auto lm_feat : lm_features )
        {
          lm_feat->inlier = false;
        }
        failed_landmarks.insert( p.first );
        continue;
      }

      double triang_cos_ang = bundle_angle_max( lm_cams, pt3d );
      bool bad_triangulation = triang_cos_ang > thresh_triang_cos_ang;
      if( bad_triangulation )
      {
        failed_landmarks.insert( p.first );
        failed_angle.insert( p.first );
        for( auto lm_feat : lm_features )
        {
          lm_feat->inlier = false;
        }
        continue;
      }

      std::shared_ptr< vital::landmark_d > lm;
      // if the landmark already exists, copy it
      if( p.second )
      {
        lm = std::make_shared< vital::landmark_d >( *p.second );  // automatically
                                                                  // copies the
                                                                  // tracks_
                                                                  // data
        lm->set_loc( pt3d );
      }
      // otherwise make a new landmark
      else
      {
        lm = std::make_shared< vital::landmark_d >( pt3d );
      }
      lm->set_cos_observation_angle( triang_cos_ang );
      lm->set_observations( lm_observations );
      triangulated_lms[ p.first ] = lm;
    }
    else if( lm_cams_rpc.size() > 1 )
    {
      vital::vector_3d pt3d =
        triangulate_rpc( lm_cams_rpc, lm_image_pts );

      // TODO: is there a way to check for bad triangulations for RPC cameras?
      auto lm = std::make_shared< vital::landmark_d >( *p.second );
      lm->set_loc( pt3d );
      lm->set_observations( lm_observations );
      triangulated_lms[ p.first ] = lm;
    }
  }
  if( !failed_landmarks.empty() )
  {
    LOG_WARN(
      logger(),
      "failed to triangulate " << failed_landmarks.size()
                               << " with " << failed_angle.size() <<
        " for angle, "
                               << failed_outlier.size() << " outliers" );
  }
  landmarks =
    vital::landmark_map_sptr(
      new vital::simple_landmark_map(
        triangulated_lms ) );
}

} // end namespace mvg

} // end namespace arrows

} // end namespace kwiver
