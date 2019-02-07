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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
 * \brief Implementation of core triangle_scan_iterator
 */

#include "triangle_scan_iterator.h"

namespace {

void min_max(double a, double b, double c, double &min, double &max)
{
  min = std::min(std::min(a, b), c);
  max = std::max(std::max(a, b), c);
}
}

namespace kwiver {
namespace arrows {
namespace core {

void triangle_scan_iterator::reset()
{
  double min, max;
  min_max(a(0), b(0), c(0), min, max);
  x0 = static_cast<int>(std::ceil(min));
  x1 = static_cast<int>(std::floor(max));

  min_max(a(1), b(1), c(1), min, max);
  y0 = static_cast<int>(std::ceil(min));
  y1 = static_cast<int>(std::floor(max));

  scan_y_ = y0 - 1;

  g = ((a + b + c) / 3).array().floor();

  vital::vector_2d ga(a(0) - g(0), a(1) - g(1));
  vital::vector_2d gb(b(0) - g(0), b(1) - g(1));
  vital::vector_2d gc(c(0) - g(0), c(1) - g(1));

  data[0][0] = gb(1) - gc(1); data[0][1] = gc(0) - gb(0); data[0][2] = gb(0) * gc(1) - gb(1) * gc(0);
  data[1][0] = gc(1) - ga(1); data[1][1] = ga(0) - gc(0); data[1][2] = gc(0) * ga(1) - gc(1) * ga(0);
  data[2][0] = ga(1) - gb(1); data[2][1] = gb(0) - ga(0); data[2][2] = ga(0) * gb(1) - ga(1) * gb(0);
  double tmp = (gb(0) * gc(1) - gb(0) * ga(1) - ga(0) * gc(1)
                - gc(0) * gb(1) + gc(0) * ga(1) + ga(0) * gb(1));

  if (tmp < 0)
    tmp = -1;
  else
    tmp = 1;

  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      data[i][j] *= tmp;
    }
  }
}

bool triangle_scan_iterator::next()
{
  if (++scan_y_ > y1)
    return false;

  double xmin = x0 - g(0);
  double xmax = x1 - g(0);
  for (auto & i : data)
  {
    double a_ = i[0];
    double b_ = i[1] * (scan_y_ - g(1)) + i[2];
    if (std::abs(a_) > 1e-5)
    {
      double x = -b_ / a_;
      if (a_ > 0)
      {
        if (x > xmin)
          xmin = x;
      }
      else
      {
        if (x < xmax)
          xmax = x;
      }
    }
  }
  start_x_ = static_cast<int>(std::ceil(xmin + g(0)));
  end_x_ = static_cast<int>(std::floor(xmax + g(0)));
  return true;
}


triangle_bb_iterator::triangle_bb_iterator(vital::vector_2d const& pt1,
                                           vital::vector_2d const& pt2,
                                           vital::vector_2d const& pt3) :
  a(pt1), b(pt2), c(pt3), has_zero_area(false)
{
  tl_corner[0] = std::floor(std::min(std::min(a[0], b[0]), c[0]));
  tl_corner[1] = std::floor(std::min(std::min(a[1], b[1]), c[1]));
  br_corner[0] = std::ceil(std::max(std::max(a[0], b[0]), c[0]));
  br_corner[1] = std::ceil(std::max(std::max(a[1], b[1]), c[1]));

  v1 = b - a;
  v2 = c - a;
  v3 = c - b;
  v1n = vital::vector_2d(-v1[1], v1[0]);
  v2n = vital::vector_2d(v2[1], -v2[0]);
  v3n = vital::vector_2d(-v3[1], v3[0]);

  double area2 = v2[1] * v1[0] - v2[0] * v1[1];
  if (area2 == 0.0)
  {
    has_zero_area = true;
    s = 1.0;
  }
  else
  {
    s = 1.0 / (v2[1] * v1[0] - v2[0] * v1[1]);
  }

  v1n_normalized = v1n.normalized();
  v2n_normalized = v2n.normalized();
  v3n_normalized = v3n.normalized();

  reset();
}


bool triangle_bb_iterator::next()
{
  if (has_zero_area)
    return false;

  if (cur_line < br_corner[1])
  {
    cur_line++;
    update_scanline_range();
    return true;
  }
  return false;
}


vital::vector_3d triangle_bb_iterator::barycentric_coordinates(vital::vector_2d const& p) const
{
  vital::vector_2d vp = p - this->a;
  double b0 = this->s * this->v1n.dot(vp);
  double b1 = this->s * this->v2n.dot(vp);
  return vital::vector_3d(1.0 - b0 - b1, b1, b0);
}


inline bool triangle_bb_iterator::is_point_inside_triangle(vital::vector_2d const& p)
{
  vital::vector_2d vp = p - this->a;
  double b0 = this->s * this->v1n.dot(vp);
  double b1 = this->s * this->v2n.dot(vp);
  return b0 >= 0 && b1 >= 0 && (b1 + b0) <= 1;
}


void triangle_bb_iterator::update_scanline_range()
{
  int left = tl_corner[0];
  int right = br_corner[0];
  while (left < right) {
    vital::vector_2d p(left, this->cur_line);
    vital::vector_2d v_ap = p - this->a;
    vital::vector_2d v_bp = p - this->b;
    vital::vector_2d v_cp = p - this->c;
    if (is_point_inside_triangle({left, cur_line}))
    {
      break;
    }
    else if (std::abs(v1n_normalized.dot(v_ap)) <= threshold_point_line_dist ||
             std::abs(v2n_normalized.dot(v_cp)) <= threshold_point_line_dist ||
             std::abs(v3n_normalized.dot(v_bp)) <= threshold_point_line_dist)
    {
       break;
    }
    else
    {
      ++left;
    }
  }
  while (right > left)
  {
    vital::vector_2d p(right, this->cur_line);
    vital::vector_2d v_ap = p - this->a;
    vital::vector_2d v_bp = p - this->b;
    vital::vector_2d v_cp = p - this->c;
    if (is_point_inside_triangle({right, cur_line}))
    {
      break;
    }
    else if (std::abs(v1n_normalized.dot(v_ap)) <= threshold_point_line_dist ||
             std::abs(v2n_normalized.dot(v_cp)) <= threshold_point_line_dist ||
             std::abs(v3n_normalized.dot(v_bp)) <= threshold_point_line_dist)
    {
      break;
    }
    else
    {
      --right;
    }
  }
  x_min = left;
  x_max = right;
}


}
}
}
