// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// VTK/Vital mesh conversion utility functions.

#include <arrows/vtk/mesh_container.h>
#include <arrows/vtk/mesh_utils.h>

#include <vtkCellArrayIterator.h>
#include <vtkTriangleStrip.h>

namespace kwiver {

namespace arrows {

namespace vtk {

/// Convert a vtk polydata mesh to a vital mesh.
vital::mesh
vtk_to_vital( vtkSmartPointer< vtkPolyData > const& mesh )
{
  int num_verts = mesh->GetNumberOfPoints();

  auto verts = std::make_unique< vital::mesh_vertex_array< 3 > >();
  std::unique_ptr< vital::mesh_face_array_base > faces =
    std::make_unique< vital::mesh_face_array >();

  // Populate the vertices
  double point[ 3 ];
  for( int v = 0; v < num_verts; ++v )
  {
    mesh->GetPoint( v, point );
    verts->push_back( { point[ 0 ], point[ 1 ], point[ 2 ] } );
  }

  // Populate the faces
  const long long* pts;
  long long num_pts;

  vtkSmartPointer< vtkCellArray > polys = mesh->GetPolys();
  vtkSmartPointer< vtkCellArray > strips = mesh->GetStrips();
  vtkSmartPointer< vtkCellArrayIterator > iter;

  // Decompose triangle strips into triangles
  if( strips->GetNumberOfCells() > 0 )
  {
    polys->DeepCopy( mesh->GetPolys() );
    iter = ::vtk::TakeSmartPointer( strips->NewIterator() );

    for( iter->GoToFirstCell(); !iter->IsDoneWithTraversal();
         iter->GoToNextCell() )
    {
      iter->GetCurrentCell( num_pts, pts );
      vtkTriangleStrip::DecomposeStrip( num_pts, pts, polys );
    }
  }

  // If there are no faces, return
  int num_faces = polys->GetNumberOfCells();
  if( num_faces == 0 )
  {
    return vital::mesh( std::move( verts ), std::move( faces ) );
  }

  iter = ::vtk::TakeSmartPointer( polys->NewIterator() );
  iter->GetCurrentCell( num_pts, pts );

  // Attempt to construct a triangular mesh
  bool triangular = num_pts == 3;
  if( triangular )
  {
    faces = std::make_unique< vital::mesh_regular_face_array< 3 > >();

    auto& faces_ref =
      static_cast< vital::mesh_regular_face_array< 3 >& >( *faces );

    for( iter->GoToFirstCell(); !iter->IsDoneWithTraversal();
         iter->GoToNextCell() )
    {
      iter->GetCurrentCell( num_pts, pts );

      // If not all faces are triangles, default to general case
      if( num_pts != 3 )
      {
        triangular = false;
        break;
      }

      faces_ref.push_back(
        vital::mesh_regular_face< 3 >(
            {
              static_cast< unsigned int >( pts[ 0 ] ),
              static_cast< unsigned int >( pts[ 1 ] ),
              static_cast< unsigned int >( pts[ 2 ] ) } ) );
    }
  }

  // If unable to construct a triangular mesh, construct a general mesh
  if( !triangular )
  {
    faces = std::make_unique< vital::mesh_face_array >();

    auto& faces_ref =
      static_cast< vital::mesh_face_array& >( *faces );

    iter->GoToFirstCell();
    for( int f = 0; f < num_faces && !iter->IsDoneWithTraversal(); ++f )
    {
      iter->GetCurrentCell( num_pts, pts );
      faces_ref.push_back( { pts, pts + num_pts } );
      iter->GoToNextCell();
    }
  }

  return vital::mesh( std::move( verts ), std::move( faces ) );
}

/// Convert a vital mesh to a vtk polydata mesh
vtkSmartPointer< vtkPolyData >
vital_to_vtk( vital::mesh const& mesh )
{
  int num_verts = mesh.num_verts();
  int num_faces = mesh.num_faces();

  auto points = vtkSmartPointer< vtkPoints >::New();
  auto polys = vtkSmartPointer< vtkCellArray >::New();

  vital::mesh_vertex_array_base const& vertices = mesh.vertices();
  vital::mesh_face_array_base const& faces = mesh.faces();

  // Populate the vertices
  for( int v = 0; v < num_verts; ++v )
  {
    double point[ 3 ] = { vertices( v, 0 ), vertices( v, 1 ),
                          vertices( v, 2 ) };
    points->InsertNextPoint( point );
  }

  // Populate the faces
  for( int f = 0; f < num_faces; ++f )
  {
    auto pts = vtkSmartPointer< vtkIdList >::New();
    int num_pts = faces.num_verts( f );

    for( int v = 0; v < num_pts; ++v )
    {
      pts->InsertNextId( faces( f, v ) );
    }
    polys->InsertNextCell( pts );
  }

  auto out = vtkSmartPointer< vtkPolyData >::New();
  out->SetPoints( points );
  out->SetPolys( polys );
  out->BuildCells();
  return out;
}

vtkSmartPointer< vtkPolyData >
container_to_polydata( kwiver::vital::mesh_container const& container )
{
  if( auto vtk_container =
        dynamic_cast< vtk::mesh_container const* >( &container ) )
  {
    return vtk_container->get_data();
  }

  return vital_to_vtk( container.mesh() );
}

} // namespace vtk

} // namespace arrows

} // namespace kwiver
