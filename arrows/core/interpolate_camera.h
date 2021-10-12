/*ckwg +29
 * Copyright 2013-2016 by Kitware, Inc.
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
 * \brief Header for camera interpolation functions
 */

#ifndef ALGORITHMS_INTERPOLATE_CAMERA_H_
#define ALGORITHMS_INTERPOLATE_CAMERA_H_


#include <vital/vital_config.h>
#include <arrows/core/kwiver_algo_core_export.h>

#include <vector>
#include <vital/types/camera_perspective.h>


namespace kwiver {
namespace arrows {


/// Generate an interpolated camera between \c A and \c B by a given fraction \c f
/**
 * \c f should be 0 < \c f < 1. A value outside this range is valid, but \c f
 * must not be 0.
 *
 * \param A Camera to interpolate from.
 * \param B Camera to interpolate to.
 * \param f Decimal fraction in between A and B for the returned camera to represent.
 */
KWIVER_ALGO_CORE_EXPORT
vital::simple_camera_perspective
interpolate_camera(vital::simple_camera_perspective const& A,
                   vital::simple_camera_perspective const& B, double f);


/// Genreate an interpolated camera from sptrs
/**
 * \relatesalso interpolate_camera
 *
 */
KWIVER_ALGO_CORE_EXPORT
vital::camera_perspective_sptr
interpolate_camera(vital::camera_perspective_sptr A,
                   vital::camera_perspective_sptr B, double f);


/// Generate N evenly interpolated cameras in between \c A and \c B
/**
 * \c n must be >= 1.
 */
KWIVER_ALGO_CORE_EXPORT
void interpolated_cameras(vital::simple_camera_perspective const& A,
                          vital::simple_camera_perspective const& B,
                          size_t n,
                          std::vector< vital::simple_camera_perspective > & interp_cams);


} // end namespace arrows
} // end namespace kwiver

#endif // ALGORITHMS_INTERPOLATE_CAMERA_H_
