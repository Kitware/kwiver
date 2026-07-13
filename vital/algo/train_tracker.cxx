// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief train_tracker algorithm definition instantiation

#include "train_tracker.h"

#include <vital/vital_config.h>

namespace kwiver {

namespace vital {

namespace algo {

train_tracker
::train_tracker()
{
  attach_logger( "algo.train_tracker" );
}

void
train_tracker
::add_data_from_disk(
  VITAL_UNUSED vital::category_hierarchy_sptr object_labels,
  VITAL_UNUSED std::vector< std::string > train_image_names,
  VITAL_UNUSED std::vector< kwiver::vital::object_track_set_sptr >
  train_groundtruth,
  VITAL_UNUSED std::vector< std::string > test_image_names,
  VITAL_UNUSED std::vector< kwiver::vital::object_track_set_sptr >
  test_groundtruth )
{
  throw std::runtime_error( "Method not implemented" );
}

void
train_tracker
::add_data_from_memory(
  VITAL_UNUSED vital::category_hierarchy_sptr object_labels,
  VITAL_UNUSED std::vector< kwiver::vital::image_container_sptr > train_images,
  VITAL_UNUSED std::vector< kwiver::vital::object_track_set_sptr >
  train_groundtruth,
  VITAL_UNUSED std::vector< kwiver::vital::image_container_sptr > test_images,
  VITAL_UNUSED std::vector< kwiver::vital::object_track_set_sptr >
  test_groundtruth )
{
  throw std::runtime_error( "Method not implemented" );
}

} // namespace algo

} // namespace vital

} // namespace kwiver
