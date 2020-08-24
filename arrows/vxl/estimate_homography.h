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
 * \brief VXL homography estimation algorithm
 */

#ifndef KWIVER_ARROWS_VXL_ESTIMATE_HOMOGRAPHY_H_
#define KWIVER_ARROWS_VXL_ESTIMATE_HOMOGRAPHY_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/estimate_homography.h>

namespace kwiver {
namespace arrows {
namespace vxl {

/// A class that uses RREL in VXL to estimate a homography from matching 2D points
class KWIVER_ALGO_VXL_EXPORT estimate_homography
  : public vital::algo::estimate_homography
{
public:
  PLUGIN_INFO( "vxl",
               "Use VXL (rrel) to robustly estimate a homography from matched features." )

  // No configuration yet for this class.
  /// \cond DoxygenSuppress
  virtual void set_configuration(vital::config_block_sptr /*config*/) {}
  virtual bool check_configuration(vital::config_block_sptr /*config*/) const { return true; }
  /// \endcond

  /// Estimate a homography matrix from corresponding points
  /**
   * If estimation fails, a NULL-containing sptr is returned
   *
   * \param [in]  pts1 the vector or corresponding points from the source image
   * \param [in]  pts2 the vector of corresponding points from the destination image
   * \param [out] inliers for each point pair, the value is true if
   *                      this pair is an inlier to the homography estimate
   * \param [in]  inlier_scale error distance tolerated for matches to be inliers
   */
  virtual vital::homography_sptr
  estimate(const std::vector<vital::vector_2d>& pts1,
           const std::vector<vital::vector_2d>& pts2,
           std::vector<bool>& inliers,
           double inlier_scale = 1.0) const;
  using vital::algo::estimate_homography::estimate;

};


} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver


#endif
