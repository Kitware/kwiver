/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#ifndef VITAL_ALGO_INTERPOLATE_TRACK_H
#define VITAL_ALGO_INTERPOLATE_TRACK_H

#include <vital/vital_config.h>
#include <vital/algo/algorithm.h>


namespace kwiver {
namespace vital {
namespace algo {

class VITAL_ALGO_EXPORT interpolate_track
  : public kwiver::vital::algorithm_def<interpolate_track>
{
public:
  /// Return the name of this algorithm.
  static std::string static_type_name() { return "interpolate_track"; }

  /// This method interpolates the states between track states.
  /**
   * This method interpolates track states to fill in missing states
   * between the states supplied in the input parameter. An output
   * track is created that contains all states between the first and
   * last state in the intpu track.
   *
   * @param init_states List of states to interpolate between.
   *
   * @return Output track with missing states filled in.
   */
  virtual object_track& interpolate( const object_track& init_states ) = 0;

  /// Return fraction of task completed.
  /**
   * This method returns a value between 0 and 1.0 indicating the
   * fraction of the current task that has been completed. This is
   * designed to be used by a GUI to maintain a progress bar.
   *
   * @return Fraction of task completed.
   */
  virtual float progress const();

protected:
  interpolate_track();

};

} } } // end namespace

#endif /* VITAL_ALGO_INTERPOLATE_TRACK_H */
