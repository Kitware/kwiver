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

/**
 * \file
 * \brief projected_track_set implementation
 */

#include "projected_track_set.h"

#include <vital/types/feature.h>

namespace kwiver {
namespace arrows {

using namespace kwiver::vital;

/// Use the cameras to project the landmarks back into their images.
feature_track_set_sptr
projected_tracks(landmark_map_sptr landmarks, camera_map_sptr cameras)
{
  std::vector<track_sptr> tracks;

  camera_map::map_camera_t cam_map = cameras->cameras();
  landmark_map::map_landmark_t lm_map = landmarks->landmarks();

  for( landmark_map::map_landmark_t::iterator l = lm_map.begin(); l != lm_map.end(); l++ )
  {
    track_sptr t = track::create();
    t->set_id( l->first );
    tracks.push_back( t );

    for( const camera_map::map_camera_t::value_type& p : cam_map )
    {
      const camera_sptr cam = p.second;
      auto fts = std::make_shared<feature_track_state>(p.first);
      fts->feature = std::make_shared<feature_d>( cam->project( l->second->loc() ) );
      fts->inlier = true;
      t->append( fts );
    }
  }
  return std::make_shared<feature_track_set>( tracks );
}


} // end namespace arrows
} // end namespace kwiver
