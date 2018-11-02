#include "compute_mesh_depth_map.h"

#include <vital/types/camera_affine.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/image.h>
#include <vital/types/vector.h>


namespace {

/// Helper function to check if a point is inside a triangle
bool is_point_inside_triangle(const kwiver::vital::vector_2d& p,
                              const kwiver::vital::vector_2d& a,
                              const kwiver::vital::vector_2d& b,
                              const kwiver::vital::vector_2d& c)
{
  kwiver::vital::vector_2d AB = b - a;
  kwiver::vital::vector_2d AC = c - a;
  kwiver::vital::vector_2d AP = p - a;
  double inv_total_area = 1.0 / (AB[0] * AC[1] - AB[1] * AC[0]);
  double area_1 = inv_total_area * (AB[0] * AP[1] - AB[1] * AP[0]);
  double area_2 = inv_total_area * (AP[0] * AC[1] - AP[1] * AC[0]);
  return area_1 >= 0 && area_2 >= 0 && (area_1 + area_2) <= 1;
}

/// Helper function to compute barycentric coordinates of p w.r.t the triangle abc
kwiver::vital::vector_3d barycentric_coordinates(const kwiver::vital::vector_2d& p,
                                                 const kwiver::vital::vector_2d& a,
                                                 const kwiver::vital::vector_2d& b,
                                                 const kwiver::vital::vector_2d& c)
{
  kwiver::vital::matrix_3x3d abc;
  abc << a, b, c, 1, 1, 1;
  double det_inv = 1.0 / abc.determinant();
  kwiver::vital::vector_3d res;
  res(0) = ((b(1) - c(1)) * (p(0) - c(0)) - (b(0) - c(0)) * (p(1) - c(1))) * det_inv;
  res(1) = ((c(1) - a(1)) * (p(0) - c(0)) - (c(0) - a(0)) * (p(1) - c(1))) * det_inv;
  res(2) = 1.0 - res(0) - res(1);
  return res;
}

}

namespace kwiver {
namespace arrows {

vital::image_container_sptr compute_mesh_depth_map(vital::mesh_sptr mesh, vital::camera_sptr camera)
{
  unsigned int nb_vertices = mesh->num_verts();
  int width = static_cast<int>(camera->image_width());
  int height = static_cast<int>(camera->image_height());
  vital::mesh_vertex_array<3>& vertices = dynamic_cast< vital::mesh_vertex_array<3>& >(mesh->vertices());

  // Project all points on image
  std::vector<vital::vector_2d> points_2d(nb_vertices);
  std::vector<double> points_depth(nb_vertices);

  for (unsigned int i = 0; i < vertices.size(); ++i)
  {
    points_2d[i] = camera->project(vertices[i]);
  }

  // Compute the points depth
  vital::camera_perspective* cam_perspective = dynamic_cast<vital::camera_perspective*>(camera.get());
  vital::camera_affine* cam_affine = dynamic_cast<vital::camera_affine*>(camera.get());
  if (cam_perspective)
  {
    for (unsigned int i=0; i < vertices.size(); ++i)
    {
      points_depth[i] = cam_perspective->depth(vertices[i]);
    }
  }
  else if (cam_affine)
  {
    for (unsigned int i=0; i < vertices.size(); ++i)
    {
      points_depth[i] = dynamic_cast<vital::camera_affine*>(camera.get())->depth(vertices[i]);
    }
  }
  else
  {
    LOG_ERROR(vital::get_logger("arrows.core.compute_mesh_depth_map" ), "The camera has no depth method.");
    return nullptr;
  }

  // Initialize z_buffer with max double
  vital::image_of<double> z_buffer(width, height, 1);
  for (int j = 0; j < height; ++j)
  {
    for (int i = 0; i < width; ++i)
    {
      z_buffer(i, j) = std::numeric_limits<double>::infinity();
    }
  }

  // Write faces on z_buffer with depth test
  vital::mesh_face_array& faces = dynamic_cast< vital::mesh_face_array& >(mesh->faces());
  for (unsigned int f_id = 0; f_id < faces.size(); ++f_id)
  {
    const vital::vector_2d& a_uv = points_2d[faces(f_id, 0)];
    const vital::vector_2d& b_uv = points_2d[faces(f_id, 1)];
    const vital::vector_2d& c_uv = points_2d[faces(f_id, 2)];

    // skip the face if the three points are outside the image
    if ((a_uv[0] < 0 || a_uv[0] >= width || a_uv[1] < 0 || a_uv[1] >= height) &&
        (b_uv[0] < 0 || b_uv[0] >= width || b_uv[1] < 0 || b_uv[1] >= height) &&
        (c_uv[0] < 0 || c_uv[0] >= width || c_uv[1] < 0 || c_uv[1] >= height))
      continue;
    double a_depth = points_depth[faces(f_id, 0)];
    double b_depth = points_depth[faces(f_id, 1)];
    double c_depth = points_depth[faces(f_id, 2)];

    // the rasterization is done over the face axis-aligned bounding box
    int u_min = std::max(0, static_cast<int>(std::round(std::min(a_uv[0], std::min(b_uv[0], c_uv[0])))));
    int u_max = std::min(width - 1, static_cast<int>(std::round(std::max(a_uv[0], std::max(b_uv[0], c_uv[0])))));
    int v_min = std::max(0, static_cast<int>(std::round(std::min(a_uv[1], std::min(b_uv[1], c_uv[1])))));
    int v_max = std::min(height - 1, static_cast<int>(std::round(std::max(a_uv[1], std::max(b_uv[1], c_uv[1])))));

    for (int v = v_min; v <= v_max; ++v)
    {
      for (int u = u_min; u <= u_max; ++u)
      {
        vital::vector_2d p(u, v);

        // Handle pixels on triangle boundaries. Assignment rules:
        //  - if the pixel center is inside the triangle
        //  - if the pixel is not alread assigned and if the pixel intersects the triangle
        bool pixel_belongs_to_triangle = is_point_inside_triangle(p, a_uv, b_uv, c_uv);
        if (! pixel_belongs_to_triangle && std::isinf(z_buffer(u, v)))
        {
          // check for pixel - triangle intersection, by sub-sampling points in the pixel
          for (float dy = -0.5; dy <= 0.5; dy += 0.5)
          {
            for (float dx = -0.5; dx <= 0.5; dx += 0.5)
            {
              if (is_point_inside_triangle(p + vital::vector_2d(dx, dy), a_uv, b_uv, c_uv))
              {
                pixel_belongs_to_triangle = true;
                p += vital::vector_2d(dx, dy);
                break;
              }
            }
            if (pixel_belongs_to_triangle) break;
          }
        }

        if (pixel_belongs_to_triangle)
        {
          vital::vector_3d bary_coords = barycentric_coordinates(p, a_uv, b_uv, c_uv);
          // interpolate depth
          double depth=0.0;
          if (cam_perspective)
          {
            // for perspective cameras, depth is not linearly interpolated
            depth = 1.0 / (bary_coords[0] * (1.0 / a_depth) +
                           bary_coords[1] * (1.0 / b_depth) +
                           bary_coords[2] * (1.0 / c_depth));
          }
          else
          {
            // for other cameras, depth is linearly interpolated
            depth = bary_coords[0] * a_depth + bary_coords[1] * b_depth + bary_coords[2] * c_depth;
          }
          // depth test
          if (depth >= 0 && depth < z_buffer(u, v))
          {
            z_buffer(u, v) = depth;
          }
        }
      }
    }
  }

  return std::make_shared<vital::simple_image_container>(z_buffer);
}

}
}
