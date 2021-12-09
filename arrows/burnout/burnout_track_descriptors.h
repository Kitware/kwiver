// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#ifndef KWIVER_ARROWS_BURNOUT_TRACK_DESCRIPTORS
#define KWIVER_ARROWS_BURNOUT_TRACK_DESCRIPTORS

#include <arrows/burnout/kwiver_algo_burnout_export.h>

#include <vital/algo/compute_track_descriptors.h>

namespace kwiver {
namespace arrows {
namespace burnout {

// ----------------------------------------------------------------
/// @brief burnout_track_descriptors
///
class KWIVER_ALGO_BURNOUT_EXPORT burnout_track_descriptors
  : public vital::algo::compute_track_descriptors
{
public:
  burnout_track_descriptors();
  virtual ~burnout_track_descriptors();

  PLUGIN_INFO( "burnout",
               "Track descriptors using burnout" )

  vital::config_block_sptr get_configuration() const override;

  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  kwiver::vital::track_descriptor_set_sptr
  compute( kwiver::vital::timestamp ts,
           kwiver::vital::image_container_sptr image_data,
           kwiver::vital::object_track_set_sptr tracks ) override;

  kwiver::vital::track_descriptor_set_sptr flush() override;

private:

  class priv;
  const std::unique_ptr<priv> d;
};

} } }

#endif // KWIVER_ARROWS_BURNOUT_TRACK_DESCRIPTORS
