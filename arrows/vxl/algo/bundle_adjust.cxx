// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of VXL bundle adjustment algorithm

#include "bundle_adjust.h"

#include <iostream>
#include <set>

#include <arrows/vxl/camera_map.h>
#include <vital/io/eigen_io.h>
#include <vital/util/cpu_timer.h>
#include <vital/vital_config.h>

#include <vpgl/algo/vpgl_bundle_adjust.h>

using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace vxl {

/// Private implementation class
class bundle_adjust::priv
{
public:
  /// Constructor
  priv( bundle_adjust& parent ) : parent{ parent } {}

  bundle_adjust& parent;

  /// the vxl sparse bundle adjustor
  vpgl_bundle_adjust ba;
  // vpgl_bundle_adjust does not currently allow accessors for parameters,
  // so we need to cache the parameters here.
  bool
  c_verbose() const { return parent.c_verbose; }
  bool
  c_use_m_estimator() const { return parent.c_use_m_estimator; }
  double
  c_m_estimater_scale() const { return parent.c_m_estimator_scale; }
  bool
  c_estimate_focal_length() const { return parent.c_estimate_focal_length; }
  bool
  c_normalize_data() const { return parent.c_normalize_data; }
  unsigned
  c_max_iterations() const { return parent.c_max_iterations; }
  double
  c_x_tolerance() const { return parent.c_x_tolerance; }
  double
  c_g_tolerance() const { return parent.c_g_tolerance; }
};

void
bundle_adjust
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.bundle_adjust" );
}

/// Check that the algorithm's currently configuration is valid
bool
bundle_adjust
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

/// Optimize the camera and landmark parameters given a set of feature tracks
void
bundle_adjust
::optimize(
  camera_map_sptr& cameras,
  landmark_map_sptr& landmarks,
  feature_track_set_sptr tracks,
  sfm_constraints_sptr constraints ) const
{
  if( !cameras || !landmarks || !tracks )
  {
    // TODO throw an exception for missing input data
    return;
  }
  if( constraints && constraints->get_metadata()->size() > 0 )
  {
    LOG_WARN(
      logger(), "constraints provided but will be ignored "
                "by this algorithm" );
  }
  typedef vxl::camera_map::map_vcam_t map_vcam_t;
  typedef vital::landmark_map::map_landmark_t map_landmark_t;

#define SBA_TIMED( msg, code )                               \
do                                                           \
{                                                            \
  kwiver::vital::cpu_timer t;                                \
  if( d->c_verbose() )                                       \
  {                                                          \
    t.start();                                               \
    LOG_DEBUG(logger(), msg << " ... " );                    \
  }                                                          \
  code                                                       \
  if( d->c_verbose() )                                       \
  {                                                          \
    t.stop();                                                \
    LOG_DEBUG(logger(), " --> " << t.elapsed() << "s CPU" ); \
  }                                                          \
} while( false )

  // extract data from containers
  map_vcam_t vcams = camera_map_to_vpgl( *cameras );
  map_landmark_t lms = landmarks->landmarks();
  std::vector< track_sptr > trks = tracks->tracks();

  //
  // Find the set of all frame numbers containing a camera and track data
  //

  // convidience set of all lm IDs that the active cameras view
  std::set< track_id_t > lm_ids;

  // Nested relation of frame number to a map of track IDs to feature of the
  // track on that frame
  typedef std::map< track_id_t, feature_sptr > super_map_inner_t;
  typedef std::map< frame_id_t, super_map_inner_t > super_map_t;
  super_map_t frame2track2feature_map;

  SBA_TIMED(
    "Constructing id-map and super-map",
    for( const map_vcam_t::value_type& p : vcams )
        {
          const frame_id_t& frame = p.first;
          auto ftracks = tracks->active_tracks( static_cast< int >( frame ) );
          if( ftracks.empty() )
          {
            continue;
          }
          super_map_inner_t frame_lm2feature_map;

          for( const track_sptr& ft : ftracks )
          {
            const track_id_t id = ft->id();
            // make sure the track id has an associated landmark

            if( lms.find( id ) != lms.end() )
            {
              auto fts = std::dynamic_pointer_cast< feature_track_state >(
                *ft->find( frame ) );
              if( fts && fts->feature )
              {
                frame_lm2feature_map[ id ] = fts->feature;
                lm_ids.insert( id );
              }
            }
          }

          if( !frame_lm2feature_map.empty() )
          {
            frame2track2feature_map[ frame ] = frame_lm2feature_map;
          }
        } );

  //
  // Create a compact set of data to optimize,
  // with mapping back to original indices
  //

  // -> landmark mappings
  std::vector< track_id_t > lm_idindex;
  std::map< track_id_t, frame_id_t > lm_idreverse_map;
  std::vector< vgl_point_3d< double > > active_worldpts;
  // -> camera mappings
  std::vector< frame_id_t > cam_idindex;
  std::map< frame_id_t, frame_id_t > cam_idreverse_map;
  std::vector< vpgl_perspective_camera< double > > active_vcams;

  SBA_TIMED(
    "Creating index mappings",
    for( const track_id_t& id : lm_ids )
        {
          lm_idreverse_map[ id ] =
            static_cast< track_id_t >( lm_idindex.size() );
          lm_idindex.push_back( id );
          vector_3d pt = lms[ id ]->loc();
          active_worldpts.push_back(
            vgl_point_3d< double >(
              pt.x(), pt.y(),
              pt.z() ) );
        }
    for( const super_map_t::value_type& p : frame2track2feature_map )
        {
          cam_idreverse_map[ p.first ] =
            static_cast< frame_id_t >( cam_idindex.size() );
          cam_idindex.push_back( p.first );
          active_vcams.push_back( vcams[ p.first ] );
        } );

  // Construct the camera/landmark visibility matrix
  std::vector< std::vector< bool > >
  mask( active_vcams.size(),
    std::vector< bool >( active_worldpts.size(), false ) );
  // Analogous 2D matrix of the track state (feature) location for a given
  // camera/landmark pair
  std::vector< std::vector< feature_sptr > >
  feature_mask( active_vcams.size(),
    std::vector< feature_sptr >( active_worldpts.size(), feature_sptr() ) );
  // compact location vector
  std::vector< vgl_point_2d< double > > image_pts;

  SBA_TIMED(
    "Creating masks and point vector",
    for( const super_map_t::value_type& p : frame2track2feature_map )
        {
          // p.first  -> frame ID
          // p.second -> super_map_inner_t
          const frame_id_t c_idx = cam_idreverse_map[ p.first ];
          std::vector< bool >& mask_row = mask[ c_idx ];
          std::vector< feature_sptr >& fmask_row = feature_mask[ c_idx ];
          for( const super_map_inner_t::value_type& q : p.second )
          {
            // q.first  -> lm ID
            // q.second -> feature_sptr
            mask_row[ lm_idreverse_map[ q.first ] ] = true;
            fmask_row[ lm_idreverse_map[ q.first ] ] = q.second;
          }
        }
    // Populate the vector of observations in the correct order using mask
    // matrices
    vector_2d t_loc;
    for( size_t i = 0; i < active_vcams.size(); ++i )
        {
          for( size_t j = 0; j < active_worldpts.size(); ++j )
          {
            if( mask[ i ][ j ] )
            {
              t_loc = feature_mask[ i ][ j ]->loc();
              image_pts.push_back(
                vgl_point_2d< double >(
                  t_loc.x(),
                  t_loc.y() ) );
            }
          }
        } );

  // Run the vpgl bundle adjustment on the selected data
  SBA_TIMED(
    "VXL bundle optimization",
    d->ba.optimize( active_vcams, active_worldpts, image_pts, mask );
  );

  // map optimized results back into vital structures
  SBA_TIMED(
    "Mapping optimized results back to VITAL structures",
    for( size_t i = 0; i < active_vcams.size(); ++i )
        {
          vcams[ cam_idindex[ i ] ] = active_vcams[ i ];
        }
    for( size_t i = 0; i < active_worldpts.size(); ++i )
        {
          const vgl_point_3d< double >& pt = active_worldpts[ i ];
          vector_3d loc( pt.x(), pt.y(), pt.z() );
          // Cloning here so we don't change the landmarks contained in the
          // input
          // map.
          landmark_sptr lm = lms[ lm_idindex[ i ] ]->clone();
          lms[ lm_idindex[ i ] ] = lm;
          if( landmark_d* lmd = dynamic_cast< landmark_d* >( lm.get() ) )
          {
            lmd->set_loc( loc );
          }
          else if( landmark_f* lmf = dynamic_cast< landmark_f* >( lm.get() ) )
          {
            lmf->set_loc( loc.cast< float >() );
          }
        }
    cameras = camera_map_sptr( new camera_map( vcams ) );
    landmarks = landmark_map_sptr( new simple_landmark_map( lms ) );
  );

#undef SBA_TIMED
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
