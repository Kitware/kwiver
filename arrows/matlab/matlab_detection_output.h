// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining matlab image object set writer

#ifndef KWIVER_VITAL_BINDINGS_MATLAB_DETECTION_OUTPUT_H_
#define KWIVER_VITAL_BINDINGS_MATLAB_DETECTION_OUTPUT_H_

#include <arrows/matlab/kwiver_algo_matlab_export.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/detected_object_set_output.h>

namespace kwiver {

namespace arrows {

namespace matlab {

class KWIVER_ALGO_MATLAB_EXPORT matlab_detection_output
  : public vital::algo::detected_object_set_output
{
public:
  PLUGGABLE_IMPL(
    matlab_detection_output,
    "Bridge to matlab detection output writer.",
    PARAM_DEFAULT(
      program_file, std::string,
      "File name of the matlab detection writer program to run.",
      "" ) )

  virtual ~matlab_detection_output();

  bool check_configuration( vital::config_block_sptr config ) const override;

  void write_set(
    const kwiver::vital::detected_object_set_sptr set,
    std::string const& image_name ) override;

protected:
  void initialize() override;
  void set_configuration_internal(
    vital::config_block_sptr config ) override;

private:
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace matlab

} // namespace arrows

}     // end namespace

#endif
