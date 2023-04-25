// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of merge_metadata_streams filter.

#ifndef KWIVER_ARROWS_CORE_MERGE_METADATA_STREAMS_H_
#define KWIVER_ARROWS_CORE_MERGE_METADATA_STREAMS_H_

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/metadata_filter.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
class KWIVER_ALGO_CORE_EXPORT merge_metadata_streams
  : public vital::algo::metadata_filter
{
public:
  merge_metadata_streams();
  virtual ~merge_metadata_streams();

  PLUGIN_INFO(
    "merge_metadata_streams",
    "Combines multiple metadata streams into exactly one." )

  vital::config_block_sptr get_configuration() const override;
  void set_configuration( vital::config_block_sptr config ) override;
  bool check_configuration( vital::config_block_sptr config ) const override;

  vital::metadata_vector filter(
    vital::metadata_vector const& input_metadata,
    vital::image_container_scptr const& input_image ) override;
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
