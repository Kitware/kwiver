#ifndef KWIVER_ARROWS_CORE_RENDER_MESH_DEPTH_MAP_H
#define KWIVER_ARROWS_CORE_RENDER_MESH_DEPTH_MAP_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/types/camera.h>
#include <vital/types/image_container.h>
#include <vital/types/mesh.h>

namespace kwiver {
namespace arrows {


/// This function renders a depthmap of a mesh seen by a camera
/**
 * @brief render_mesh_depth_map
 * @param mesh [in]
 * @param camera [in]
 * @return a depth map
 */
KWIVER_ALGO_CORE_EXPORT
vital::image_container_sptr render_mesh_depth_map(kwiver::vital::mesh_sptr mesh, kwiver::vital::camera_sptr camera);

}
}
#endif // KWIVER_ARROWS_CORE_RENDER_MESH_DEPTH_MAP_H
