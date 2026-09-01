// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining matlab image_filter

#ifndef VITAL_BINDINGS_MATLAB_IMAGE_FILTER_H
#define VITAL_BINDINGS_MATLAB_IMAGE_FILTER_H

#include <arrows/matlab/kwiver_algo_matlab_export.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/image_filter.h>

namespace kwiver {

namespace arrows {

namespace matlab {

class KWIVER_ALGO_MATLAB_EXPORT matlab_image_filter
  : public vital::algo::image_filter
{
public:
  matlab_image_filter();
  virtual ~matlab_image_filter();

  PLUGGABLE_IMPL(
    matlab_image_filter,
    "Bridge to matlab image filter implementation.",
    PARAM_DEFAULT(
      program_file, std::string,
      "File name of the matlab image filter program to run.",
      "" ) )
  void set_configuration_internal(
    vital::config_block_sptr config ) override;

protected:
  void initialize() override;

  bool check_configuration( vital::config_block_sptr config ) const override;

  // Main detection method
  vital::image_container_sptr filter(
    vital::image_container_sptr image_data ) override;

private:
  class priv;

  KWIVER_UNIQUE_PTR( priv, d );
};

} // namespace matlab

} // namespace arrows

}     // end namespace

#endif // VITAL_BINDINGS_MATLAB_IMAGE_FILTER_H
