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
void adjust_cameras_contributions(std::vector<vital::vector_2d> const& tex_coords,
                                  image_fusion_method const& method,
                                  vital::image_of<double>& scores_image)
{
  for (unsigned int i = 2; i < tex_coords.size(); i+=3)
  {
    triangle_scan_iterator tsi(tex_coords[i-2], tex_coords[i-1], tex_coords[i]);
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

  triangle_scan_iterator tsi(v1, v2, v3);
  vital::vector_2d vt1(v2 - v1), vt2(v3 - v1);
  vital::vector_2d vn1(-vt1[1], vt1[0]), vn2(vt2[1], -vt2[0]);
  double inv_area = 1.0 / (vt1[0] * vt2[1] - vt1[1] * vt2[0]);
  for (tsi.reset(); tsi.next(); )
  {
    int y = tsi.scan_y();
    if (y < 0 || y >= static_cast<int>(scores_image.height()))
      continue;
    int min_x = std::max(0, tsi.start_x());
    int max_x = std::min(static_cast<int>(scores_image.width()) - 1, tsi.end_x());

    for (int x = min_x; x <= max_x; ++x)
    {
      vital::vector_2d vp(vital::vector_2d(x, y) - v1);
      double bary_coord_3 = inv_area * vn1.dot(vp);
      double bary_coord_2 = inv_area * vn2.dot(vp);
      double bary_coord_1 = 1.0 - bary_coord_2 - bary_coord_3;
      // Corresponding 3d point
      vital::vector_3d pt3d = bary_coord_1 * pt1 + bary_coord_2 * pt2 + bary_coord_3 * pt3;

      for (size_t i = 0; i < cameras.size(); ++i)
      {
        // Corresponding point in image i
        vital::vector_2d pt_img = cameras[i]->project(pt3d);
        points_image(x, y, i*2) = pt_img(0);
        points_image(x, y, i*2+1) = pt_img(1);
        // border check from the camera i
        if (pt_img(0) < 0 || pt_img(0) >= cameras[i]->image_width() || pt_img(1) < 0 || pt_img(1) >= cameras[i]->image_height())
        {
          continue;
        }
        // visibility test from the camera i
        double interpolated_depth = bary_coord_1 * depths_pt1[i] +
                                    bary_coord_2 * depths_pt2[i] +
                                    bary_coord_3 * depths_pt3[i];
        if (std::abs(interpolated_depth - bilinear_interp_safe<double>(depth_maps[i], pt_img(0), pt_img(1))) > depth_threshold)
        {
          continue;
        }
        scores_image(x, y, i) = scores[i];
      }
    }
  }
}



}
}
}

