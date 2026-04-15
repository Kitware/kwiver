// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV BRIEF descriptor extractor wrapper

#ifndef KWIVER_ARROWS_EXTRACT_DESCRIPTORS_BRIEF_H_
#define KWIVER_ARROWS_EXTRACT_DESCRIPTORS_BRIEF_H_

#include <opencv2/opencv_modules.hpp>
#if KWIVER_OPENCV_VERSION_MAJOR < 3 || defined( HAVE_OPENCV_XFEATURES2D )

#include <vital/algo/extract_descriptors.h>

#include <arrows/ocv/algo/extract_descriptors.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_BRIEF
  : public extract_descriptors
{
public:
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
#define EXTRACT_DESCRIPTORS_BRIEF_EXTRA_PARAMETERS           \
,                                                            \
PARAM_DEFAULT(                                               \
    use_orientation, bool,                                   \
    "sample patterns using keypoints orientation, disabled " \
    "by default.",                                           \
    false )
#else
#define EXTRACT_DESCRIPTORS_BRIEF_EXTRA_PARAMETERS
#endif

  PLUGGABLE_IMPL(
    extract_descriptors_BRIEF,
    "OpenCV feature-point descriptor extraction via the BRIEF algorithm",
    PARAM_DEFAULT(
      bytes, int,
      "Length of descriptor in bytes. It can be equal 16, 32 "
      "or 64 bytes.",
      32 )
    EXTRACT_DESCRIPTORS_BRIEF_EXTRA_PARAMETERS
  );

  /// Destructor
  virtual ~extract_descriptors_BRIEF();

  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void update_extractor_parameters() const override;
};

#undef EXTRACT_DESCRIPTORS_BRIEF_EXTRA_PARAMETERS
#define KWIVER_OCV_HAS_BRIEF

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#endif // has OCV support

#endif // KWIVER_ARROWS_EXTRACT_DESCRIPTORS_BRIEF_H_
