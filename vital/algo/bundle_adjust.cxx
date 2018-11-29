/*ckwg +29
 * Copyright 2014-2015 by Kitware, Inc.
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
 * \brief Instantiation of \link kwiver::vital::algo::algorithm_def algorithm_def<T>
 *        \endlink for \link kwiver::vital::algo::bundle_adjust bundle_adjust \endlink
 */

#include <vital/algo/bundle_adjust.h>
#include <vital/algo/algorithm.txx>
#include <vital/logger/logger.h>

namespace kwiver {
namespace vital {
namespace algo {

bundle_adjust
::bundle_adjust()
{
  attach_logger( "algo.bundle_adjust" );
}

/// Set a callback function to report intermediate progress
void
bundle_adjust
::set_callback(callback_t cb)
{
  this->m_callback = cb;
}

void
bundle_adjust
::optimize(vital::camera_map_sptr& cameras,
           vital::landmark_map_sptr& landmarks,
           vital::feature_track_set_sptr tracks,
           const std::set<vital::frame_id_t>& fixed_cameras,
           const std::set<vital::landmark_id_t>& fixed_landmarks,
           vital::metadata_map_sptr metadata) const
{
  if (!fixed_cameras.empty() || !fixed_landmarks.empty())
  {
    LOG_WARN(logger(), "This implementation does not support fixing cameras or landmarks");
  }
  optimize(cameras, landmarks, tracks, metadata);
}

} } }

/// \cond DoxygenSuppress
INSTANTIATE_ALGORITHM_DEF(kwiver::vital::algo::bundle_adjust);
/// \endcond
