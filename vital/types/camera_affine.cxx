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

#include "camera_affine.h"

#include <Eigen/Dense>

namespace kwiver {
namespace vital {


camera_affine
::camera_affine()
  : m_logger( kwiver::vital::get_logger( "vital.camera_affine" ) )
{
}


/// Project  3D point into a 2D image point
vector_2d camera_affine::project(const vector_3d &pt) const
{
  return (get_matrix() * pt.homogeneous()).hnormalized();
}


/// Compute the depth from the camera to pt along the ray
double camera_affine::depth(const vector_3d &pt) const
{
  return this->get_viewing_distance() - pt.dot(this->center().head(3));
}


/// Construct an affine camera from a ray direction, an up vector, a stare point, a principal point,
/// a scale and the image dimension
simple_camera_affine::simple_camera_affine(const vector_3d &ray, const vector_3d &up,
                                           const vector_3d &stare_pt, const vector_2d &pp,
                                           const vector_2d &scale, unsigned int image_width,
                                           unsigned int image_height) :
  image_width_(image_width), image_height_(image_height)
{
  vector_3d uvec = up.normalized();
  vector_3d rvec = ray.normalized();
  matrix_3x3d R;
  bool use_default_orientation = false;

  // if up and ray vectors are collinear
  if (std::abs(uvec.dot(rvec)-1) < 1e-5 || std::abs(uvec.dot(rvec)+1) < 1e-5)
  {
    // if ray, up and z are collinear and
    if (std::abs(rvec(2) - 1) < 1e-5)
    { // ray is pointing in +z
      R << 1, 0, 0,
           0, 1, 0,
           0, 0, 1;
      use_default_orientation = true;
    }
    else if (std::abs(rvec(2) + 1) < 1e-5)
    { // ray is pointing in -z
      R << 1,  0,  0,
           0, -1,  0,
           0,  0, -1;
      use_default_orientation = true;
    }
    // otherwise keep ray and set up vector to z
    else
    {
      uvec = {0.0, 0.0, 1.0};
    }
  }

  if (! use_default_orientation)
  {
    vector_3d x = -uvec.cross(rvec);
    vector_3d y = rvec.cross(x);
    x.normalize();
    y.normalize();

    R.row(0) = x;
    R.row(1) = y;
    R.row(2) = rvec;
  }

  P_ = matrix_3x4d::Zero();
  P_.row(0).head(3) = R.row(0) * scale(0);
  P_.row(1).head(3) = R.row(1) * scale(1);
  P_(2, 3) = 1.0;

  vector_2d uv0 = this->project(stare_pt);
  vector_2d t = pp - uv0;
  P_(0, 3) = t(0);
  P_(1, 3) = t(1);
  view_distance_ = 0.0;
  ray_dir_ = rvec;
}


/// Construct an affine camera from a matrix and the image dimension
simple_camera_affine::simple_camera_affine(const matrix_3x4d &camera_matrix, unsigned int image_width,
                                           unsigned int image_height)
  : view_distance_(0.0), image_width_(image_width), image_height_(image_height)
{
  ray_dir_ = {0.0, 0.0, 1.0};   // default ray
  set_matrix(camera_matrix);
}


/// Copy constructor
simple_camera_affine::simple_camera_affine(const camera_affine &base) :
  view_distance_(base.get_viewing_distance()),
  image_width_(base.image_height()),
  image_height_(base.image_height())
{
  this->set_matrix(base.get_matrix());
}


/// Set the camera matrix
void simple_camera_affine::set_matrix(const kwiver::vital::matrix_3x4d &new_camera_matrix)
{
  assert(new_camera_matrix(2, 3) != 0);

  P_ = new_camera_matrix;
  P_ = P_ / P_(2, 3);
  P_(2, 0) = 0.0;
  P_(2, 1) = 0.0;
  P_(2, 2) = 0.0;

  Eigen::FullPivLU<matrix_3x4d> lu(P_);
  matrix_3x4d nullspace = lu.kernel();
  vector_3d cc = nullspace.col(0);
  vector_3d old_ray = ray_dir_;
  ray_dir_ = cc.normalized();
  orient_ray_direction(old_ray);
}


/// Get the camera center (an ideal point)
vector_4d simple_camera_affine::center() const
{
  vector_4d center;
  center.head(3) = -ray_dir_;
  center(3) = 0.0;
  return center;
}


/// Orient the camera ray direction so that the dot product with look_dir is positive
void simple_camera_affine::orient_ray_direction(const vector_3d &look_dir)
{
  if (ray_dir_.dot(look_dir) < 0)
  {
    ray_dir_ *= -1;
  }
}


/// Get the camera principal plane
vector_4d simple_camera_affine::principal_plane() const
{
  vector_4d plane;
  plane.head(3) = ray_dir_;
  plane(3) = view_distance_;
  return plane;
}


}
}
