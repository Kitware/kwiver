// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface for write_track_descriptor_set_csv

#ifndef KWIVER_ARROWS_WRITE_TRACK_DESCRIPTOR_SET_CSV_H
#define KWIVER_ARROWS_WRITE_TRACK_DESCRIPTOR_SET_CSV_H

#include <arrows/core/kwiver_algo_core_export.h>
#include <vital/vital_config.h>

#include <vital/algo/write_track_descriptor_set.h>

#include <memory>

namespace kwiver {

namespace arrows {

namespace core {

class KWIVER_ALGO_CORE_EXPORT write_track_descriptor_set_csv
  : public vital::algo::write_track_descriptor_set
{
public:
  PLUGGABLE_IMPL(
    write_track_descriptor_set_csv,
    "Track descriptor set csv writer.",
    PARAM_DEFAULT( write_raw_descriptor, bool, "write_raw_descriptor", true ),
    PARAM_DEFAULT( write_world_loc, bool, "write_world_loc", false )
  )

  virtual ~write_track_descriptor_set_csv();

  virtual bool check_configuration( vital::config_block_sptr config ) const;

  virtual void write_set(
    const kwiver::vital::track_descriptor_set_sptr set,
    const std::string& source_id ) override;

private:
  void initialize() override;

  class priv;
  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace core

} // namespace arrows

}     // end namespace

#endif // KWIVER_ARROWS_TRACK_DESCRIPTOR_SET_OUTPUT_CSV_H
