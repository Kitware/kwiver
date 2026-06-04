// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL match_features_constrained algorithm impl interface

#ifndef KWIVER_ARROWS_VXL_MATCH_FEATURES_CONSTRAINED_H_
#define KWIVER_ARROWS_VXL_MATCH_FEATURES_CONSTRAINED_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/match_features.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// A match_feature algorithm that uses feature position, orientation, and scale
/// constraints
///
///  This matching algorithm assumes that the features to be matched are already
///  somewhat well aligned geometrically.  The use cases are very similar images
///  (e.g. adjacent frames of video) and features that have been transformed
///  into approximate alignment by a pre-processing step
///  (e.g. image registration)
///
///  This algorithm first reduces the search space for each feature using a
///  search radius in the space of location (and optionally orientation and
///  scale) to find only geometrically nearby features.  It then looks at
///  the descriptors for the neighbors and finds the best match by appearance.
class KWIVER_ALGO_VXL_EXPORT match_features_constrained
  : public vital::algo::match_features
{
public:
  PLUGGABLE_IMPL(
    match_features_constrained,
    "Use VXL to match descriptors under the constraints of similar geometry "
    "(rotation, scale, position).",
    PARAM_DEFAULT(
      scale_thresh, double,
      "Ratio threshold of scales between matching keypoints (>=1.0)"
      " -1 turns scale thresholding off",
      2.0 ),
    PARAM_DEFAULT(
      angle_thresh, double,
      "Angle difference threshold between matching keypoints"
      " -1 turns angle thresholding off",
      -1.0 ),
    PARAM_DEFAULT(
      radius_thresh, double,
      "Search radius for a match in pixels",
      200.0 )
  )

  /// Destructor
  virtual ~match_features_constrained() = default;

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Match one set of features and corresponding descriptors to another
  ///
  /// \param [in] feat1 the first set of features to match
  /// \param [in] desc1 the descriptors corresponding to \a feat1
  /// \param [in] feat2 the second set fof features to match
  /// \param [in] desc2 the descriptors corresponding to \a feat2
  /// \returns a set of matching indices from \a feat1 to \a feat2
  virtual vital::match_set_sptr
  match(
    vital::feature_set_sptr feat1, vital::descriptor_set_sptr desc1,
    vital::feature_set_sptr feat2, vital::descriptor_set_sptr desc2 ) const;

private:
  void initialize() override;
  /// private implementation class
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif
