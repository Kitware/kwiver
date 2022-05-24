// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL homography estimation algorithm

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

  estimate_homography();
  ~estimate_homography() override;

  /// Get this algorithm's \link vital::config_block configuration block
  /// \endlink.
  vital::config_block_sptr get_configuration() const override;
  /// Set this algorithm's properties via a config block.
  void set_configuration( vital::config_block_sptr config ) override;
  /// Check that the algorithm's currently configuration is valid.
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Estimate a homography matrix from corresponding points
  ///
  /// If estimation fails, a NULL-containing sptr is returned
  ///
  /// \param [in]  pts1 the vector or corresponding points from the source image
  /// \param [in]  pts2 the vector of corresponding points from the destination image
  /// \param [out] inliers for each point pair, the value is true if
  ///                      this pair is an inlier to the homography estimate
  /// \param [in]  inlier_scale error distance tolerated for matches to be inliers
  vital::homography_sptr
  estimate(const std::vector<vital::vector_2d>& pts1,
           const std::vector<vital::vector_2d>& pts2,
           std::vector<bool>& inliers,
           double inlier_scale = 1.0) const override;
  using vital::algo::estimate_homography::estimate;

private:
  class priv;
  std::unique_ptr< priv > const d;

};

} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver

#endif
