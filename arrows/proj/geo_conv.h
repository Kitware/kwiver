// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief PROJ geo_conversion functor interface
 */

#ifndef KWIVER_ARROWS_PROJ_GEO_CONV_H_
#define KWIVER_ARROWS_PROJ_GEO_CONV_H_

#include <arrows/proj/kwiver_algo_proj_export.h>

#include <vital/types/geodesy.h>

#include <vital/vital_config.h>

#include <unordered_map>

namespace kwiver {

namespace arrows {

namespace proj {

/// PROJ implementation of geo_conversion functor
class KWIVER_ALGO_PROJ_EXPORT geo_conversion
  : public vital::geo_conversion
{
public:
  char const* id() const override;

  vital::geo_crs_description_t describe( int crs ) override;

  /// Conversion operator
  vital::vector_2d operator()(
    vital::vector_2d const& point, int from, int to ) override;

  /// Conversion operator
  vital::vector_3d operator()(
    vital::vector_3d const& point, int from, int to ) override;
};

} // namespace proj

} // namespace arrows

} // namespace kwiver

#endif
