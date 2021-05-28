// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Header file for a map from frame IDs to cameras
 */

#ifndef VITAL_CAMERA_MAP_H_
#define VITAL_CAMERA_MAP_H_

#include "camera.h"

#include <vital/vital_types.h>
#include <vital/vital_config.h>

#include <set>
#include <map>
#include <memory>

namespace kwiver {
namespace vital {

/// An abstract mapping between frame IDs and cameras
class camera_map
{
public:
  /// typedef for std::map from integer IDs to cameras
  typedef std::map< frame_id_t, camera_sptr > map_camera_t;

  /// Destructor
  virtual ~camera_map() = default;

  /// Return the number of cameras in the map
  virtual size_t size() const = 0;

  /// Return a map from integer IDs to camera shared pointers
  virtual map_camera_t cameras() const = 0;
};

/// typedef for a camera shared pointer
typedef std::shared_ptr< camera_map > camera_map_sptr;

/// A concrete camera_map that simply wraps a std::map.
class simple_camera_map :
  public camera_map
{
public:
  /// Default Constructor
  simple_camera_map() { }

  /// Constructor from a std::map of cameras
  explicit simple_camera_map( map_camera_t const& cameras )
    : data_( cameras ) { }

  /// Return the number of cameras in the map
  virtual size_t size() const { return data_.size(); }

  /// Return a map from integer IDs to camera shared pointers
  virtual map_camera_t cameras() const { return data_; }

protected:
  /// The map from integer IDs to camera shared pointers
  map_camera_t data_;
};

template<class T>
class camera_map_of_;

template<class T>
using camera_map_of_sptr = std::shared_ptr<camera_map_of_<T>>;

/// A concrete camera_map that simply wraps a std::map.
template<class T>
class camera_map_of_ :
  public camera_map
{
public:
  typedef std::map< frame_id_t, std::shared_ptr<T> > frame_to_T_sptr_map;

  /// Default Constructor
  camera_map_of_() { }

  /// Constructor from a std::map of cameras
  explicit camera_map_of_(frame_to_T_sptr_map const& cameras)
    : data_(cameras) { }

  /// Return the number of cameras in the map
  virtual size_t size() const { return data_.size(); }

  /// Return a map from integer IDs to camera shared pointers
  virtual map_camera_t cameras() const
  {
    map_camera_t ret_map;
    for (auto const& d : data_)
    {
      ret_map[d.first] = d.second;
    }
    return ret_map;
  }

  /// Return the frame ids in the map
  virtual std::set<frame_id_t> get_frame_ids() const
  {
    std::set<frame_id_t> cam_ids;
    for (auto const& d : data_)
    {
      cam_ids.insert(d.first);
    }
    return cam_ids;
  }

  /// Find a camera in the map
  /**
  * \param [in] fid the frame id of the camera to return
  * \return     the camera if found or a null camera if it is not found
  */
  std::shared_ptr<T> find(frame_id_t fid) const
  {
    auto it = data_.find(fid);
    if (it == data_.end())
    {
      return std::shared_ptr<T>();
    }
    else
    {
      return it->second;
    }
  }

  /// Erase a camera from the map
  /**
  * \param [in] fid the frame id of the camera to erase
  */
  void erase(frame_id_t fid)
  {
    data_.erase(fid);
  }

  /// Insert a camera into the map
  /**
  * \param [in] fid the frame id of the camera to insert
  * \param [in] cam the camera to insert
  */
  //@{
  void insert(frame_id_t fid, std::shared_ptr<T> const& cam)
  {
    data_[fid] = cam;
  }

  void insert(frame_id_t fid, std::shared_ptr<T>&& cam)
  {
    data_[fid] = std::move(cam);
  }
  //@}

  /// Clear the map of all cameras
  void clear()
  {
    data_.clear();
  }

  /// Set the map from a map of base cameras.
  /**
  * Only simple perspective cameras will be added to the map.  All
  * others are ignored.  The map is emptied before the cameras are added.
  * \param [in] base_cams the cams to add
  */
  void set_from_base_cams(camera_map_sptr base_cams)
  {
    auto base_cams_map = base_cams->cameras();
    set_from_base_camera_map(base_cams_map);
  }

  /// Set the map from a map of base cameras.
  /**
  * Only simple perspective cameras will be added to the map.  All
  * others are ignored.
  * \param [in] base_cams_map the map of cams to add
  */
  void set_from_base_camera_map(const camera_map::map_camera_t &base_cams_map)
  {
    clear();
    for (auto &c : base_cams_map)
    {
      auto pc = std::dynamic_pointer_cast<T>(c.second);
      if (pc)
      {
        data_[c.first] = pc;
      }
    }
  }

  /// Create a clone of the map cloning each cameara in the map
  camera_map_of_sptr<T> clone()
  {
    auto the_clone = std::make_shared<camera_map_of_<T>>();
    for (auto &d : data_)
    {
      the_clone->insert(d.first, std::static_pointer_cast<T>(d.second->clone()));
    }
    return the_clone;
  }

  /// Convert to a camera map of a type B for which B is a base class of T
  template <typename B>
  std::map< frame_id_t, std::shared_ptr<B> > map_of_()
  {
    std::map< frame_id_t, std::shared_ptr<B> > new_map;
    for (auto &d : data_)
    {
      new_map[d.first] = d.second;
    }
    return new_map;
  }

  /// return a map from integer IDs to simple perspective camera shared pointers
  virtual frame_to_T_sptr_map const& T_cameras() const { return data_; }

protected:
  /// The map from integer IDs to camera shared pointers
  frame_to_T_sptr_map data_;

};

}} // end namespace vital

#endif // VITAL_CAMERA_MAP_H_
