/*ckwg +29
 * Copyright 2013-2016, 2019-2020 by Kitware, Inc.
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
 * \brief VXL version of similarity transform estimation
 */

#ifndef KWIVER_ARROWS_VXL_ESTIMATE_SIMILARITY_TRANSFORM_H_
#define KWIVER_ARROWS_VXL_ESTIMATE_SIMILARITY_TRANSFORM_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/estimate_similarity_transform.h>
#include <vital/types/vector.h>

namespace kwiver {
namespace arrows {
namespace vxl {

/// VXL implementation of similarity transform estimation
class KWIVER_ALGO_VXL_EXPORT estimate_similarity_transform
  : public vital::algo::estimate_similarity_transform
{
public:
  PLUGIN_INFO( "vxl",
               "Use VXL (vpgl) to estimate a 3D similarity transformation "
               "between corresponding landmarks." )

  // No custom configuration at this time
  /// \cond Doxygen Suppress
  virtual void set_configuration(vital::config_block_sptr /*config*/) { };
  virtual bool check_configuration(vital::config_block_sptr /*config*/) const { return true; }
  /// \endcond

  /// Estimate the similarity transform between two corresponding point sets
  /**
   * \param from List of length N of 3D points in the from space.
   * \param to   List of length N of 3D points in the to space.
   * \throws algorithm_exception When the from and to points sets are
   *                             misaligned, insufficient or degenerate.
   * \returns An estimated similarity transform mapping 3D points in the
   *          \c from space to points in the \c to space (i.e. transforms
   *          \c from into \c to).
   */
  virtual vital::similarity_d
  estimate_transform(std::vector<vital::vector_3d> const& from,
                     std::vector<vital::vector_3d> const& to) const;
  using vital::algo::estimate_similarity_transform::estimate_transform;

};


} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver

#endif
