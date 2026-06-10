// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of vtkDecimatePro mesh filtering algorithm

#include <arrows/vtk/algo/decimate_mesh_filter.h>
#include <arrows/vtk/algo/triangulate_mesh_filter.h>
#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>
#include <vital/exceptions.h>
#include <vtkDecimatePro.h>

namespace kwiver {

namespace arrows {

namespace vtk {

// ----------------------------------------------------------------------------
void
decimate_mesh_filter
::initialize()
{
  attach_logger( "arrows.vtk.decimate_mesh_filter" );
}

// ----------------------------------------------------------------------------
decimate_mesh_filter
::~decimate_mesh_filter()
{}

// ----------------------------------------------------------------------------
vital::mesh_container_sptr
decimate_mesh_filter
::filter( vital::mesh_container_sptr input )
{
  if( !input )
  {
    VITAL_THROW( vital::invalid_value, "Null input mesh" );
  }

  // vtkDecimatePro requires triangular input
  triangulate_mesh_filter triangulator;
  vtkSmartPointer< vtkPolyData > mesh =
    container_to_polydata( *triangulator.filter( input ) );

  auto decimator = vtkSmartPointer< vtkDecimatePro >::New();
  decimator->SetInputData( mesh );
  decimator->SetTargetReduction( c_target_reduction );
  decimator->SetFeatureAngle( c_feature_angle );
  decimator->SetPreserveTopology( c_preserve_topology );
  decimator->SetMaximumError( c_maximum_error );
  decimator->SetAbsoluteError( c_absolute_error );
  decimator->SetErrorIsAbsolute( c_error_is_absolute );
  decimator->SetAccumulateError( c_accumulate_error );
  decimator->SetSplitAngle( c_split_angle );
  decimator->SetSplitting( c_splitting );
  decimator->SetPreSplitMesh( c_pre_split_mesh );
  decimator->SetDegree( c_degree );
  decimator->SetBoundaryVertexDeletion( c_boundary_vertex_deletion );
  decimator->SetInflectionPointRatio( c_inflection_point_ratio );
  decimator->Update();

  return std::make_shared< mesh_container >( decimator->GetOutput() );
}

} // namespace vtk

} // namespace arrows

} // namespace kwiver
