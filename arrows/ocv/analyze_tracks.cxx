// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of OpenCV analyze tracks algorithm

#include "analyze_tracks.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <set>
#include <vector>

#include <arrows/core/track_set_impl.h>
#include <vital/vital_config.h>

#include <opencv2/core/core.hpp>
using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv {

/// Private implementation class
class analyze_tracks::priv
{
public:

  /// Constructor
  priv()
  : output_summary(true),
    output_pt_matrix(true),
    frames_to_compare({1, 5, 10, 50})
  {
  }

  /// Destructor
  ~priv()
  {
  }

  /// Text output parameters
  bool output_summary;
  bool output_pt_matrix;
  std::vector<int> frames_to_compare;
};

/// Constructor
analyze_tracks
::analyze_tracks()
: d_(new priv)
{
}

/// Destructor
analyze_tracks
::~analyze_tracks()
{
}

/// Get this algorithm's \link vital::config_block configuration block \endlink
vital::config_block_sptr
analyze_tracks
::get_configuration() const
{
  vital::config_block_sptr config = vital::algo::analyze_tracks::get_configuration();

  config->set_value("output_summary", d_->output_summary,
                    "Output a summary descriptor of high-level properties.");
  config->set_value("output_pt_matrix", d_->output_pt_matrix,
                    "Output a matrix showing details about the percentage of "
                    "features tracked for every frame, from each frame to "
                    "some list of frames in the past.");

  std::stringstream ss;
  if ( !d_->frames_to_compare.empty() )
  {
    ss << d_->frames_to_compare[0];
    for(unsigned i=1; i < d_->frames_to_compare.size(); ++i)
    {
      ss << ", " << d_->frames_to_compare[i];
    }
  }
  config->set_value("frames_to_compare", ss.str(),
                    "A comma seperated list of frame difference intervals we want "
                    "to use for the pt matrix. For example, if \"1, 4\" the pt "
                    "matrix will contain comparisons between the current frame and "
                    "last frame in addition to four frames ago.");

  return config;
}

/// Set this algorithm's properties via a config block
void
analyze_tracks
::set_configuration(vital::config_block_sptr in_config)
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  d_->output_summary = config->get_value<bool>( "output_summary" );
  d_->output_pt_matrix = config->get_value<bool>( "output_pt_matrix" );

  std::string ftc = config->get_value<std::string>( "frames_to_compare" );

  std::stringstream ss(ftc);
  d_->frames_to_compare.clear();

  int next_int;

  while (ss >> next_int)
  {
    d_->frames_to_compare.push_back(next_int);

    if (ss.peek() == ',')
    {
      ss.ignore();
    }
  }
}

/// Check that the algorithm's currently configuration is valid
bool
analyze_tracks
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

/// Output various information about the tracks stored in the input set
void
analyze_tracks
::print_info(track_set_sptr track_set,
             stream_t& stream) const
{
  // Early exist case
  if( !d_->output_pt_matrix && !d_->output_summary )
  {
    return;
  }

  // Convert this track set to one with a frame-indexed implementation,
  // which is much more efficient for the operations below
  typedef std::unique_ptr<track_set_implementation> tsi_uptr;
  track_set = std::make_shared<vital::track_set>(
                tsi_uptr(new core::frame_index_track_set_impl(track_set->tracks()) ) );

  // Constants
  const unsigned num_tracks = static_cast<unsigned>(track_set->size());
  const frame_id_t first_frame = track_set->first_frame();
  const frame_id_t last_frame = track_set->last_frame();
  const frame_id_t total_frames = last_frame - first_frame + 1;

  // Output percent tracked matrix
  if( d_->output_pt_matrix )
  {
    stream << std::endl;
    stream << "        Percent of Features Tracked Matrix         " << std::endl;
    stream << "---------------------------------------------------" << std::endl;
    stream << "(FrameID) (NumTrks) (%TrkFromID ";

    for( unsigned i = 0; i < d_->frames_to_compare.size(); i++ )
    {
      stream << " -" << d_->frames_to_compare[i];
    }

    stream << ")" << std::endl;
    stream << std::endl;
  }

  // Generate matrix
  cv::Mat_<double> data( static_cast<int>(total_frames), static_cast<int>(d_->frames_to_compare.size()) + 2 );

  for( frame_id_t fid = first_frame; fid <= last_frame; fid++ )
  {
    data.at<double>( static_cast<int>(fid), 0 ) = static_cast<double>(fid);
    data.at<double>( static_cast<int>(fid), 1 ) = static_cast<double>(track_set->active_tracks( fid ).size());

    for( unsigned i = 0; i < d_->frames_to_compare.size(); i++ )
    {
      int adj = d_->frames_to_compare[ i ];

      if( fid < first_frame + adj )
      {
        data.at<double>( static_cast<int>(fid), i+2 ) = -1.0;
      }
      else
      {
        data.at<double>( static_cast<int>(fid), i+2 ) = track_set->percentage_tracked( fid-adj, fid );
      }
    }
  }

  // Output matrix if enabled
  if( d_->output_pt_matrix )
  {
    stream << data << std::endl;
  }

  // Output number of tracks in stream
  if( d_->output_summary )
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
