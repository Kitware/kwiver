// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Header for vtkDecimatePro mesh filtering algorithm

#ifndef KWIVER_ARROWS_VTK_DECIMATE_MESH_FILTER_H_
#define KWIVER_ARROWS_VTK_DECIMATE_MESH_FILTER_H_

#include <arrows/vtk/kwiver_algo_vtk_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/mesh_filter.h>
#include <vital/plugin_management/pluggable_macro_magic.h>

#include <vtkType.h>

namespace kwiver {

namespace arrows {

namespace vtk {

class KWIVER_ALGO_VTK_EXPORT decimate_mesh_filter
  : public vital::algo::mesh_filter
{
public:
  PLUGGABLE_IMPL(
    decimate_mesh_filter,
    "Use VTK's DecimatePro algorithm to decimate mesh.",
    PARAM_DEFAULT(
      target_reduction, double,
      "Specify the desired reduction in the total number of polygons "
      "(e.g., if TargetReduction is set to 0.9, this filter will try to "
      "reduce the data set to 10% of its original size).",
      0.9 ),
    PARAM_DEFAULT(
      preserve_topology, bool,
      "Turn on/off whether to preserve the topology of the original mesh. "
      "If on, mesh splitting and hole elimination will not occur.",
      false ),
    PARAM_DEFAULT(
      feature_angle, double,
      "Specify the mesh feature angle. This angle is used to define what "
      "an edge is (i.e., if the surface normal between two adjacent "
      "triangles is >= FeatureAngle, an edge exists).",
      15.0 ),
    PARAM_DEFAULT(
      splitting, bool,
      "Turn on/off the splitting of the mesh at corners, along edges, at "
      "non-manifold points, or anywhere else a split is required.",
      true ),
    PARAM_DEFAULT(
      split_angle, double,
      "Specify the mesh split angle. This angle is used to control the "
      "splitting of the mesh.",
      75.0 ),
    PARAM_DEFAULT(
      pre_split_mesh, bool,
      "In some cases you may wish to split the mesh prior to algorithm "
      "execution. This separates the mesh into semi-planar patches.",
      false ),
    PARAM_DEFAULT(
      maximum_error, double,
      "Set the largest decimation error that is allowed during the "
      "decimation process.",
      VTK_DOUBLE_MAX ),
    PARAM_DEFAULT(
      accumulate_error, bool,
      "The computed error can either be computed directly from the mesh "
      "or the error may be accumulated as the mesh is modified.",
      false ),
    PARAM_DEFAULT(
      error_is_absolute, int,
      "The MaximumError is normally defined as a fraction of the dataset "
      "bounding diagonal. By setting ErrorIsAbsolute to 1, the error is "
      "instead defined as that specified by AbsoluteError.",
      0 ),
    PARAM_DEFAULT(
      absolute_error, double,
      "Same as MaximumError, but to be used when ErrorIsAbsolute is 1.",
      VTK_DOUBLE_MAX ),
    PARAM_DEFAULT(
      boundary_vertex_deletion, bool,
      "Turn on/off the deletion of vertices on the boundary of a mesh.",
      true ),
    PARAM_DEFAULT(
      degree, int,
      "If the number of triangles connected to a vertex exceeds \"Degree\", "
      "then the vertex will be split.",
      25 ),
    PARAM_DEFAULT(
      inflection_point_ratio, double,
      "Specify the inflection point ratio. An inflection point occurs when "
      "the ratio of reduction error between two iterations is >= "
      "InflectionPointRatio.",
      10.0 )
  )

  virtual ~decimate_mesh_filter();

  bool
  check_configuration( vital::config_block_sptr ) const override
  {
    return true;
  }

  vital::mesh_container_sptr
  filter( vital::mesh_container_sptr input ) override;

private:
  void initialize() override;
};

} // namespace vtk

} // namespace arrows

} // namespace kwiver

#endif
