// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief train_tracker algorithm definition

#ifndef VITAL_ALGO_TRAIN_TRACKER_H_
#define VITAL_ALGO_TRAIN_TRACKER_H_

#include <vital/algo/algorithm.h>
#include <vital/types/category_hierarchy.h>
#include <vital/types/object_track_set.h>
#include <vital/types/image_container.h>
#include <vital/vital_config.h>

namespace kwiver {

namespace vital {

namespace algo {

/// An abstract base class for training object trackers
class VITAL_ALGO_EXPORT train_tracker
  : public kwiver::vital::algorithm_def< train_tracker >
{
public:
  /// Return the name of this algorithm
  static std::string static_type_name() { return "train_tracker"; }

  /// Train a tracking model given a list of images and tracks
  ///
  /// This varient is geared towards offline training.
  ///
  /// \param object_labels object category labels for training
  /// \param train_image_list list of train image filenames
  /// \param train_groundtruth track annotations loaded for each sequence
  /// \param test_image_list list of test image filenames
  /// \param test_groundtruth track annotations loaded for each sequence
  virtual void
  add_data_from_disk(vital::category_hierarchy_sptr object_labels,
    std::vector< std::string > train_image_names,
    std::vector< kwiver::vital::object_track_set_sptr > train_groundtruth,
    std::vector< std::string > test_image_names = std::vector< std::string >(),
    std::vector< kwiver::vital::object_track_set_sptr > test_groundtruth
     = std::vector< kwiver::vital::object_track_set_sptr >());

  /// Train a tracking model given images and tracks
  ///
  /// This varient is geared towards online training, and is not required
  /// to be defined.
  ///
  /// \throws runtime_exception if not defined.
  ///
  /// \param object_labels object category labels for training
  /// \param train_images vector of input train images
  /// \param train_groundtruth track annotations loaded for each train sequence
  /// \param test_images optional vector of input test images
  /// \param test_groundtruth optional track annotations loaded for each test sequence
  virtual void
  add_data_from_memory(vital::category_hierarchy_sptr object_labels,
    std::vector< kwiver::vital::image_container_sptr > train_images,
    std::vector< kwiver::vital::object_track_set_sptr > train_groundtruth,
    std::vector< kwiver::vital::image_container_sptr > test_images
      = std::vector< kwiver::vital::image_container_sptr >(),
    std::vector< kwiver::vital::object_track_set_sptr > test_groundtruth
      = std::vector< kwiver::vital::object_track_set_sptr >());


  /// Train a tracking model given all loaded data
  ///
  /// This varient is geared towards either offline or online training
  /// depending on the implementation.
  ///
  /// \throws runtime_exception if not defined or there's a data issue.
  ///
  /// \param object_labels object category labels for training
  virtual void update_model() = 0;

protected:
  train_tracker();
};

/// Shared pointer for train_tracker algorithm definition class
typedef std::shared_ptr< train_tracker > train_tracker_sptr;

} // namespace algo

} // namespace vital

} // namespace kwiver

#endif // VITAL_ALGO_TRAIN_TRACKER_H_
