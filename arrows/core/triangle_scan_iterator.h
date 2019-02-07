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
 * \brief Header for kwiver::arrows::triangle_scan_iterator
 */

#ifndef KWIVER_ARROWS_CORE_TRIANGLE_SCAN_ITERATOR_H
#define KWIVER_ARROWS_CORE_TRIANGLE_SCAN_ITERATOR_H

#include <arrows/core/kwiver_algo_core_export.h>
#include <vital/types/vector.h>

namespace kwiver {
namespace arrows {
namespace core {


/// Provides access to the pixels of a triangle using scanlines
class KWIVER_ALGO_CORE_EXPORT triangle_scan_iterator
{
public:
  triangle_scan_iterator(vital::vector_2d const & pt1,
                         vital::vector_2d const & pt2,
                         vital::vector_2d const & pt3) :
    a(pt1), b(pt2), c(pt3)
  {
    reset();
  }

  /// Reset the iterator state
  void reset();

  /// Update the iterator to the next scanline
  bool next();

  /// Current scanline index
  int scan_y() const { return scan_y_; }

  /// Index of the first pixel of the current scanline
  int start_x() const { return start_x_; }

  /// Index of the last pixel of the current scanline
  int end_x() const { return end_x_; }

private:
  vital::vector_2d const &a, &b, &c;
  vital::vector_2d g;
  int scan_y_;
  int start_x_, end_x_;
  int x0, y0, x1, y1;
  double data[3][3];
};


/// Provides access to the pixels of a triangle by iterating over its bounding box.
/// Compared to the scanline method this method is slower, but includes the pixels
/// which are only partially inside the triangle (even if the pixel center is outside)
class KWIVER_ALGO_CORE_EXPORT triangle_bb_iterator
{
public:
  triangle_bb_iterator(vital::vector_2d const& pt1,
                       vital::vector_2d const& pt2,
                       vital::vector_2d const& pt3);

  bool next();

  void reset() { cur_line = tl_corner[1]-1; }

  int scan_y() const { return cur_line; }

  int start_x() const { return x_min; }

  int end_x() const { return x_max; }

  vital::vector_3d barycentric_coordinates(vital::vector_2d const& p) const;

private:
  bool is_point_inside_triangle(vital::vector_2d const& p);

  void update_scanline_range();

private:
  const double threshold_point_line_dist = 0.707;
  vital::vector_2d a, b, c;
  vital::vector_2i tl_corner, br_corner;
  vital::vector_2d v1, v2, v3;
  vital::vector_2d v1n, v2n, v3n;
  vital::vector_2d v1n_normalized, v2n_normalized, v3n_normalized;
  double s;
  int cur_line;
  int x_min, x_max;
  bool has_zero_area;
};

}
}
}


#endif // KWIVER_ARROWS_CORE_TRIANGLE_SCAN_ITERATOR_H
