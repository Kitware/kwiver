// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of vtkTriangleFilter algorithm

#include <arrows/vtk/algo/triangulate_mesh_filter.h>
#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>
#include <vital/exceptions.h>
#include <vtkTriangleFilter.h>

namespace kwiver {

namespace arrows {

namespace vtk {

void
triangulate_mesh_filter
::initialize()
{
  attach_logger( "arrows.vtk.triangulate_mesh_filter" );
}

triangulate_mesh_filter::
~triangulate_mesh_filter() {}

vital::mesh_container_sptr
triangulate_mesh_filter
::filter( vital::mesh_container_sptr input )
{
  if( !input )
  {
    VITAL_THROW( vital::invalid_value, "Null input mesh" );
  }

  vtkSmartPointer< vtkPolyData > mesh = container_to_polydata( *input );

  auto triangulator = vtkSmartPointer< vtkTriangleFilter >::New();
  triangulator->SetInputData( mesh );
  triangulator->Update();

  return std::make_shared< mesh_container >( triangulator->GetOutput() );
}

} // namespace vtk

} // namespace arrows

} // namespace kwiver
