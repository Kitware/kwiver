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

#ifndef VITAL_ALGO_CLOSE_LOOPS_H_
#define VITAL_ALGO_CLOSE_LOOPS_H_

#include <vital/vital_config.h>

#include <vital/algo/algorithm.h>
#include <vital/types/image_container.h>
#include <vital/types/feature_track_set.h>

#include <ostream>

/**
 * \file
 * \brief Header defining abstract \link kwiver::vital::algo::close_loops
 *        close_loops \endlink algorithm
 */

namespace kwiver {
namespace vital {
namespace algo {

/// \brief Abstract base class for loop closure algorithms.
/**
 * Different algorithms can perform loop closure in a variety of ways, either
 * in attempt to make either short or long term closures. Similarly to
 * track_features, this class is designed to be called in an online fashion.
 */
class VITAL_ALGO_EXPORT close_loops
  : public kwiver::vital::algorithm_def<close_loops>
{
public:

  /// Return the name of this algorithm.
  static std::string static_type_name() { return "close_loops"; }

  /// Attempt to perform closure operation and stitch tracks together.
  /**
   * \param frame_number the frame number of the current frame
   * \param input the input feature track set to stitch
   * \param image image data for the current frame
   * \param mask Optional mask image where positive values indicate
   *                  regions to consider in the input image.
   * \returns an updated set of feature tracks after the stitching operation
   */
  virtual kwiver::vital::feature_track_set_sptr
  stitch( kwiver::vital::frame_id_t frame_number,
          kwiver::vital::feature_track_set_sptr input,
          kwiver::vital::image_container_sptr image,
          kwiver::vital::image_container_sptr mask = kwiver::vital::image_container_sptr()) const = 0;

protected:
  close_loops();

};

typedef std::shared_ptr<close_loops> close_loops_sptr;

} } } // end namespace

#endif // VITAL_ALGO_CLOSE_LOOPS_H_
