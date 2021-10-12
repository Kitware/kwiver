/*ckwg +29
 * Copyright 2014-2015 by Kitware, Inc.
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
 * \brief Implementation for conversions between vital and vpgl cameras
 */

#include "camera.h"

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace vxl {

/// Construct a camera_sptr from a vpgl_perspective_camera
template <typename T>
camera_perspective_sptr
vpgl_camera_to_vital(const vpgl_perspective_camera<T>& vcam)
{
  simple_camera_perspective* mcam = new simple_camera_perspective;
  vpgl_camera_to_vital(vcam, *mcam);
  return camera_perspective_sptr(mcam);
}


/// Convert a vpgl_perspective_camera to a vital::camera_
template <typename T>
void
vpgl_camera_to_vital(const vpgl_perspective_camera<T>& vcam,
                     simple_camera_perspective& mcam)
{
  simple_camera_intrinsics mk;
  vpgl_calibration_to_vital(vcam.get_calibration(), mk);
  mcam.set_intrinsics(mk.clone());

  const vnl_quaternion<T>& vr = vcam.get_rotation().as_quaternion();
  mcam.set_rotation(rotation_d(rotation_<T>(Eigen::Quaternion<T>(vr.r(), vr.x(), vr.y(), vr.z()))));

  const vgl_point_3d<T>& vc = vcam.get_camera_center();
  mcam.set_center(vital::vector_3d(vc.x(), vc.y(), vc.z()));
}


/// Convert a vital::camera_ to a vpgl_perspective_camera
template <typename T>
void
vital_to_vpgl_camera(const camera_perspective& mcam,
                     vpgl_perspective_camera<T>& vcam)
{
  vpgl_calibration_matrix<T> vk;
  vital_to_vpgl_calibration(*mcam.intrinsics(), vk);
  vcam.set_calibration(vk);

  const Eigen::Quaternion<double>& mrd = mcam.rotation().quaternion();
  const Eigen::Quaternion<T> mr(mrd);
  vcam.set_rotation(vgl_rotation_3d<T>(vnl_quaternion<T>(mr.x(), mr.y(),
                                                         mr.z(), mr.w())));

  const vital::vector_3d& mc = mcam.center();
  vcam.set_camera_center(vgl_point_3d<T>(static_cast<T>(mc.x()),
                                         static_cast<T>(mc.y()),
                                         static_cast<T>(mc.z())));
}


/// Convert a vpgl_calibration_matrix to a vital::camera_intrinsics_
template <typename T>
void
vpgl_calibration_to_vital(const vpgl_calibration_matrix<T>& vcal,
                          simple_camera_intrinsics& mcal)
{
  vgl_point_2d<T> vpp = vcal.principal_point();
  mcal = simple_camera_intrinsics(vcal.focal_length() * vcal.x_scale(),
                                  vector_2d(vpp.x(), vpp.y()),
                                  vcal.x_scale() / vcal.y_scale(),
                                  vcal.skew());
}


/// Convert a vital::camera_intrinsics_ to a vpgl_calibration_matrix
template <typename T>
void
vital_to_vpgl_calibration(const camera_intrinsics& mcal,
                          vpgl_calibration_matrix<T>& vcal)
{
  const vector_2d& mpp = mcal.principal_point();
  vcal = vpgl_calibration_matrix<T>(static_cast<T>(mcal.focal_length()),
                                    vgl_point_2d<T>(static_cast<T>(mpp.x()),
                                                    static_cast<T>(mpp.y())),
                                    1, static_cast<T>(1.0 / mcal.aspect_ratio()),
                                    static_cast<T>(mcal.skew()));
}



/// \cond DoxygenSuppress
#define INSTANTIATE_CAMERA(T) \
template KWIVER_ALGO_VXL_EXPORT camera_perspective_sptr vpgl_camera_to_vital(const vpgl_perspective_camera<T>& vcam); \
template KWIVER_ALGO_VXL_EXPORT void vpgl_camera_to_vital(const vpgl_perspective_camera<T>&, simple_camera_perspective&); \
template KWIVER_ALGO_VXL_EXPORT void vital_to_vpgl_camera(const camera_perspective&, vpgl_perspective_camera<T>&); \
template KWIVER_ALGO_VXL_EXPORT void vpgl_calibration_to_vital(const vpgl_calibration_matrix<T>& vcal, \
                                                         simple_camera_intrinsics& mcal); \
template KWIVER_ALGO_VXL_EXPORT void vital_to_vpgl_calibration(const camera_intrinsics& mcal, \
                                                         vpgl_calibration_matrix<T>& vcal)

INSTANTIATE_CAMERA(double);
INSTANTIATE_CAMERA(float);

#undef INSTANTIATE_CAMERA
/// \endcond

} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver
