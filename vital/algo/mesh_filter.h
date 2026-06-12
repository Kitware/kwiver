// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to abstract filter mesh algorithm

#ifndef VITAL_ALGO_MESH_FILTER_H
#define VITAL_ALGO_MESH_FILTER_H

#include <vital/algo/algorithm.h>
#include <vital/vital_config.h>

#include <vital/types/mesh_container.h>

namespace kwiver {

namespace vital {

namespace algo {

/// Abstract base class for mesh filter algorithms.
class VITAL_ALGO_EXPORT mesh_filter
  : public kwiver::vital::algorithm
{
public:
  mesh_filter();
  PLUGGABLE_INTERFACE( mesh_filter );

  /// Filter an input mesh and return resulting mesh.
  ///
  /// This method implements the filtering operation. The method does
  /// not modify the mesh in place. The resulting mesh must be a
  /// newly allocated mesh.
  ///
  /// \param mesh_data Mesh to filter.
  /// \returns a filtered version of the input mesh.
  virtual mesh_container_sptr
  filter( mesh_container_sptr mesh_data ) = 0;
};

typedef std::shared_ptr< mesh_filter > mesh_filter_sptr;

} // namespace algo

} // namespace vital

} // namespace kwiver

#endif
