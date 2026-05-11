// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VTK mesh container implementation

#ifndef KWIVER_ARROWS_VTK_MESH_CONTAINER_H_
#define KWIVER_ARROWS_VTK_MESH_CONTAINER_H_

#include <arrows/vtk/kwiver_algo_vtk_export.h>
#include <vital/vital_config.h>

#include <arrows/vtk/mesh_utils.h>
#include <vital/types/mesh_container.h>
#include <vtkPolyData.h>

namespace kwiver {

namespace arrows {

namespace vtk {

/// VTK implementation of a mesh container for VTK PolyData representation
class KWIVER_ALGO_VTK_EXPORT mesh_container
  : public vital::mesh_container
{
public:
  /// Generic Mesh Constructor
  mesh_container( vital::mesh const& d )
    : data( vital_to_vtk( d ) ),
      tex_coords_( d.tex_coords() ) {}

  /// vtkPolyData Constructor
  mesh_container( vtkSmartPointer< vtkPolyData > d )
    : data( d ) {}

  /// Converting Constructor
  mesh_container( vital::mesh_container const& other )
    : data( container_to_polydata( other ) ),
      tex_coords_( other.tex_coords() ) {}

  // ----------------------------------------------------------------------------
  /// The number of vertices in the mesh
  virtual size_t
  num_verts() const { return data->GetNumberOfPoints(); }

  /// The number of faces in the mesh
  virtual size_t
  num_faces() const { return data->GetNumberOfPolys(); }

  /// Get an in-memory mesh class to access the data
  virtual vital::mesh
  mesh() const
  {
    vital::mesh m = vtk_to_vital( data );
    m.set_tex_coords( tex_coords_ );
    return m;
  }

  /// Get native vtkPolyData pointer to data
  vtkSmartPointer< vtkPolyData >
  get_data() const { return data; }

  /// Get the texture coordinate status for the mesh
  virtual vital::mesh::tex_coord_type
  has_tex_coords() const
  {
    if( tex_coords_.size() == 0 )
    {
      return vital::mesh::tex_coord_type::TEX_COORD_NONE;
    }
    if( tex_coords_.size() == num_verts() )
    {
      return vital::mesh::tex_coord_type::TEX_COORD_ON_VERT;
    }
    return vital::mesh::tex_coord_type::TEX_COORD_ON_CORNER;
  }

  /// Get the texture coordinates for the mesh
  virtual std::vector< vital::vector_2d >
  tex_coords() const
  {
    return tex_coords_;
  }

  /// Set the texture coordinates for the mesh
  virtual void
  set_tex_coords( std::vector< vital::vector_2d > const& tc )
  {
    tex_coords_ = tc;
  }

protected:
  vtkSmartPointer< vtkPolyData > data;
  std::vector< vital::vector_2d > tex_coords_;
};

using mesh_container_sptr = std::shared_ptr< mesh_container >;

} // namespace vtk

} // namespace arrows

}     // end namespace vtk

#endif // KWIVER_ARROWS_VTK_MESH_CONTAINER_H_
