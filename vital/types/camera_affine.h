/*ckwg +29
 * Copyright 2013-2018 by Kitware, Inc.
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
 * \brief Header for \link kwiver::vital::camera_affine camera_affine \endlink and
 *        \link kwiver::vital::camera_affine_camera_affine<T> \endlink classes
 */

#ifndef VITAL_CAMERA_AFFINE_H_
#define VITAL_CAMERA_AFFINE_H_

#include <vital/vital_export.h>

#include <iostream>
#include <vector>
#include <memory>

#include <vital/vital_config.h>
#include <vital/types/camera.h>
#include <vital/types/matrix.h>
#include <vital/types/vector.h>
#include <vital/logger/logger.h>


namespace kwiver {
namespace vital {

/// forward declaration of affine camera class
class camera_affine;
/// typedef for a camera_affine shared pointer
typedef std::shared_ptr< camera_affine > camera_affine_sptr;


// ------------------------------------------------------------------
/// An abstract representation of affine camera
class VITAL_EXPORT camera_affine : public camera
{
public:
  /// Destructor
  virtual ~camera_affine() = default;

  /// Create a clone of this camera_affine object
  virtual camera_sptr clone() const = 0;

  /// Accessor for the camera center of projection (an ideal point)
  virtual vector_4d center() const = 0;

  /// Get the projection matrix
  virtual matrix_3x4d get_matrix() const = 0;

  // Get the distance from the origin along the ray
  virtual double get_viewing_distance() const = 0;

  /// Project a 3D point into a 2D image point
  virtual vector_2d project( const vector_3d& pt ) const;

  /// Compute the distance of the 3D point to the image plane
  /**
   *  Points with negative depth are behind the camera
   */
  virtual double depth(const vector_3d& pt) const;

protected:
  camera_affine();

  kwiver::vital::logger_handle_t m_logger;
};


/// A representation of an affine camera
class VITAL_EXPORT simple_camera_affine :
  public camera_affine
{
public:
  /// Default Constructor
  simple_camera_affine ( )
  : ray_dir_( 0.0, 0.0, 1.0 ),
    P_(matrix_3x4d::Zero()),
    view_distance_(0.0),
    image_width_(0),
    image_height_(0)
  {
    P_(0, 0 ) = 1.0;
    P_(1, 1) = 1.0;
    P_(2, 3) = 1.0;
  }

  /// Construct an affine camera from a ray direction, an up vector, a stare point,
  /// a principal point, a scale and the image dimension
  simple_camera_affine(const vector_3d &ray, const vector_3d &up,
                       const vector_3d &stare_pt, const vector_2d &pp,
                       const vector_2d &scale, unsigned int image_width,
                       unsigned int image_height);



  /// Construct an affine camera from a matrix and the image dimension
  simple_camera_affine(const matrix_3x4d &camera_matrix,
                       unsigned int image_width,
                       unsigned int image_height);



  /// Constructor - from base class
  simple_camera_affine(const camera_affine &base);

  /// Create of clone of this affine camera
  virtual camera_sptr clone() const {
    return camera_sptr(new simple_camera_affine(*this));
  }

  /// Set the camera matrix
  void set_matrix(const kwiver::vital::matrix_3x4d &new_camera_matrix);

  /// Get the camera matrix
  matrix_3x4d get_matrix() const { return P_; }

  /// Get the camera center (an ideal point)
  vector_4d center() const;

  /// Get/Set the distance from the origin along the ray
  double get_viewing_distance() const { return view_distance_; }
  void set_viewing_distance(double dist) { view_distance_ = dist; }

  /// Oient the camera ray direction so that the dot product with look_dir is positive
  void orient_ray_direction(const vector_3d &look_dir);

  /// Get the camera principal plane
  vector_4d principal_plane() const;

  /// Get image dimension
  unsigned int image_width() const { return image_width_; }
  unsigned int image_height() const { return image_height_; }

protected:
  /// Needed to assign a sense to the ray
  vector_3d ray_dir_;
  /// Camera projection matrix
  matrix_3x4d P_;
  /// Distance from the origin along the ray
  double view_distance_;
  /// Image dimensions
  unsigned int image_width_;
  unsigned int image_height_;
};


}
}   // end namespace vital


#endif // VITAL_CAMERA_AFFINE_H_
