// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief core mesh_container interface

#ifndef VITAL_MESH_CONTAINER_H_
#define VITAL_MESH_CONTAINER_H_

#include <vital/vital_config.h>

#include <vital/types/mesh.h>
#include <vital/types/metadata.h>

namespace kwiver {

namespace vital {

/// An abstract representation of a mesh container.
///
/// This class provides an interface for passing mesh data
/// between algorithms.  It is intended to be a wrapper for mesh
/// classes in third-party libraries and facilitate conversion between
/// various representations.  It provides limited access to the underlying
/// data and is not intended for direct use in mesh processing algorithms.
class mesh_container
{
public:
  /// Destructor
  virtual ~mesh_container() = default;

  /// The number of vertices in the mesh
  virtual size_t num_verts() const = 0;

  /// The number of faces in the mesh
  virtual size_t num_faces() const = 0;

  /// Get an in-memory mesh class to access the data
  virtual mesh_sptr get_mesh() const = 0;

  /// Get the texture coordinate status for the mesh
  virtual mesh::tex_coord_type has_tex_coords() const = 0;

  /// Get the texture coordinates for the mesh
  virtual std::vector< vector_2d > const& tex_coords() const = 0;

  /// Set the texture coordinates for the mesh
  virtual void set_tex_coords( std::vector< vector_2d > const& tc ) = 0;
};

/// Shared pointer for base mesh_container type
using mesh_container_sptr = std::shared_ptr< mesh_container >;
using mesh_container_scptr = std::shared_ptr< mesh_container const >;

// ----------------------------------------------------------------------------
/// This concrete mesh container is simply a wrapper around a mesh
class simple_mesh_container
  : public mesh_container
{
public:
  /// Constructor
  explicit simple_mesh_container( const mesh_sptr d ) : data( d ) {}

  /// The number of vertices in the mesh
  virtual size_t
  num_verts() const { return data->num_verts(); }

  /// The number of faces in the mesh
  virtual size_t
  num_faces() const { return data->num_faces(); }

  /// Get an in-memory mesh class to access the data
  virtual mesh_sptr
  get_mesh() const { return data; }

  /// Get the texture coordinate status for the mesh
  virtual mesh::tex_coord_type
  has_tex_coords() const
  {
    return data->has_tex_coords();
  }

  /// Get the texture coordinates for the mesh
  virtual std::vector< vector_2d > const&
  tex_coords() const
  {
    return data->tex_coords();
  }

  /// Set the texture coordinates for the mesh
  virtual void
  set_tex_coords( std::vector< vector_2d > const& tc )
  {
    data->set_tex_coords( tc );
  }

protected:
  /// data for this mesh container
  mesh_sptr data;
};

} // namespace vital

}   // end namespace vital

#endif // VITAL_MESH_CONTAINER_H_
