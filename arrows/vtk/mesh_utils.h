// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VTK/Vital mesh conversion utility functions.

#ifndef KWIVER_ARROWS_VTK_MESH_UTILS_H_
#define KWIVER_ARROWS_VTK_MESH_UTILS_H_

#include <arrows/vtk/kwiver_algo_vtk_export.h>

#include <vital/types/mesh.h>
#include <vital/types/mesh_container.h>

#include <vtkPolyData.h>

namespace kwiver {

namespace arrows {

namespace vtk {

/// Convert a vtk polydata mesh to a vital mesh.
KWIVER_ALGO_VTK_EXPORT
kwiver::vital::mesh
vtk_to_vital( vtkSmartPointer< vtkPolyData > const& mesh );

/// Convert a vital mesh to a vtk polydata mesh.
KWIVER_ALGO_VTK_EXPORT
vtkSmartPointer< vtkPolyData >
vital_to_vtk( kwiver::vital::mesh const& mesh );

/// Convert a vital mesh container to a vtk polydata mesh.
///
/// If the container is already a vtk::mesh_container, returns its underlying
/// vtkPolyData without copying. Otherwise, converts it.
KWIVER_ALGO_VTK_EXPORT
vtkSmartPointer< vtkPolyData >
container_to_polydata( kwiver::vital::mesh_container const& container );

} // namespace vtk

} // namespace arrows

} // namespace kwiver

#endif
