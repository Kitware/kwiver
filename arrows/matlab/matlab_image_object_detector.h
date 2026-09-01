// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Header defining matlab image object detector

#ifndef VITAL_BINDINGS_MATLAB_IMAGE_OBJECT_DETECTOR_H
#define VITAL_BINDINGS_MATLAB_IMAGE_OBJECT_DETECTOR_H

#include <arrows/matlab/kwiver_algo_matlab_export.h>
#include <vital/algo/algorithm.txx>
#include <vital/algo/image_object_detector.h>

namespace kwiver {

namespace arrows {

namespace matlab {

class KWIVER_ALGO_MATLAB_EXPORT matlab_image_object_detector
  : public vital::algo::image_object_detector
{
public:
  PLUGGABLE_IMPL(
    matlab_image_object_detector,
    "Wrapper/bridge to matlab object detector implementation.",
    PARAM_DEFAULT(
      program_file, std::string,
      "File name of the matlab image object detector program to run.",
      "" ) )

  virtual ~matlab_image_object_detector();

  bool check_configuration( vital::config_block_sptr config ) const override;

  // Main detection method
  vital::detected_object_set_sptr detect(
    vital::image_container_sptr image_data ) const override;

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

#endif // VITAL_BINDINGS_MATLAB_IMAGE_OBJECT_DETECTOR_H
