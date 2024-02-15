// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface for detected_object_set_output_csv

#ifndef KWIVER_ARROWS_METADATA_MAP_IO_CSV_H
#define KWIVER_ARROWS_METADATA_MAP_IO_CSV_H

#include <arrows/core/kwiver_algo_core_export.h>

#include <vital/algo/metadata_map_io.h>

#include <vital/plugin_management/pluggable_macro_magic.h>

#include <iostream>
#include <vector>
#include <string>

namespace kwiver {

namespace arrows {

namespace core {

class KWIVER_ALGO_CORE_EXPORT metadata_map_io_csv
  : public vital::algo::metadata_map_io
{
public:
  
  PLUGGABLE_IMPL(
    metadata_map_io_csv,
    "Metadata map writer using CSV format.",
    PARAM_DEFAULT(column_names, std::string, 
                   "Comma-separated values specifying column order. Can " 
                   "either be the enum names, e.g. VIDEO_KEY_FRAME or the " 
                   "description, e.g. 'Is frame a key frame'. For composite " 
                   "data types, index using '.', e.g. 'SENSOR_LOCATION.2' " 
                   "for sensor altitude.", 
                   ""), 
    PARAM_DEFAULT(column_overrides, std::string,
                  "Comma-separated values overriding the final column names" 
                  "as they appear in the output file. Order matches up with" 
                  "column_names.", 
                  ""), 
    PARAM_DEFAULT(write_enum_names, bool, 
                    "Write enum names rather than descriptive names",
                    false), 
    PARAM_DEFAULT(write_remaining_columns, bool, 
                    "Write columns present in the metadata but not in the " 
                    "manually-specified list.", 
                    true), 
    PARAM_DEFAULT(every_n_microseconds, uint64_t, 
                  "Minimum time between successive rows of output. Frames " 
                  "more frequent than this will be ignored. If nonzero, " 
                  "frames without a timestamp are also ignored.", 
                  0),
    PARAM_DEFAULT(every_n_frames, uint64_t, 
                 "Number of frames to skip between successive rows of " 
                  "output, plus one. A value of 1 will print every frame.", 
                  0))
  
  virtual ~metadata_map_io_csv();
  


  /// Unimplemented.
  ///
  /// \param filename the path to the file the load
  /// \throws kwiver::vital::file_write_exception not implemented
  kwiver::vital::metadata_map_sptr load_(
    std::istream& fin, std::string const& filename ) const override;

  /// Implementation specific save functionality.
  ///
  /// Save metadata to a CSV file. Uses the union of fields taken from all
  /// packets as the header, and inserts empty fields when values are missing
  /// for a given frame
  ///
  /// \param filename the path to the file to save
  /// \param data the metadata for a video to save
  void save_( std::ostream& fout,
              kwiver::vital::metadata_map_sptr data,
              std::string const& filename ) const override;

  /// Check supplied configuration.
  ///
  /// The options \c every_n_microseconds and \c every_n_frames cannot appear
  /// in the same configuration.
  bool check_configuration( vital::config_block_sptr config ) const override;

protected:
  void initialize() override;
  void set_configuration_internal(vital::config_block_sptr config) override;

private:
  class priv;
  KWIVER_UNIQUE_PTR(priv,d_);
};

} // namespace core

} // namespace arrows

} // namespace kwiver

#endif
