/*ckwg +29
 * Copyright 2018 by Kitware, SAS.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Header for core generate_texture functions
 */

#ifndef KWIVER_ARROWS_CORE_GENERATE_TEXTURE_H
#define KWIVER_ARROWS_CORE_GENERATE_TEXTURE_H

#include <arrows/core/render_mesh_depth_map.h>
#include <arrows/core/triangle_scan_iterator.h>
#include <arrows/core/uv_unwrap_mesh.h>

#include <vital/types/camera.h>
#include <vital/types/camera_perspective.h>
#include <vital/types/image.h>
#include <vital/types/image_container.h>
#include <vital/types/mesh.h>
#include <vital/types/vector.h>

#include <vital/util/transform_image.h>

#include <iostream>
#include <numeric>


namespace kwiver {
namespace arrows {
namespace core {


template <class T>
void dilate_atlas(vital::image& atlas, vital::image_of<char>& mask, int nb_iter);


/// This function samples an image at a non-integer location with bilinear interpolation
/**
* \param img [in] image
* \param x [in] location
* \param y [in] location
* \param d [in] depth
*/
template <class T>
inline double bilinear_interp_safe(vital::image_of<T> const& img, double x, double y, int d=0)
{
  if (x < 0 || y < 0 || x >= img.width() - 1|| y >= img.height() - 1)
    return 0.0;

  int p1x = static_cast<int>(x);
  double normx = x - p1x;
  int p1y = static_cast<int>(y);
  double normy = y - p1y;

  ptrdiff_t w_step = img.w_step(), h_step = img.h_step();
  T const* pix1 = reinterpret_cast<T const*>(img.first_pixel()) + img.d_step() * d + h_step * p1y + w_step * p1x;

  if (normx < 1e-9 && normy < 1e-9) return *pix1;
  if (normx < 1e-9) return static_cast<T>(pix1[0] + (pix1[h_step] - pix1[0]) * normy);
  if (normy < 1e-9) return static_cast<T>(pix1[0] + (pix1[w_step] - pix1[0]) * normx);

  double i1 = pix1[0] + (pix1[h_step] - pix1[0]) * normy;
  double i2 = pix1[w_step] + (pix1[w_step + h_step] - pix1[w_step]) * normy;

  return i1 + (i2 - i1) * normx;
}


/// This functions dispatches calls to templated versions of bilinear interpolation based on pixels type
/**
* \param img [in] image
* \param x [in] location
* \param y [in] location
* \param d [in] depth
*/
double bilinear_interp_safe(vital::image const& img, double x, double y, int d=0)
{
  if (img.pixel_traits() == vital::image_pixel_traits_of<unsigned char>())
    return bilinear_interp_safe(vital::image_of<unsigned char>(img), x, y, d);
  else if (img.pixel_traits() == vital::image_pixel_traits_of<int>())
    return bilinear_interp_safe(vital::image_of<int>(img), x, y, d);
  else if (img.pixel_traits() == vital::image_pixel_traits_of<float>())
    return bilinear_interp_safe(vital::image_of<float>(img), x, y, d);
  else if (img.pixel_traits() == vital::image_pixel_traits_of<double>())
    return bilinear_interp_safe(vital::image_of<double>(img), x, y, d);
}


/// Base functor used to fuse multiple images. It adjusts the contribution of each image.
struct image_fusion_method
{
  virtual void operator ()(std::vector<double>& scores) const = 0;
};


/// This functor sets the highest score to 1 and the other to 0
struct select_max_score : image_fusion_method
{
  virtual void operator ()(std::vector<double>& scores) const
  {
    auto max_it = std::max_element(scores.begin(), scores.end());
    if (*max_it > 0)
    {
      std::fill(scores.begin(), scores.end(), 0.0);
      *max_it = 1.0;
    }
  }
};


/// This functor normalizes the scores
struct normalize_scores : image_fusion_method
{
  virtual void operator ()(std::vector<double>& scores) const
  {
    double sum = std::accumulate(scores.begin(), scores.end(), 0.0);
    if (sum > 0)
      std::for_each(scores.begin(), scores.end(), [&sum](double& v) { v /= sum; });
  }
};


/// This function renders a triangle and fills it with data from different images
/**
 * \param v1 [in] 2D triangle point
 * \param v2 [in] 2D triangle point
 * \param v3 [in] 2D triangle point
 * \param pt1 [in] 3D triangle point
 * \param pt2 [in] 3D triangle point
 * \param pt3 [in] 3D triangle point
 * \param cameras [in] list of cameras
 * \param images [in] list of images
 * \param depths_pt1 [in] depths of pt1 w.r.t cameras
 * \param depths_pt2 [in] depths of pt2 w.r.t cameras
 * \param depths_pt3 [in] depths of pt3 w.r.t cameras
 * \param depth_maps [in] list of depthmaps
 * \param texture [out] texture where the triangle is rendered
 * \param depth_threshold [in] threshold used for depth test
 */
template <class T>
void render_triangle_from_images(vital::vector_2d const& v1, vital::vector_2d const& v2, vital::vector_2d const& v3,
                                 vital::vector_3d const& pt1, vital::vector_3d const& pt2, vital::vector_3d const& pt3,
                                 std::vector<vital::camera_sptr> const& cameras, std::vector<vital::image> const& images,
                                 std::vector<double> const& depths_pt1,
                                 std::vector<double> const& depths_pt2,
                                 std::vector<double> const& depths_pt3,
                                 std::vector<vital::image> const& depth_maps,
                                 vital::image_of<T>& texture, double depth_threshold,
                                 image_fusion_method const& adjust_images_contributions)
{
  // Compute a score for each image
  std::vector<double> scores(images.size(), 0.0);
  vital::matrix_3x3d points;
  for (size_t i = 0; i < images.size(); ++i)
  {
    points << cameras[i]->project(pt1), cameras[i]->project(pt2), cameras[i]->project(pt3), 1, 1, 1;
    scores[i] = std::max(-points.determinant(), 0.0);
  }

  triangle_scan_iterator tsi(v1, v2, v3);
  vital::vector_2d vt1(v2 - v1), vt2(v3 - v1);
  vital::vector_2d vn1(-vt1[1], vt1[0]), vn2(vt2[1], -vt2[0]);
  double inv_area = 1.0 / (vt1[0] * vt2[1] - vt1[1] * vt2[0]);
  std::vector<vital::vector_2d> points_2d(images.size());
  std::vector<T> values(texture.depth(), 0.0);
  for (tsi.reset(); tsi.next(); )
  {
    int y = tsi.scan_y();
    if (y < 0 || y >= static_cast<int>(texture.height()))
      continue;
    int min_x = std::max(0, tsi.start_x());
    int max_x = std::min(static_cast<int>(texture.width()) - 1, tsi.end_x());

    for (int x = min_x; x <= max_x; ++x)
    {
      vital::vector_2d vp(vital::vector_2d(x, y) - v1);
      double bary_coord_3 = inv_area * vn1.dot(vp);
      double bary_coord_2 = inv_area * vn2.dot(vp);
      double bary_coord_1 = 1.0 - bary_coord_2 - bary_coord_3;
      // Corresponding 3d point
      vital::vector_3d pt3d = bary_coord_1 * pt1 + bary_coord_2 * pt2 + bary_coord_3 * pt3;

      std::vector<double> point_scores = scores;
      for (size_t i = 0; i < images.size(); ++i)
      {
        // Corresponding point in image i
        vital::vector_2d pt_img = cameras[i]->project(pt3d);
        points_2d[i] = pt_img;
        // border check from the camera i
        if (pt_img(0) < 0 || pt_img(0) >= images[i].width() || pt_img(1) < 0 || pt_img(1) >= images[i].height())
        {
          point_scores[i] = 0;
          continue;
        }
        // visibility test from the camera i
        double interpolated_depth = bary_coord_1 * depths_pt1[i] +
                                    bary_coord_2 * depths_pt2[i] +
                                    bary_coord_3 * depths_pt3[i];
        if (std::abs(interpolated_depth -  bilinear_interp_safe(depth_maps[i], pt_img(0), pt_img(1))) > depth_threshold)
        {
          point_scores[i] = 0;
        }
      }

      // adjust the scores for fusion
      std::fill(values.begin(), values.end(), 0.0);
      adjust_images_contributions(point_scores);
      for (size_t i = 0; i < images.size(); ++i)
      {
        if (point_scores[i] > 0)
        {
          for (size_t d = 0; d < texture.depth(); ++d)
            values[d] += point_scores[i] * bilinear_interp_safe(images[i], points_2d[i](0), points_2d[i](1), d);
        }
      }
      // fill the texture pixel
      for (size_t d = 0; d < texture.depth(); ++d)
        texture(x, y, d) = values[d];
    }
  }
}


/// This functions dilates the texture atlas using a binary mask
/**
 * \param texture [in/out] image to dilate
 * \param mask [in/out] binary mask used for dilation
 * \param nb_iter [in] the dilation is repeated nb_iter times
 */
template <class T>
void dilate_atlas(vital::image& texture, vital::image_of<char>& mask, int nb_iter)
{
  int height = static_cast<int>(texture.height());
  int width = static_cast<int>(texture.width());

  // lambda to copy a pixel with any depth
  auto copy_pixel = [&texture](unsigned int x_d, unsigned int y_d, unsigned int x_s, unsigned int y_s)
  {
    T* dest = &texture.at<T>(x_d, y_d, 0);
    T* src = &texture.at<T>(x_s, y_s, 0);
    for (int i = 0; i < texture.depth(); ++i, src += texture.d_step(), dest += texture.d_step())
    {
      *dest = *src;
    }
  };
  for (int n = 0; n < nb_iter; ++n)
  {
    // horizontal dilate
    for (unsigned int y = 0; y < height; ++y)
    {
      for (unsigned int x = 0; x < width - 1; ++x)
      {
        char d = mask(x + 1, y) - mask(x, y);
        if (d == 1)
        {
          mask(x, y) = 1;
          copy_pixel(x, y, x + 1, y);
        }
        else if (d == -1)
        {
          mask(x + 1, y) = 1;
          copy_pixel(x + 1, y, x, y);
          ++x;
        }
      }
    }
    // vertical dilate
    for (unsigned int x = 0; x < width; ++x)
    {
      for (unsigned int y = 0; y < height - 1; ++y)
      {
        char d = mask(x, y + 1) - mask(x, y);
        if (d == 1)
        {
          mask(x, y) = 1;
          copy_pixel(x, y, x, y + 1);
        }
        else if (d == -1)
        {
          mask(x, y + 1) = 1;
          copy_pixel(x, y + 1, x, y);
          ++y;
        }
      }
    }
  }
}


/// This function generates a texture from a set of images and maps it on the mesh
/**
 * \param mesh [in/out] the mesh to texture.
 * \param cameras [in] a list of cameras perspective
 * \param images [in] a list of images
 * \param resolution [in] resolution of the texture (mesh unit/pixel)
 */
template <class T>
vital::image_container_sptr
generate_texture(vital::mesh_sptr mesh, std::vector<vital::camera_perspective_sptr> const& cameras,
                 std::vector<vital::image> const& images, double resolution)
{
  if (mesh->faces().regularity() != 3)
  {
    LOG_ERROR(vital::get_logger("arrows.core.generate_texture" ), "The mesh has to be triangular.");
    return nullptr;
  }

  kwiver::vital::mesh_vertex_array<3>& vertices = dynamic_cast< kwiver::vital::mesh_vertex_array<3>& >(mesh->vertices());
  auto const& triangles = static_cast< kwiver::vital::mesh_regular_face_array<3> const& >(mesh->faces());

  // Unwrap the mesh
  if (mesh->has_tex_coords() == 0)
  {
    uv_unwrap_mesh unwrap;
    vital::config_block_sptr config = unwrap.get_configuration();
    unwrap.set_configuration(config);
    unwrap.unwrap(mesh);
  }

  auto tcoords = mesh->tex_coords();
  // Rescale tcoords to real pixel values
  size_t scale = 1;
  for (unsigned int f = 0; f < mesh->num_faces(); ++f)
  {
    vital::matrix_3x3d points_2d_h;
    points_2d_h << tcoords[f * 3 + 0], tcoords[f * 3 + 1], tcoords[f * 3 + 2], 1, 1, 1;
    double area_2d = points_2d_h.determinant();
    auto const& v1 = vertices[triangles(f, 0)];
    auto const& v2 = vertices[triangles(f, 1)];
    auto const& v3 = vertices[triangles(f, 2)];
    vital::vector_3d a3 = v2 - v1;
    vital::vector_3d b3 = v3 - v1;
    double area_3d = a3.cross(b3).norm();

    if (area_2d > 0 && area_3d > 0 && !std::isinf(area_2d) && !std::isinf(area_3d))
    {
      scale = static_cast<size_t>(std::ceil(sqrt(area_3d / area_2d) / resolution));
      break;
    }
  }
  for (auto& tc : tcoords)
  {
    tc.y() = 1.0 - tc.y();
    tc *= scale;
  }

  // Render the depth maps of the mesh seen by the different cameras
  std::vector<vital::image> depth_maps(images.size());
  for (unsigned int i = 0; i < images.size(); ++i)
  {
    depth_maps[i] = render_mesh_depth_map(mesh, cameras[i])->get_image();
  }

  // Compute the depth of each points w.r.t each camera
  std::vector< std::vector<double> > per_camera_point_depth(mesh->num_verts(), std::vector<double>(cameras.size(), 0.0));
  for (unsigned int v = 0; v < mesh->num_verts(); ++v)
  {
    for (unsigned int c = 0; c < cameras.size(); ++c)
    {
      per_camera_point_depth[v][c] = cameras[c]->depth(vertices[v]);
    }
  }

  std::vector<vital::camera_sptr> cameras_base(cameras.size());
  for (unsigned int k = 0; k < cameras.size(); ++k)
  {
    cameras_base[k] = cameras[k];
  }

  vital::image_of<T> texture(scale, scale, images[0].depth());
  vital::transform_image(texture, [](T){ return 0; });
  kwiver::vital::image_of<char> texture_mask(scale, scale, 1);
  vital::transform_image(texture_mask, [](char){ return 0; });
  for (unsigned int f = 0; f < mesh->num_faces(); ++f)
  {
    unsigned int p1 = triangles(f, 0);
    unsigned int p2 = triangles(f, 1);
    unsigned int p3 = triangles(f, 2);
    vital::vector_3d const& pt_0 = vertices[p1];
    vital::vector_3d const& pt_1 = vertices[p2];
    vital::vector_3d const& pt_2 = vertices[p3];

    render_triangle_from_images<T>(tcoords[f * 3], tcoords[f * 3 + 1], tcoords[f * 3 + 2],
                                  pt_0, pt_1, pt_2, cameras_base, images,
                                  per_camera_point_depth[p1],
                                  per_camera_point_depth[p2],
                                  per_camera_point_depth[p3],
                                  depth_maps, texture, 0.1, select_max_score());
    kwiver::arrows::core::render_triangle<char>(tcoords[f * 3], tcoords[f * 3 + 1],
                                                tcoords[f * 3 + 2], 1, texture_mask);
  }

  kwiver::arrows::core::dilate_atlas<T>(texture, texture_mask, 4);

  // Update texture coordinates
  for (auto& tc : tcoords)
  {
    tc[0] += 0.5;   // half-pixel shift
    tc[1] += 0.5;
    tc /= scale;
    tc[1] = 1.0 - tc[1];
  }
  mesh->set_tex_coords(tcoords);

  return std::make_shared<vital::simple_image_container>(texture);
}


}
}
}

#endif // KWIVER_ARROWS_CORE_GENERATE_TEXTURE_H
