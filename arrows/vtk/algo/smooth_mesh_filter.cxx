// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of vtkSmoothPolyDataFilter algorithm

#include <arrows/vtk/algo/smooth_mesh_filter.h>
#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>
#include <vital/exceptions.h>
#include <vtkSmoothPolyDataFilter.h>

namespace kwiver {

namespace arrows {

namespace vtk {

// ----------------------------------------------------------------------------
void
smooth_mesh_filter
::initialize()
{
  attach_logger( "arrows.vtk.smooth_mesh_filter" );
}

// ----------------------------------------------------------------------------
smooth_mesh_filter
::~smooth_mesh_filter()
{}

// ----------------------------------------------------------------------------
bool
smooth_mesh_filter
::check_configuration( vital::config_block_sptr config ) const
{
  double convergence =
    config->get_value< double >( "convergence", c_convergence );
  if( convergence < 0.0 || convergence > 1.0 )
  {
    return false;
  }

  int number_of_iterations =
    config->get_value< int >( "number_of_iterations", c_number_of_iterations );
  if( number_of_iterations < 0 )
  {
    return false;
  }

  double feature_angle =
    config->get_value< double >( "feature_angle", c_feature_angle );
  if( feature_angle < 0.0 || feature_angle > 180.0 )
  {
    return false;
  }

  double edge_angle =
    config->get_value< double >( "edge_angle", c_edge_angle );
  if( edge_angle < 0.0 || edge_angle > 180.0 )
  {
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
vital::mesh_container_sptr
smooth_mesh_filter
::filter( vital::mesh_container_sptr input )
{
  if( !input )
  {
    VITAL_THROW( vital::invalid_value, "Null input mesh" );
  }

  vtkSmartPointer< vtkPolyData > mesh = container_to_polydata( *input );

  auto smoother = vtkSmartPointer< vtkSmoothPolyDataFilter >::New();
  smoother->SetInputData( mesh );
  smoother->SetConvergence( c_convergence );
  smoother->SetNumberOfIterations( c_number_of_iterations );
  smoother->SetRelaxationFactor( c_relaxation_factor );
  smoother->SetFeatureEdgeSmoothing( c_feature_edge_smoothing );
  smoother->SetFeatureAngle( c_feature_angle );
  smoother->SetEdgeAngle( c_edge_angle );
  smoother->SetBoundarySmoothing( c_boundary_smoothing );
  smoother->SetGenerateErrorScalars( c_generate_error_scalars );
  smoother->SetGenerateErrorVectors( c_generate_error_vectors );
  smoother->SetOutputPointsPrecision( c_output_points_precision );
  smoother->Update();

  return std::make_shared< mesh_container >( smoother->GetOutput() );
}

} // namespace vtk

} // namespace arrows

} // namespace kwiver
