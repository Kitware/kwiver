// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief kd_tree adaptor that applies a filter to the features
 */

#ifndef KWIVER_ARROWS_VXL_KD_TREE_H_
#define KWIVER_ARROWS_VXL_KD_TREE_H_

#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/algo/nearest_neighbors.h>

namespace kwiver {

namespace arrows {

namespace vxl {

/// A feature detector that applies a filter to results
class KWIVER_ALGO_VXL_EXPORT kd_tree
  : public kwiver::vital::algo::nearest_neighbors
{
public:
  PLUGIN_INFO( "vxl_kd_tree",
               "KD Tree search to find nearest points." )

  /// Constructor
  kd_tree();

  /// Destructor
  virtual ~kd_tree();

  /// Get this algorithm's \link vital::config_block configuration block
  /// \endlink
  virtual vital::config_block_sptr get_configuration() const;

  /// Set this algorithm's properties via a config block
  virtual void set_configuration( vital::config_block_sptr config );

  /// Check that the algorithm's currently configuration is valid
  virtual bool check_configuration( vital::config_block_sptr config ) const;

  /// Build the search tree
  ///
  /// /param [in] points the set of points to build the search tree from
  virtual void build( std::vector< vital::point_3d >& points ) const;

  /// Find the K nearest neighbors for target point against
  /// the points in the search tree.
  ///
  /// /param [in] point the point to search against
  /// /param [in] K the number of nearest points for each point
  /// /param [out] indices the indices for the K nearest points
  /// /param [out] distances the distance to each nearest point
  virtual void find_nearest_point(
    vital::point_3d& point,
    int K,
    std::vector< int >& indices,
    std::vector< double >& distances ) const;

  /// Find the K nearest neighbors for multiple target points against
  /// the points in the search tree.
  ///
  /// /param [in] vec the points to search against
  /// /param [in] K the number of nearest points for each point
  /// /param [out] indices the indices for the K nearest points
  /// /param [out] distances the distance to each nearest point
  virtual void find_nearest_points(
    std::vector< vital::point_3d >& vec,
    int K,
    std::vector< std::vector< int > >& indices,
    std::vector< std::vector< double > >& distances ) const;

  /// Find all the nearest neighbors within a radius of a target point
  /// against the points in the search tree.
  ///
  /// /param [in] point the points to search against
  /// /param [in] r the radius of the sphere to search over
  /// /param [out] indices the indices for the nearest points
  virtual void find_within_radius(
    vital::point_3d& point,
    double r,
    std::vector< int >& indices ) const;

private:

  /// private implementation class
  class priv;

  const std::unique_ptr< priv > d_;
};

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver

#endif // KWIVER_ARROWS_VXL_KD_TREE_H_
