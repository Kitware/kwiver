/*ckwg +29
 * Copyright 2013-2017 by Kitware, Inc.
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
 * \brief Header defining abstract \link kwiver::vital::algo::track_features feature
 *        tracking \endlink algorithm
 */

#ifndef VITAL_ALGO_TRACK_FEATURES_H_
#define VITAL_ALGO_TRACK_FEATURES_H_


#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/feature_track_set.h>

namespace kwiver {
namespace vital {
namespace algo {

/// An abstract base class for tracking feature points
class VITAL_ALGO_EXPORT track_features
  : public algorithm_def<track_features>
{
public:

  /// Return the name of this algorithm
  static std::string static_type_name() { return "track_features"; }

  /// Extend a previous set of feature tracks using the current frame
  /**
   * \throws image_size_mismatch_exception
   *    When the given non-zero mask image does not match the size of the
   *    dimensions of the given image data.
   *
   * \param [in] prev_tracks the feature tracks from previous tracking steps
   * \param [in] frame_number the frame number of the current frame
   * \param [in] image_data the image pixels for the current frame
   * \param [in] mask Optional mask image that uses positive values to denote
   *                  regions of the input image to consider for feature
   *                  tracking. An empty sptr indicates no mask (default
   *                  value).
   * \returns an updated set of feature tracks including the current frame
   */
  virtual feature_track_set_sptr
  track(feature_track_set_sptr prev_tracks,
        unsigned int frame_number,
        image_container_sptr image_data,
        image_container_sptr mask = image_container_sptr()) const = 0;

protected:
    track_features();

};


/// Shared pointer for generic track_features definition type.
typedef std::shared_ptr<track_features> track_features_sptr;

} } } // end namespace

#endif // VITAL_ALGO_TRACK_FEATURES_H_
