// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header for \link kwiver::vital::camera_rig camera_rig \endlink

#ifndef VITAL_CAMERA_RIG_H_
#define VITAL_CAMERA_RIG_H_

#include <camera.h>
#include <rotation.h>

#include <unordered_map>

namespace kwiver {
namespace vital {

/// forward declaration of camera_rig class
class camera_rig;
/// camera_rig shared pointer
typedef std::shared_ptr< camera_rig > camera_rig_sptr;
/// collection of cameras keyed by string tags
typedef std::unordered_map< std::string, camera_sptr > camera_collection;

// ----------------------------------------------------------------------------
/// An abstract representation of camera rig
///
/// The base class of camera rigs.
class VITAL_EXPORT camera_rig
{
public:
  virtual ~camera_rig() = default;

  /// Apply a rigid transform [rotation|translation] to the whole rig
  virtual void transform( const rotation_d & R, const vector_3d & t ) const = 0;

  /// Access cameras of the rig.
  /// \return camera collection
  virtual const camera_collection & cameras() const { return cameras_; }

  /// Add a camera.
  /// \return inserted camera
  virtual camera_sptr add(std::string const & tag, camera_sptr camera)
  {
    return cameras_[tag] = camera;
  }

  /// Remove a particular camera.
  /// \return removed camera
  virtual camera_sptr remove(std::string const & tag)
  {
    auto it = cameras_.find(tag);
    if (it==cameras_.end()) return nullptr;
    auto res = it->second;
    cameras_.erase(it);
    return res;
  }

  /// Access a particular camera.
  virtual camera_sptr camera(std::string const & tag)
  {
    auto it = cameras_.find(tag);
    return it==cameras_.end() ? nullptr : it->second;
  }

  /// Access a particular camera.
  virtual camera_sptr const camera(std::string const & tag) const
  {
    auto it = cameras_.find(tag);
    return it==cameras_.end() ? nullptr : it->second;
  }

protected:
  camera_rig() = default;
  camera_collection cameras_;
};

// ----------------------------------------------------------------------------
/// A representation of camera stereo rig
///
/// A camera stereo rig class.
class VITAL_EXPORT camera_rig_stereo: public camera_rig
{
public:
  camera_rig_stereo(camera_sptr left, camera_sptr right)
  {
    add("left", left);
    add("right", right);
  }
  virtual ~camera_rig_stereo() = default;
  camera_sptr left(){ return camera("left"); }
  camera_sptr right(){ return camera("right"); }
  camera_sptr const left()const { return camera("left"); }
  camera_sptr const right()const { return camera("right"); }
  /// Calibrate the stereo rig using 3D world points and corresponding left and right image points.
  /// \return was calibraiton successful?
  bool calibrate(
      std::vector< kwiver::vital::vector_2d > const& image_points_left,
      std::vector< kwiver::vital::vector_2d > const& image_points_right,
      std::vector< kwiver::vital::vector_3d > const& world_points
      )=0;
};

}
}   // vital

#endif // VITAL_CAMERA_RIG_H_
