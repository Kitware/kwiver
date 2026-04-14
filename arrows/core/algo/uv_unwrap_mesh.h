// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for mesh uv unwrapping

#ifndef KWIVER_ARROWS_CORE_UV_UNWRAP_MESH_H
#define KWIVER_ARROWS_CORE_UV_UNWRAP_MESH_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/uv_unwrap_mesh.h>
#include <vital/types/mesh.h>
#include <vital/vital_config.h>

#include <vital/algo/algorithm.txx>

#include "vital/plugin_management/pluggable_macro_magic.h"

namespace kwiver {

namespace arrows {

namespace core {

/// A class for unwrapping a mesh and generating texture coordinates
class KWIVER_ALGO_CORE_EXPORT uv_unwrap_mesh
  : public vital::algo::uv_unwrap_mesh
{
public:
  PLUGGABLE_IMPL(
    uv_unwrap_mesh,
    "A class for unwrapping a mesh and generating texture coordinates. ",
    PARAM_DEFAULT(
      spacing, double,
      "Spacing between triangles as a fraction of the texture size. "
      "Should be in (0.0, 1.0].",
      0.005 ),
    PARAM_DEFAULT(
      sort_descending, bool,
      "Sort triangles from largest to smallest area before packing. "
      "Set to false to use ascending sort (legacy behavior).",
      true ),
    PARAM_DEFAULT(
      compact, bool,
      "Use compact triangle packing by alternating 180-degree rotations "
      "so adjacent triangles interlock. Set to false for simple row "
      "packing (legacy behavior).",
      true ),
    PARAM_DEFAULT(
      padding_ratio, double,
      "When compact packing is enabled, fraction of the margin used as "
      "horizontal padding between adjacent triangles. Must be in (0.0, 1.0].",
      1.0 ),
    PARAM_DEFAULT(
      iterations, int,
      "When compact packing is enabled, number of simulation iterations "
      "used to find an efficient texture atlas width. Must be >= 1.",
      10 )
  )

  /// Destructor
  virtual ~uv_unwrap_mesh();

  /// Check configuration
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Unwrap a mesh and generate texture coordinate
  ///
  /// \param mesh [in/out]
  void unwrap( kwiver::vital::mesh_sptr mesh ) const override;

private:
  void initialize() override;
  /// private implementation class
  class priv;
  KWIVER_UNIQUE_PTR( priv, d_ );
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif // KWIVER_ARROWS_CORE_UV_UNWRAP_MESH_H
