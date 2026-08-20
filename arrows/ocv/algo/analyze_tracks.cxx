// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of OpenCV analyze tracks algorithm

#include "analyze_tracks.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

#include <arrows/core/track_set_impl.h>
#include <vital/vital_config.h>

#include <opencv2/core/core.hpp>
using namespace kwiver::vital;

namespace kwiver {

namespace arrows {

namespace ocv {

namespace {

std::vector< int >
frames_to_compare_vec( std::string frames_to_compare_str )
{
  std::vector< int > value;
  std::stringstream ss( frames_to_compare_str );
  int next_int;

  while( ss >> next_int )
  {
    value.push_back( next_int );
    if( ss.peek() == ',' )
    {
      ss.ignore();
    }
  }
  return value;
}

} // namespace

/// Destructor
analyze_tracks
::~analyze_tracks()
{}

/// Check that the algorithm's currently configuration is valid
bool
analyze_tracks
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

/// Output various information about the tracks stored in the input set
void
analyze_tracks
::print_info(
  track_set_sptr track_set,
  stream_t& stream ) const
{
  // Early exist case
  if( !this->get_output_pt_matrix() && !this->get_output_summary() )
  {
    return;
  }

  // Convert this track set to one with a frame-indexed implementation,
  // which is much more efficient for the operations below
  typedef std::unique_ptr< track_set_implementation > tsi_uptr;
  track_set = std::make_shared< vital::track_set >(
    tsi_uptr( new core::frame_index_track_set_impl( track_set->tracks() ) ) );

  // Constants
  auto const num_tracks = track_set->size();
  const frame_id_t first_frame = track_set->first_frame();
  const frame_id_t last_frame = track_set->last_frame();
  const frame_id_t total_frames = last_frame - first_frame + 1;

  auto frames_to_compare =
    frames_to_compare_vec( this->get_frames_to_compare() );
  // Output percent tracked matrix
  if( this->get_output_pt_matrix() )
  {
    stream << std::endl;
    stream << "        Percent of Features Tracked Matrix         " <<
      std::endl;
    stream << "---------------------------------------------------" <<
      std::endl;
    stream << "(FrameID) (NumTrks) (%TrkFromID ";

    for( size_t i = 0; i < frames_to_compare.size(); i++ )
    {
      stream << " -" << frames_to_compare[ i ];
    }

    stream << ")" << std::endl;
    stream << std::endl;
  }

  // Generate matrix
  cv::Mat_< double > data( static_cast< int >( total_frames ),
    static_cast< int >( frames_to_compare.size() ) + 2 );

  for( frame_id_t fid = first_frame; fid <= last_frame; fid++ )
  {
    data.at< double >(
      static_cast< int >( fid ),
      0 ) = static_cast< double >( fid );
    data.at< double >(
      static_cast< int >( fid ),
      1 ) = static_cast< double >( track_set->active_tracks( fid ).size() );

    for( size_t i = 0; i < frames_to_compare.size(); i++ )
    {
      int adj = frames_to_compare[ i ];

      if( fid < first_frame + adj )
      {
        data.at< double >( static_cast< int >( fid ), i + 2 ) = -1.0;
      }
      else
      {
        data.at< double >(
          static_cast< int >( fid ),
          i + 2 ) = track_set->percentage_tracked( fid - adj, fid );
      }
    }
  }

  // Output matrix if enabled
  if( this->get_output_pt_matrix() )
  {
    stream << data << std::endl;
  }

  // Output number of tracks in stream
  if( this->get_output_summary() )
  {
    stream << std::endl;
    stream << "Track Set Properties" << std::endl;
    stream << "--------------------" << std::endl;
    stream << std::endl;
    stream << "Largest Track ID: " << num_tracks << std::endl;
    stream << "Smallest Frame ID: " << first_frame << std::endl;
    stream << "Largest Frame ID: " << last_frame << std::endl;
    stream << std::endl;
  }
}

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver
