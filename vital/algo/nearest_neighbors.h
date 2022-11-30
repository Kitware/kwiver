// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief nearest_neighbors algorithm definition

#ifndef VITAL_ALGO_NEAREST_NEIGHBORS_H_
#define VITAL_ALGO_NEAREST_NEIGHBORS_H_

#include <vital/algo/algorithm.h>
#include <vital/vital_config.h>

#include <vital/types/point.h>

namespace kwiver {

namespace vital {

namespace algo {

/// An abstract base class for detecting feature points
class VITAL_ALGO_EXPORT nearest_neighbors
  : public kwiver::vital::algorithm_def< nearest_neighbors >
{
public:
  /// Return the name of this algorithm
  static std::string static_type_name() { return "nearest_neighbors"; }

  /// Build the search tree
  ///
  /// /param [in] points the set of points to build the search tree from
  virtual void build( std::vector< point_3d >& points ) const = 0;

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
    std::vector< double >& distances ) const = 0;

  /// Find the K nearest neighbors for target points against
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
    std::vector< std::vector< double > >& distances ) const = 0;

  /// Find all the nearest neighbors within a radius of a target point
  /// against the points in the search tree.
  ///
  /// /param [in] point the points to search against
  /// /param [in] r the radius of the sphere to search over
  /// /param [out] indices the indices for the nearest points
  virtual void find_within_radius(
    vital::point_3d& point,
    double r,
    std::vector< int >& indices ) const = 0;

protected:
  nearest_neighbors();
};

/// Shared pointer for nearest_neighbors algorithm definition class
typedef std::shared_ptr< nearest_neighbors > nearest_neighbors_sptr;

} // namespace algo

} // namespace vital

} // namespace kwiver

#endif // VITAL_ALGO_NEAREST_NEIGHBORS_H_
