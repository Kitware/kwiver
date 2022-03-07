// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Definition for PDAL point cloud writer
 */

#ifndef KWIVER_ARROWS_PDAL_POINTCLOUD_IO_H_
#define KWIVER_ARROWS_PDAL_POINTCLOUD_IO_H_

#include <arrows/pdal/kwiver_algo_pdal_export.h>

#include <vital/types/local_geo_cs.h>
#include <vital/types/landmark_map.h>

namespace kwiver {
namespace arrows {
namespace pdal {

/// Write landmarks to a file with PDAL
KWIVER_ALGO_PDAL_EXPORT
void
save_point_cloud_las(vital::path_t const& filename,
        vital::local_geo_cs const& lgcs,
        vital::landmark_map_sptr const& landmarks);

/// Write point cloud to a file with PDAL
KWIVER_ALGO_PDAL_EXPORT
void
save_point_cloud_las(vital::path_t const& filename,
        vital::local_geo_cs const& lgcs,
        std::vector<vital::vector_3d> const& points,
        std::vector<vital::rgb_color> const& colors = {});

} // end namespace pdal
} // end namespace arrows
} // end namespace kwiver

#endif // KWIVER_ARROWS_PDAL_POINTCLOUD_IO_H_
