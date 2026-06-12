// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Header for vtkSmoothPolyDataFilter algorithm

#ifndef KWIVER_ARROWS_VTK_SMOOTH_MESH_FILTER_H_
#define KWIVER_ARROWS_VTK_SMOOTH_MESH_FILTER_H_

#include <arrows/vtk/kwiver_algo_vtk_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/mesh_filter.h>
#include <vital/plugin_management/pluggable_macro_magic.h>

namespace kwiver {

namespace arrows {

namespace vtk {

class KWIVER_ALGO_VTK_EXPORT smooth_mesh_filter
  : public vital::algo::mesh_filter
{
public:
  PLUGGABLE_IMPL(
    smooth_mesh_filter,
    "Use vtkSmoothPolyDataFilter to apply Laplacian smoothing to a mesh.",
    PARAM_DEFAULT(
      convergence, double,
      "Specify a convergence criterion for the iteration process. Smaller "
      "numbers result in more smoothing iterations.",
      0.0 ),
    PARAM_DEFAULT(
      number_of_iterations, int,
      "Specify the number of iterations for Laplacian smoothing.",
      20 ),
    PARAM_DEFAULT(
      relaxation_factor, double,
      "Specify the relaxation factor for Laplacian smoothing. In general, "
      "small relaxation factors and large numbers of iterations are more "
      "stable than larger relaxation factors and smaller numbers of "
      "iterations.",
      0.01 ),
    PARAM_DEFAULT(
      feature_edge_smoothing, bool,
      "Turn on/off smoothing along sharp interior edges.",
      false ),
    PARAM_DEFAULT(
      feature_angle, double,
      "Specify the feature angle for sharp edge identification.",
      45.0 ),
    PARAM_DEFAULT(
      edge_angle, double,
      "Specify the edge angle to control smoothing along edges (either "
      "interior or boundary).",
      15.0 ),
    PARAM_DEFAULT(
      boundary_smoothing, bool,
      "Turn on/off the smoothing of vertices on the boundary of the mesh.",
      true ),
    PARAM_DEFAULT(
      generate_error_scalars, bool,
      "Turn on/off the generation of scalar distance values.",
      false ),
    PARAM_DEFAULT(
      generate_error_vectors, bool,
      "Turn on/off the generation of error vectors.",
      false ),
    PARAM_DEFAULT(
      output_points_precision, int,
      "Set the desired precision for the output points type.",
      2 )
  )

  virtual ~smooth_mesh_filter();

  bool check_configuration( vital::config_block_sptr config ) const override;

  vital::mesh_container_sptr
  filter( vital::mesh_container_sptr input ) override;

private:
  void initialize() override;
};

} // namespace vtk

} // namespace arrows

} // namespace kwiver

#endif
