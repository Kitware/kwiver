#ifndef KWIVER_ARROWS_CORE_COMPUTE_MESH_DEPTH_MAP_H
#define KWIVER_ARROWS_CORE_COMPUTE_MESH_DEPTH_MAP_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/types/camera.h>
#include <vital/types/image_container.h>
#include <vital/types/mesh.h>

namespace kwiver {
namespace arrows {


/// This function computes a depthmap of a mesh seen by a camera
/**
 * @brief compute_mesh_depth_map
 * @param mesh [in]
 * @param camera [in]
 * @return a depth map
 */
KWIVER_ALGO_CORE_EXPORT
std::pair<kwiver::vital::image_container_sptr, kwiver::vital::image_container_sptr>
compute_mesh_depth_map(kwiver::vital::mesh_sptr mesh, kwiver::vital::camera_sptr camera);

}
}
#endif // KWIVER_ARROWS_CORE_COMPUTE_MESH_DEPTH_MAP_H
