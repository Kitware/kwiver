// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief C Interface to \p vital::camera class

#ifndef VITAL_C_CAMERA_H_
#define VITAL_C_CAMERA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <vital/bindings/c/common.h>
#include <vital/bindings/c/error_handle.h>
#include <vital/bindings/c/types/camera_intrinsics.h>
#include <vital/bindings/c/types/covariance.h>
#include <vital/bindings/c/types/eigen.h>
#include <vital/bindings/c/types/rotation.h>
#include <vital/bindings/c/vital_c_export.h>

/// Opaque structure to a vital::camera class
typedef struct vital_camera_s vital_camera_t;

/// Destroy a vital_camera_t instance
///
/// The given might not refer to a valid camera instance, causing the error
/// handle to be populated (code -1).
///
/// \param cam Camera instance pointer to destroy
/// \param eh Vital error handle instance
VITAL_C_EXPORT
void vital_camera_destroy( vital_camera_t *cam,
                           vital_error_handle_t *eh );

/// Create a new simple camera
///
/// Input instances are copied.
///
/// \param center 3D center coordinate of the camera
/// \param rotation rotation instance to use
/// \param intrinsics camera intrinsics to use
/// \param eh Vital error handle instance.
/// \return New reference to the created instance.
VITAL_C_EXPORT
vital_camera_t*
vital_camera_new( vital_eigen_matrix3x1d_t const *center,
                  vital_rotation_d_t const *rotation,
                  vital_camera_intrinsics_t const *intrinsics,
                  vital_error_handle_t *eh );

/// Create a new simple camera instance with default parameters
///
/// \param eh Vital error handle instance.
/// \return New reference to the created instance.
VITAL_C_EXPORT
vital_camera_t*
vital_camera_new_default( vital_error_handle_t *eh );

/// Create a new simple camera from a string
///
/// String input is expected to be of the format that would be produced by the
/// `vital_camera_to_string` function.
///
/// \param s String create the new camera from.
/// \param eh Vital error handle instance.
/// \return New reference to the created instance.
VITAL_C_EXPORT
vital_camera_t*
vital_camera_new_from_string( char const *s, vital_error_handle_t *eh );

/// Clone the given camera instance, returning a new camera instance
///
/// \param cam Camera instance to clone.
/// \param eh Vital error handle instance.
/// \returns New camera instance that is the clone of the input camera.
VITAL_C_EXPORT
vital_camera_t*
vital_camera_clone( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Get the 3D center point of the camera as a new 3x1 matrix (column-vector)
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance.
/// \returns New 3D center coordinate.
VITAL_C_EXPORT
vital_eigen_matrix3x1d_t*
vital_camera_center( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Get the 3D translation vector of the camera as a new 3x1 matrix (column-vector)
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance.
/// \returns New 3D translation vector.
VITAL_C_EXPORT
vital_eigen_matrix3x1d_t*
vital_camera_translation( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Get the covariance of the camera center as a new vital covariance instance
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance.
/// \returns New covariance instance
VITAL_C_EXPORT
vital_covariance_3d_t*
vital_camera_center_covar( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Get rotation of the camera as a new vital rotation instance
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance.
/// \returns new rotation instance
VITAL_C_EXPORT
vital_rotation_d_t*
vital_camera_rotation( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Get new reference to the shared intrinsics instance of the camera
///
/// Camera intrinsics are reference counted, so the returned reference should be
/// destroyed using `vital_camera_intrinsics_destroy` despite not being a copy.
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance.
/// \returns New reference to the camera's intrinsics instance.
VITAL_C_EXPORT
vital_camera_intrinsics_t*
vital_camera_intrinsics( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Create a clone of this camera that is rotated to look at the given point
///
/// The camera should also be rotated about its principal axis such that
/// the vertical image direction is closest to \c up_direction in the world.
///
/// \param stare_point the location at which the camera is oriented to point
/// \param up_direction the vector which is "up" in the world (defaults to Z-axis)
/// \returns New camera instance that is the clone of the one given, but set to
///          look at the given point.
VITAL_C_EXPORT
vital_camera_t*
vital_camera_clone_look_at( vital_camera_t const *cam,
                            vital_eigen_matrix3x1d_t const *stare_point,
                            vital_eigen_matrix3x1d_t const *up_direction,
                            vital_error_handle_t *eh );

/// Convert camera to a 3x4 homogeneous projection matrix instance
///
/// \note This matrix representation does not account for lens distortion
///  models that may be used in the camera intrinsics.
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance.
/// \returns New Eigen 3x4 matrix instance
VITAL_C_EXPORT
vital_eigen_matrix3x4d_t*
vital_camera_as_matrix( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Project a 3D point into a (new) 2D image point via the given camera
///
/// \param cam Camera instance to use
/// \param pt 3D coordinate to transform.
/// \param eh Vital error handle instance.
/// \returns New 2D coordinate vector
VITAL_C_EXPORT
vital_eigen_matrix2x1d_t*
vital_camera_project( vital_camera_t const *cam,
                      vital_eigen_matrix3x1d_t const *pt,
                      vital_error_handle_t *eh );

/// Compute the distance of the 3D point to the image plane
///
/// Points with negative depth are behind the camera
///
/// \param cam Camera instance to use
/// \param pt 3D coordinate to vet the depth of.
/// \param eh Vital error handle instance.
/// \returns Distance value
VITAL_C_EXPORT
double
vital_camera_depth( vital_camera_t const *cam,
                    vital_eigen_matrix3x1d_t const *pt,
                    vital_error_handle_t *eh );

/// Convert the camera into a new string representation
///
/// \param cam Camera instance to use
/// \param eh Vital error handle instance
/// \returns New character string
VITAL_C_EXPORT
char*
vital_camera_to_string( vital_camera_t const *cam, vital_error_handle_t *eh );

/// Read in a KRTD file, producing a new vital::camera object
///
/// \param filepath Path to the KRTD file to use
/// \param eh Vital error handle instance.
/// \returns new camera instance
VITAL_C_EXPORT
vital_camera_t*
vital_camera_read_krtd_file( char const *filepath,
                             vital_error_handle_t *eh );

/// Output the given vital_camera_t object to the specified file path
///
/// \param cam Camera instance to use
/// \param filepath path to the KRTD file to write to
/// \param eh Vital error handle instance.
VITAL_C_EXPORT
void
vital_camera_write_krtd_file( vital_camera_t const *cam,
                              char const *filepath,
                              vital_error_handle_t *eh );

#ifdef __cplusplus
}
#endif

#endif // VITAL_C_CAMERA_H_
