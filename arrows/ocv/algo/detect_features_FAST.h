// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV FAST feature detector wrapper

#ifndef KWIVER_ARROWS_DETECT_FEATURES_FAST_H_
#define KWIVER_ARROWS_DETECT_FEATURES_FAST_H_

#include <arrows/ocv/algo/detect_features.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

class KWIVER_ALGO_OCV_EXPORT detect_features_FAST
  : public ocv::detect_features
{
public:
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
#define DETECT_FEATURES_FAST_EXTRA_PARAMETERS                                       \
,                                                                                   \
PARAM_DEFAULT(                                                                      \
    neighborhood_type,                                                              \
    int,                                                                            \
    "one of the three neighborhoods as defined in the paper: "                      \
    "TYPE_5_8="  KWIVER_STRINGIFY( cv::FastFeatureDetector::TYPE_5_8 ) ","          \
                                                                       "TYPE_7_12=" \
    KWIVER_STRINGIFY( cv::FastFeatureDetector::TYPE_7_12 ) ", "                     \
                                                           "TYPE_9_16="             \
    KWIVER_STRINGIFY( cv::FastFeatureDetector::TYPE_9_16 ) ".",                     \
    static_cast< int >( cv::FastFeatureDetector::TYPE_9_16 ) )
#else
#define DETECT_FEATURES_FAST_EXTRA_PARAMETERS
#endif

  PLUGGABLE_IMPL(
    detect_features_FAST,
    "OpenCV feature detection via the FAST algorithm",

    PARAM_DEFAULT(
      threshold, int,
      "Integer threshold on difference between intensity of "
      "the central pixel and pixels of a circle around this "
      "pixel", 10 ),

    PARAM_DEFAULT(
      nonmaxSuppression, bool,
      "Integer threshold on difference between intensity of "
      "the central pixel and pixels of a circle around this "
      "pixel", true ),

    PARAM_DEFAULT(
      target_num_features_detected,  int,
      "algorithm tries to output approximately this many features. "
      "Disable by setting to negative value.", 2500 )
    DETECT_FEATURES_FAST_EXTRA_PARAMETERS
  );
  /// Destructor
  virtual ~detect_features_FAST();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

  /// Extract a set of image features from the provided image
  ///
  /// A given mask image should be one-channel (mask->depth() == 1). If the
  /// given mask image has more than one channel, only the first will be
  /// considered.
  /// This method overrides the base detect method and adds dynamic threshold
  /// adaptation.  It adjusts the detector's feature strength threshold to try
  /// and extract a target number of features in each frame. Because scene
  /// content varies between images, different feature strength thresholds may
  /// be necessary to get the same number of feautres in different images.
  ///
  /// \param image_data contains the image data to process
  /// \param mask Mask image where regions of positive values (boolean true)
  ///            indicate regions to consider. Only the first channel will be
  ///            considered.
  /// \returns a set of image features
  vital::feature_set_sptr
  detect(
    vital::image_container_sptr image_data,
    vital::image_container_sptr mask = vital::image_container_sptr() ) const
  override;

private:
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
  void initialize() override;

  class priv;

  KWIVER_UNIQUE_PTR( priv, p_ );
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#undef DETECT_FEATURES_FAST_EXTRA_PARAMETERS

#endif // KWIVER_ARROWS_DETECT_FEATURES_FAST_H_
