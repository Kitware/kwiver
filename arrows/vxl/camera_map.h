/*ckwg +29
 * Copyright 2014-2016, 2019 by Kitware, Inc.
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
 * \brief Header file for a map from frame IDs to vpgl cameras
 */

#ifndef KWIVER_ARROWS_VXL_CAMERA_MAP_H_
#define KWIVER_ARROWS_VXL_CAMERA_MAP_H_

#include <vital/vital_config.h>
#include <arrows/vxl/kwiver_algo_vxl_export.h>

#include <vital/types/camera_map.h>

#include <vpgl/vpgl_perspective_camera.h>

#include <map>

namespace kwiver {
namespace arrows {
namespace vxl {

/// A concrete camera_map that wraps a map of vpgl_perspective_camera
class KWIVER_ALGO_VXL_EXPORT camera_map
: public vital::camera_map
{
public:
  /// typedef for a map of frame numbers to vpgl_perspective_camera
  typedef std::map<kwiver::vital::frame_id_t, vpgl_perspective_camera<double> > map_vcam_t;

  /// Default Constructor
  camera_map() {}

  /// Constructor from a std::map of vpgl_perspective_camera
  explicit camera_map(const map_vcam_t& cameras)
  : data_(cameras) {}

  /// Return the number of cameras in the map
  virtual size_t size() const { return data_.size(); }

  /// Return a map from integer IDs to camera shared pointers
  virtual map_camera_t cameras() const;

  /// Return underlying map from IDs to vpgl_perspective_camera
  virtual map_vcam_t vpgl_cameras() const { return data_; }

protected:

  /// The map from integer IDs to vpgl_perspective_camera
  map_vcam_t data_;
};


/// Convert any camera map to a vpgl camera map
KWIVER_ALGO_VXL_EXPORT
camera_map::map_vcam_t
camera_map_to_vpgl(const vital::camera_map& cam_map);


} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver

#endif
