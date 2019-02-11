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
 * \brief Implementation for core generate_texture functions
 */

#include "generate_texture.h"

#include <algorithm>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace core {



/// Sets the highest score to 1 and the other to 0
void select_max_score::operator ()(double *scores, int nb_scores) const
{
  auto max_it = std::max_element(scores, scores + nb_scores);
  if (*max_it > 0)
  {
    std::fill(scores, scores + nb_scores, 0.0);
    *max_it = 1.0;
  }
}


/// Normalizes the scores
void normalize_scores::operator ()(double *scores, int nb_scores) const
{
  double sum = std::accumulate(scores, scores + nb_scores, 0.0);
  if (sum > 0)
    std::for_each(scores, scores + nb_scores, [&sum](double& v) { v /= sum; });
}


/// Adjust cameras contributions
void adjust_cameras_contributions(vital::vector_2d const& v1,
                                  vital::vector_2d const& v2,
                                  vital::vector_2d const& v3,
                                  image_fusion_method const& method,
                                  vital::image_of<double>& scores_image)
{
  triangle_bb_iterator tsi(v1, v2, v3);
  for (tsi.reset(); tsi.next(); )
  {
    int y = tsi.scan_y();
    if (y < 0 || y >= static_cast<int>(scores_image.height()))
       continue;
    int min_x = std::max(0, tsi.start_x());
    int max_x = std::min(static_cast<int>(scores_image.width()) - 1, tsi.end_x());
    for (int x = min_x; x <= max_x; ++x)
    {
      double* pt = scores_image.first_pixel() + scores_image.h_step() * y + scores_image.w_step() * x;
      method(pt, scores_image.depth());
    }
  }
}


/// Render triangle score and visibility
void render_triangle_scores(vital::vector_2d const& v1, vital::vector_2d const& v2, vital::vector_2d const& v3,
                            vital::vector_3d const& pt1, vital::vector_3d const& pt2, vital::vector_3d const& pt3,
                            std::vector<vital::camera_sptr> const& cameras,
                            std::vector<double> const& depths_pt1,
                            std::vector<double> const& depths_pt2,
                            std::vector<double> const& depths_pt3,
                            std::vector< vital::image_of<double> > const& depth_maps,
                            double depth_threshold,
                            vital::image_of<double>& scores_image,
                            vital::image_of<double>& points_image)
{
  // Compute a score for each image
  std::vector<double> scores(cameras.size(), 0.0);
  vital::matrix_3x3d points;
  for (size_t i = 0; i < cameras.size(); ++i)
  {
    points << cameras[i]->project(pt1), cameras[i]->project(pt2), cameras[i]->project(pt3), 1, 1, 1;
    scores[i] = std::max(-points.determinant(), 0.0);
  }

  triangle_bb_iterator tsi(v1, v2, v3);
  for (tsi.reset(); tsi.next(); )
  {
    int y = tsi.scan_y();
    if (y < 0 || y >= static_cast<int>(scores_image.height()))
      continue;
    int min_x = std::max(0, tsi.start_x());
    int max_x = std::min(static_cast<int>(scores_image.width()) - 1, tsi.end_x());

    for (int x = min_x; x <= max_x; ++x)
    {
      vital::vector_3d bary = tsi.barycentric_coordinates({x, y});
      vital::vector_3d pt3d = bary.x() * pt1 + bary.y() * pt2 + bary.z() * pt3;

      for (size_t i = 0; i < cameras.size(); ++i)
      {
        // Corresponding point in image i
        vital::vector_2d pt_img = cameras[i]->project(pt3d);
        points_image(x, y, i*2) = pt_img(0);
        points_image(x, y, i*2+1) = pt_img(1);
        // border check from the camera i
        if (pt_img(0) < 0 || pt_img(0) >= cameras[i]->image_width() || pt_img(1) < 0 || pt_img(1) >= cameras[i]->image_height())
        {
          scores_image(x, y, i) = 0.0;
          continue;
        }
        // visibility test from the camera i
        double interpolated_depth = bary.x() * depths_pt1[i] +
                                    bary.y() * depths_pt2[i] +
                                    bary.z() * depths_pt3[i];
        if (std::abs(interpolated_depth - bilinear_interp_safe<double>(depth_maps[i], pt_img(0), pt_img(1))) > depth_threshold)
        {
          scores_image(x, y, i) = 0.0;
          continue;
        }
        scores_image(x, y, i) = scores[i];
      }
    }
  }
}


vital::vector_2d find_largest_face_dimensions(std::vector<vital::vector_2d> const& coords, unsigned int nb_faces)
{
  // Find the bounding box dimension of the largest face
  double max_w = std::numeric_limits<double>::min();
  double max_h = std::numeric_limits<double>::min();
  for (unsigned int f = 0; f < nb_faces; ++f)
  {
    vital::vector_2d tc0 = coords[f * 3];
    vital::vector_2d tc1 = coords[f * 3 + 1];
    vital::vector_2d tc2 = coords[f * 3 + 2];

    double w = std::max(tc0.x(), std::max(tc1.x(), tc2.x()))
               - std::min(tc0.x(), std::min(tc1.x(), tc2.x()));
    double h = std::max(tc0.y(), std::max(tc1.y(), tc2.y()))
               - std::min(tc0.y(), std::min(tc1.y(), tc2.y()));

    if (w > max_w)  max_w = w;
    if (h > max_h)  max_h = h;
  }
  return vital::vector_2d(max_w, max_h);
}


size_t find_texture_scaling(const vital::mesh_vertex_array<3> &vertices,
                            const std::vector<vector_2d> &tcoords,
                            const vital::mesh_regular_face_array<3> &faces,
                            double resolution)
{
  size_t scale = 1;
  for (unsigned int f = 0; f < faces.size(); ++f)
  {
    vital::matrix_3x3d points_2d_h;
    points_2d_h << tcoords[f * 3 + 0], tcoords[f * 3 + 1], tcoords[f * 3 + 2], 1, 1, 1;
    double area_2d = points_2d_h.determinant();
    auto const& v1 = vertices[faces(f, 0)];
    auto const& v2 = vertices[faces(f, 1)];
    auto const& v3 = vertices[faces(f, 2)];
    vital::vector_3d a3 = v2 - v1;
    vital::vector_3d b3 = v3 - v1;
    double area_3d = a3.cross(b3).norm();

    if (area_2d > 0 && area_3d > 0 && !std::isinf(area_2d) && !std::isinf(area_3d))
    {
      scale = static_cast<size_t>(std::ceil(sqrt(area_3d / area_2d) / resolution));
      break;
    }
  }
  return scale;
}


}
}
}

