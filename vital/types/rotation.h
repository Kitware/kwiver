// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

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
  ///
  /// This constructor creates a rotation representing the orientation of an
  /// object as determined by the given \c yaw , \c pitch , and \c roll radian
  /// values. In accordance with standard YPR convention, \c roll rotates along
  /// the x axis, which is understood to be pointing north, \c pitch along the y
  /// axis (east), and \c yaw along the z axis (down). To convert from this
  /// North-East-Down coordinate system to one in which x, y, and z are facing
  /// east, north, and up, respectively, call the \link ned_to_enu() \endlink
  /// utility function on the constructed object.
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
  ///
  /// In accordance with standard YPR convention, \c roll rotates along
  /// the x axis, which is understood to be pointing north, \c pitch along the y
  /// axis (east), and \c yaw along the z axis (down). If this object currently
  /// uses the ENU coordinate system, you must call \link enu_to_ned() \endlink
  /// on it in order to get the correct values (NED is assumed).
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

/// Convert rotation \c r from ENU coordinate system to NED.
///
/// The East-North-Up coordinate system is the convention for specifying yaw,
/// pitch, and roll for aerial craft, while the North-East-Down coordinate
/// system is common for computer vision applications on aerial photos. This
/// function converts a rotation between the two conventions.
///
/// Inverse of \link ned_to_enu() \endlink.
///
/// \param r Rotation with the x axis pointing east, y axis pointing north,
///          and z axis pointing up.
///
/// \returns A copy of \c r with the x axis pointing north, y axis pointing
///          east, and z axis pointing down.
template < typename T >
VITAL_EXPORT
rotation_< T >
enu_to_ned( rotation_< T > const& r );

/// Convert rotation \c r from NED coordinate system to ENU.
///
/// The East-North-Up coordinate system is the convention for specifying yaw,
/// pitch, and roll for aerial craft, while the North-East-Down coordinate
/// system is common for computer vision applications on aerial photos. This
/// function converts a rotation between the two conventions.
///
/// Inverse of \link enu_to_ned() \endlink.
///
/// \param r Rotation with the x axis pointing north, y axis pointing east,
///          and z axis pointing down.
///
/// \returns A copy of \c r with the x axis pointing east, y axis pointing
///          north, and z axis pointing up.
template < typename T >
VITAL_EXPORT
rotation_< T >
ned_to_enu( rotation_< T > const& r );

/// Combine platform and sensor YPR from a UAS source into a single rotation.
///
/// This is a convenience function to compose the yaw, pitch, and roll of an
/// unmanned aerial system's platform and sensor into a single rotation object
/// in the ENU coordinate system.
///
/// \param platform_yaw z (down) rotation of the aerial platform
/// \param platform_pitch y (east) rotation of the aerial platform
/// \param platform_roll x (north) rotation of the aerial platform
/// \param sensor_yaw z (down) rotation of the sensor relative to the platform
/// \param sensor_pitch y (east) rotation of the sensor relative to the platform
/// \param sensor_roll x (north) rotation of the sensor relative to the platform
///
/// \returns The total rotation of the sensor in East-North-Up coordinates
template < typename T >
VITAL_EXPORT
rotation_< T >
uas_ypr_to_rotation( T platform_yaw, T platform_pitch, T platform_roll,
                     T sensor_yaw,   T sensor_pitch,   T sensor_roll );

} } // end namespace vital

#endif // VITAL_ROTATION_H_
