// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for \link kwiver::vital::camera_perspective camera_perspective \endlink and
///        \link kwiver::vital::camera_perspective_ camera_perspective_<T> \endlink classes

#ifndef VITAL_CAMERA_PERSPECTIVE_H_
#define VITAL_CAMERA_PERSPECTIVE_H_

#include <vital/vital_export.h>

#include <iostream>
#include <vector>
#include <memory>

#include <vital/vital_config.h>
#include <vital/types/camera.h>
#include <vital/types/camera_intrinsics.h>
#include <vital/types/covariance.h>
#include <vital/types/rotation.h>
#include <vital/types/vector.h>
#include <vital/types/similarity.h>
#include <vital/logger/logger.h>

namespace kwiver {
namespace vital {

/// forward declaration of perspective camera class
class camera_perspective;
/// typedef for a camera_perspective shared pointer
typedef std::shared_ptr< camera_perspective > camera_perspective_sptr;

// ------------------------------------------------------------------
/// An abstract representation of perspective camera
///
/// The base class of camera_perspectives is abstract and provides a
/// double precision interface.  The templated derived class
/// can store values in either single or double precision.
class VITAL_EXPORT camera_perspective : public camera
{
public:
  /// Destructor
  virtual ~camera_perspective() = default;

  /// Create a clone of this camera_perspective object
  virtual camera_sptr clone() const = 0;

  /// Accessor for the camera center of projection (position)
  virtual vector_3d center() const = 0;
  /// Accessor for the translation vector
  virtual vector_3d translation() const = 0;
  /// Accessor for the covariance of camera center
  virtual covariance_3d center_covar() const = 0;
  /// Accessor for the rotation
  virtual rotation_d rotation() const = 0;
  /// Accessor for the intrinsics
  virtual camera_intrinsics_sptr intrinsics() const = 0;
  /// Accessor for the image width
  virtual unsigned int image_width() const { return intrinsics()->image_width(); }
  /// Accessor for the image height
  virtual unsigned int image_height() const { return intrinsics()->image_height(); }

  /// Create a clone of this camera that is rotated to look at the given point
  ///
  /// \param stare_point the location at which the camera is oriented to point
  /// \param up_direction the vector which is "up" in the world (defaults to Z-axis)
  /// \returns New clone, but set to look at the given point.
  virtual camera_perspective_sptr clone_look_at(
    const vector_3d &stare_point,
    const vector_3d &up_direction = vector_3d::UnitZ() ) const = 0;

  /// Convert to a 3x4 homogeneous projection matrix
  ///
  ///  \note This matrix representation does not account for lens distortion
  ///  models that may be used in the camera_intrinsics
  virtual matrix_3x4d as_matrix() const;

  /// Convert to a 3x4 pose matrix (no intrinsics)
  ///
  /// \note This matrix representation does not account for camera intrisics
  /// and only models the extinsic pose of the camera.
  matrix_3x4d pose_matrix() const;

  /// Project a 3D point into a 2D image point
  virtual vector_2d project( const vector_3d& pt ) const;

  /// Compute the distance of the 3D point to the image plane
  ///
  ///  Points with negative depth are behind the camera
  virtual double depth(const vector_3d& pt) const;

protected:
  camera_perspective();

  kwiver::vital::logger_handle_t m_logger;

};

/// output stream operator for a base class camera_perspective
VITAL_EXPORT std::ostream& operator<<( std::ostream& s,
                                       const camera_perspective& c );

/// forward declaration of simple perspective camera class
class simple_camera_perspective;
/// typedef for a simple_camera_perspective shared pointer
typedef std::shared_ptr< simple_camera_perspective > simple_camera_perspective_sptr;

/// A representation of a camera
///
/// Contains camera location, orientation, and intrinsics
class VITAL_EXPORT simple_camera_perspective :
  public camera_perspective
{
public:
  /// Default Constructor
  simple_camera_perspective ( )
  : center_( 0.0, 0.0, 0.0 ),
  orientation_(),
  intrinsics_( new simple_camera_intrinsics() )
  { }

  /// Constructor - from camera center, rotation, and intrinsics
  ///
  ///  This constructor keeps a shared pointer to the camera intrinsics object
  ///  passed in, unless it is null.  If null it creates a new simple_camera_intrinsics
  simple_camera_perspective (
                  const vector_3d &center,
                  const rotation_d &rotation,
                  camera_intrinsics_sptr intrinsics = camera_intrinsics_sptr() )
  : center_( center ),
  orientation_( rotation ),
  intrinsics_( !intrinsics
               ? camera_intrinsics_sptr(new simple_camera_intrinsics())
               : intrinsics )
  { }

  /// Constructor - from camera_perspective center, rotation, and intrinsics
  ///
  ///  This constructor make a clone of the camera_perspective intrinsics object passed in
  simple_camera_perspective ( const vector_3d &center,
                              const rotation_d &rotation,
                              const camera_intrinsics& intrinsics )
  : center_( center ),
  orientation_( rotation ),
  intrinsics_( intrinsics.clone() )
  { }

  /// Constructor - from base class
  simple_camera_perspective( const camera_perspective &base )
  : center_( base.center() ),
    center_covar_( base.center_covar() ),
    orientation_( base.rotation() ),
    intrinsics_( base.intrinsics() )
  {}

  /// Create a clone of this camera object
  virtual camera_sptr clone() const
  { return camera_sptr( new simple_camera_perspective( *this ) ); }

  /// Accessor for the camera center of projection (position)
  virtual vector_3d center() const
  { return center_; }

  /// Accessor for the translation vector
  virtual vector_3d translation() const
  { return -( orientation_ * center_ ); }

  /// Accessor for the covariance of camera center
  virtual covariance_3d center_covar() const
  { return center_covar_; }

  /// Accessor for the rotation
  virtual rotation_d rotation() const
  { return orientation_; }

  /// Accessor for the intrinsics
  virtual camera_intrinsics_sptr intrinsics() const
  { return intrinsics_; }

  /// Create a clone of this camera that is rotated to look at the given point
  ///
  /// This implementation creates a clone and call look_at on it.
  ///
  /// \param stare_point the location at which the camera is oriented to point
  /// \param up_direction the vector which is "up" in the world (defaults to Z-axis)
  /// \returns New clone, but set to look at the given point.
  virtual camera_perspective_sptr clone_look_at(
    const vector_3d &stare_point,
    const vector_3d &up_direction = vector_3d::UnitZ() ) const override;

  /// Accessor for the camera center of projection using underlying data type
  const vector_3d& get_center() const { return center_; }

  /// Accessor for the covariance of camera center using underlying data type
  const covariance_3d& get_center_covar() const { return center_covar_; }

  /// Accessor for the rotation using underlying data type
  const rotation_d& get_rotation() const { return orientation_; }

  /// Accessor for the intrinsics using underlying data type
  camera_intrinsics_sptr get_intrinsics() const { return intrinsics_; }

  /// Set the camera center of projection
  void set_center( const vector_3d& center ) { center_ = center; }

  /// Set the translation vector (relative to current rotation)
  void set_translation( const vector_3d& translation )
  {
    center_ = -( orientation_.inverse() * translation );
  }

  /// Set the covariance matrix of the feature
  void set_center_covar( const covariance_3d& center_covar )
  {
    center_covar_ = center_covar;
  }

  /// Set the rotation
  void set_rotation( const rotation_d& rotation ) { orientation_ = rotation; }

  /// Set the intrinsics
  //@{
  void set_intrinsics( const camera_intrinsics_sptr& intrinsics )
  {
    intrinsics_ = ! intrinsics
                  ? camera_intrinsics_sptr(new simple_camera_intrinsics())
                  : intrinsics;
  }
  void set_intrinsics( camera_intrinsics_sptr&& intrinsics )
  {
    intrinsics_ = ! intrinsics
                  ? std::make_shared< simple_camera_intrinsics >()
                  : std::move( intrinsics );
  }
  //@}

  /// Rotate the camera about its center such that it looks at the given point.
  ///
  /// The camera should also be rotated about its principal axis such that
  /// the vertical image direction is closest to \c up_direction in the world.
  ///
  /// \param stare_point the location at which the camera is oriented to point
  /// \param up_direction the vector which is "up" in the world (defaults to Z-axis)
  void look_at( const vector_3d &stare_point,
                const vector_3d &up_direction = vector_3d::UnitZ() );

protected:
  /// The camera center of project
  vector_3d center_;
  /// The covariance of the camera center location
  covariance_3d center_covar_;
  /// The camera rotation
  rotation_d orientation_;
  /// The camera intrinics
  camera_intrinsics_sptr intrinsics_;
};

/// input stream operator for a camera_perspective
///
/// \param s input stream
/// \param c camera_perspective to stream into
VITAL_EXPORT std::istream& operator>>( std::istream& s,
                                       simple_camera_perspective& c );

}
}   // end namespace vital

#endif // VITAL_CAMERA_PERSPECTIVE_H_
