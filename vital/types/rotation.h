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
 * \brief Header for \link kwiver::vital::rotation_ rotation_<T> \endlink class
 */

#ifndef VITAL_ROTATION_H_
#define VITAL_ROTATION_H_

#include <iostream>
#include <vector>

#include <vital/vital_export.h>

#include <vital/types/matrix.h>
#include "vector.h"
#include <Eigen/Geometry>

namespace kwiver {
namespace vital {

/// A representation of 3D rotation.
/**
 * Internally, rotation is stored in quaternion form
 */
template < typename T >
class VITAL_EXPORT rotation_
{
public:
  /// Default Constructor
  rotation_< T > ( ) : q_( 1, 0, 0, 0 ) { }

  /// Constructor - from an Eigen Quaternion
  rotation_< T > ( const Eigen::Quaternion< T > &quaternion )
  : q_( quaternion ) { }

  /// Copy Constructor from another type
  template < typename U >
  explicit rotation_< T > ( const rotation_< U > &other )
  : q_( static_cast< Eigen::Quaternion< T > > ( other.quaternion() ) ) { }

  /// Constructor - from a 4D quaternion vector (x,y,z,w)
  /**
   * Note that the constructor for an Eigen:Quaternion from four scalars assumes
   * the order (w,x,y,z). However, internally it is stored in the following
   * order (x,y,z,w). Likewise, the constructor for an Eigen:Quaternion from an
   * array assumes the order (x,y,z,w).
   */
  //TODO: normalize quaternion. If the user provides a non-normalized quaterion,
  //It will remain so. This can cause problems when converting to other types.
  //Might want to consider using the actual Eigen::Quaternion constructor. This
  //will also resolve the strange order of the coefficients.
  explicit rotation_< T > ( const Eigen::Matrix< T, 4, 1 > &quaternion )
  : q_( quaternion ) { }

  /// Constructor - from a Rodrigues vector
  /**
   * A Rodrigues vector is a minimal parameterization of rotation where
   * the direction of the vector is the axis of rotation and the
   * magnitude of the vector is the angle of rotation (in radians).
   * This representation is closely related to the tangent space on
   * the manifold of the group of rotations.
   * \param rvec Rodrigues vector to construct from.
   */
  explicit rotation_< T > ( const Eigen::Matrix< T, 3, 1 > &rvec );

  /// Constructor - from rotation angle and axis
  rotation_< T > ( T angle, const Eigen::Matrix< T, 3, 1 > &axis );

  /// Constructor - from yaw, pitch, and roll (radians)
  /**
   * This constructor is intended for use with yaw, pitch, and roll (in radians)
   * output from an inertial navigation system, specifying the orientation of a
   * moving coordinate system relative to an east/north/up (ENU) coordinate
   * system. When all three angles are zero, the coordinate system's x, y, and
   * z axes align with north, east, and down respectively.  Non-zero yaw, pitch,
   * and roll define a sequence of intrinsic rotations around the z, y, and then
   * x axes respectively.  The resulting rotation object takes a vector in ENU
   * and rotates it into the moving coordinate system.
   */
  rotation_< T > ( const T &yaw, const T &pitch, const T &roll );

  /// Constructor - from a matrix
  /**
   * requires orthonormal matrix with +1 determinant
   * \param rot orthonormal matrix to construct from
   */
  explicit rotation_< T > ( const Eigen::Matrix< T, 3, 3 > &rot );

  /// Convert to a 3x3 matrix
  Eigen::Matrix< T, 3, 3 > matrix() const;

  /// Returns the axis of rotation
  /**
   * \note axis is undefined for the identity rotation,
   *       returns (0,0,1) in this case.
   * \sa angle()
   */
  Eigen::Matrix< T, 3, 1 > axis() const;

  /// Returns the angle of the rotation in radians about the axis
  /**
   * \sa axis()
   */
  T angle() const;

  /// Access the quaternion as a 4-vector
  /**
   * The first component is real, the last 3 are imaginary (i,j,k)
   */
  Eigen::Quaternion< T > quaternion() const { return q_; }

  /// Return the rotation as a Rodrigues vector
  Eigen::Matrix< T, 3, 1 > rodrigues() const;

  /// Convert to yaw, pitch, and roll (radians)
  void get_yaw_pitch_roll( T& yaw, T& pitch, T& roll ) const;

  /// Compute the inverse rotation
  rotation_< T > inverse() const
  {
    return rotation_< T > ( q_.inverse() );
  }

  /// Compose two rotations
  rotation_< T > operator*( const rotation_< T >& rhs ) const;

  /// Rotate a vector
  /**
   * \note for a large number of vectors, it is more efficient to
   *       create a rotation matrix and use matrix multiplication
   * \param rhs right-hand side vector to operate against
   */
  Eigen::Matrix< T, 3, 1 > operator*( const Eigen::Matrix< T, 3, 1 >& rhs ) const;

  /// Equality operator
  /**
   * TODO: two quaternions can represent the same rotation but have different
   * components. The test is to calculate the product of the first rotation with
   * the inverse of the second to calculate the difference rotation. Convert the
   * difference rotation to axis and angle form, and if the angle is greater
   * than some threshold, they should not be considered equal.
   */
  inline bool operator==( const rotation_< T >& rhs ) const
  {
    return this->q_.coeffs() == rhs.q_.coeffs();
  }

  /// Inequality operator
  inline bool operator!=( const rotation_< T >& rhs ) const
  {
    return ! ( *this == rhs );
  }

protected:
  /// rotation stored internally as a quaternion vector
  Eigen::Quaternion< T > q_;
};


/// Double-precision rotation_ type
typedef rotation_< double > rotation_d;
/// Single-precision rotation_ type
typedef rotation_< float > rotation_f;


/// output stream operator for a rotation
template < typename T >
VITAL_EXPORT std::ostream&  operator<<( std::ostream& s, const rotation_< T >& r );

/// input stream operator for a rotation
template < typename T >
VITAL_EXPORT std::istream&  operator>>( std::istream& s, rotation_< T >& r );


// Generate an interpolated rotation between \c A and \c B by a given fraction
/**
 * \c f must be 0 < \c f < 1
 *
 * TODO: Have this raise an exception when f is not within the valid range.
 *
 * \param A Rotation we are interpolating from.
 * \param B Rotation we are interpolating towards.
 * \param f Fractional value describing the interpolation point between A and B.
 * \returns A rotation in between A and B to a degree proportional to the given
 *          fraction.
 */
template < typename T >
VITAL_EXPORT
rotation_< T >
interpolate_rotation( rotation_< T > const& A, rotation_< T > const& B, T f );


/// Generate N evenly interpolated rotations in between \c A and \c B.
/**
 * \c n must be >= 1.
 *
 * \param[in]   A           Rotation we are interpolating from.
 * \param[in]   B           Rotation we are interpolating towards,
 * \param[in]   n           Number of even interpolations in between A and B to generate.
 * \param[out]  interp_rots Interpolated rotations are added to this vector in
 *                          in order of generation in the A -> B direction.
 *
 * \returns A vector of \c n evenly interpolated rotations in order between A
 *          and B.
 */
template < typename T >
VITAL_EXPORT
void interpolated_rotations( rotation_< T > const& A, rotation_< T > const& B,
                             size_t n, std::vector< rotation_< T > >& interp_rots );

// TODO: Consider moving this method to a utilities header/directory
/// Compose an aerial platform's orientation with sensor orientation
/**
 * \param platform_yaw yaw angle for aerial platform
 * \param platform_pitch pitch angle for aerial platform
 * \param platform_roll roll angle for aerial platform
 * \param sensor_yaw yaw angle for aerial sensor
 * \param sensor_pitch pitch angle for aerial sensor
 * \param sensor_roll roll angle for aerial sensor
 */
template < typename T >
VITAL_EXPORT
rotation_< T > compose_rotations(
  T platform_yaw, T platform_pitch, T platform_roll,
  T sensor_yaw, T sensor_pitch, T sensor_roll );

} } // end namespace vital

#endif // VITAL_ROTATION_H_
