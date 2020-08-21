/*ckwg +29
 * Copyright 2014-2020 by Kitware, Inc.
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
 * \brief Header for core camera and landmark initialization algorithm
 */

#ifndef KWIVER_ARROWS_CORE_INITIALIZE_CAMERAS_LANDMARKS_H_
#define KWIVER_ARROWS_CORE_INITIALIZE_CAMERAS_LANDMARKS_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/initialize_cameras_landmarks.h>

namespace kwiver {
namespace arrows {
namespace core {

/// A class for initialization of cameras and landmarks
class KWIVER_ALGO_CORE_EXPORT initialize_cameras_landmarks
: public vital::algo::initialize_cameras_landmarks
{
public:
  PLUGIN_INFO( "core",
               "Run SfM to iteratively estimate new cameras and landmarks"
               " using feature tracks." )

  /// Constructor
  initialize_cameras_landmarks();

  /// Destructor
  virtual ~initialize_cameras_landmarks();

  /// Copy Constructor
  initialize_cameras_landmarks(const initialize_cameras_landmarks& other);

  /// Get this algorithm's \link vital::config_block configuration block \endlink
  virtual vital::config_block_sptr get_configuration() const;
  /// Set this algorithm's properties via a config block
  virtual void set_configuration(vital::config_block_sptr config);
  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration(vital::config_block_sptr config) const;

  /// Initialize the camera and landmark parameters given a set of feature tracks
  /**
   * The algorithm creates an initial estimate of any missing cameras and
   * landmarks using the available cameras, landmarks, and feature tracks.
   * If the input cameras map is a NULL pointer then the algorithm should try
   * to initialize all cameras covered by the track set.  If the input camera
   * map exists then the algorithm should only initialize cameras on frames for
   * which the camera is set to NULL.  Frames not in the map will not be
   * initialized.  This allows the caller to control which subset of cameras to
   * initialize without needing to manipulate the feature tracks.
   * The analogous behavior is also applied to the input landmarks map to
   * select which track IDs should be used to initialize landmarks.
   *
   * \note This algorithm may optionally revise the estimates of existing
   * cameras and landmarks passed as input.
   *
   * \param [in,out] cameras the cameras to initialize
   * \param [in,out] landmarks the landmarks to initialize
   * \param [in] tracks the feature tracks to use as constraints
   * \param [in] metadata the frame metadata to use as constraints
   */
  virtual void
  initialize(vital::camera_map_sptr& cameras,
             vital::landmark_map_sptr& landmarks,
             vital::feature_track_set_sptr tracks,
             vital::sfm_constraints_sptr constraints = nullptr) const;

  /// Set a callback function to report intermediate progress
  virtual void set_callback(callback_t cb);

private:
  /// private implementation class
  class priv;
  const std::unique_ptr<priv> d_;
};

} // end namespace core
} // end namespace arrows
} // end namespace kwiver

#endif
