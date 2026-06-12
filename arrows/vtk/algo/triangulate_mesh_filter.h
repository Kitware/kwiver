// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Header for vtkTriangleFilter algorithm

#ifndef KWIVER_ARROWS_VTK_TRIANGULATE_MESH_FILTER_H_
#define KWIVER_ARROWS_VTK_TRIANGULATE_MESH_FILTER_H_

#include <arrows/vtk/kwiver_algo_vtk_export.h>

#include <vital/algo/algorithm.txx>
#include <vital/algo/mesh_filter.h>
#include <vital/plugin_management/pluggable_macro_magic.h>

namespace kwiver {

namespace arrows {

namespace vtk {

class KWIVER_ALGO_VTK_EXPORT triangulate_mesh_filter
  : public vital::algo::mesh_filter
{
public:
  PLUGGABLE_IMPL(
    triangulate_mesh_filter,
    "Use vtkTriangleFilter algorithm to triangulate mesh."
  )

  virtual ~triangulate_mesh_filter();

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
