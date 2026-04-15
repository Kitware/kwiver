// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief OCV ORB feature detector and extractor wrapper

#ifndef KWIVER_ARROWS_FEATURE_DETECT_EXTRACT_ORB_H_
#define KWIVER_ARROWS_FEATURE_DETECT_EXTRACT_ORB_H_

#include <arrows/ocv/algo/detect_features.h>
#include <arrows/ocv/algo/extract_descriptors.h>
#include <arrows/ocv/kwiver_algo_ocv_export.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace ocv {

namespace orb {

#if KWIVER_OPENCV_VERSION_MAJOR >= 4
using ScoreType = cv::ORB::ScoreType;
#else
using ScoreType = int;
#endif

} // namespace orb

class KWIVER_ALGO_OCV_EXPORT detect_features_ORB
  : public ocv::detect_features
{
public:
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
#define DETECT_FEATURES_ORB_EXTRA_PARAMETERS \
,                                            \
PARAM_DEFAULT( fast_threshold, int, "Undocumented", 20 )
#else
#define DETECT_FEATURES_ORB_EXTRA_PARAMETERS
#endif
  PLUGGABLE_IMPL(
    detect_features_ORB,
    "OpenCV feature detection via the ORB algorithm",

    PARAM_DEFAULT(
      n_features, int,
      "The maximum number of features to retain",
      500 ),

    PARAM_DEFAULT(
      scale_factor, float,
      "Pyramid decimation ratio, greater than 1. "
      "scaleFactor==2 means the classical pyramid, where each "
      "next level has 4x less pixels than the previous, but "
      "such a big scale factor will degrade feature matching "
      "scores dramatically. On the other hand, too close to 1 "
      "scale factor will mean that to cover certain scale "
      "range you will need more pyramid levels and so the "
      "speed will suffer.",
      1.2f ),

    PARAM_DEFAULT(
      n_levels, int,
      "The number of pyramid levels. The smallest level will "
      "have linear size equal to "
      "input_image_linear_size/pow(scale_factor, "
      "n_levels).",
      9 ),

    PARAM_DEFAULT(
      edge_threshold, int,
      "This is size of the border where the features are not "
      "detected. It should roughly match the patch_size "
      "parameter.",
      31 ),

    PARAM_DEFAULT(
      first_level, int,
      "It should be 0 in the current implementation.",
      0 ),

    PARAM_DEFAULT(
      wta_k, int,
      "The number of points that produce each element of the "
      "oriented BRIEF descriptor. The default value 2 "
      "means the BRIEF where we take a random point pair "
      "and compare their brightnesses, so we get 0/1 "
      "response. Other possible values are 3 and 4. For "
      "example, 3 means that we take 3 random points (of "
      "course, those point coordinates are random, but "
      "they are generated from the pre-defined seed, so "
      "each element of BRIEF descriptor is computed "
      "deterministically from the pixel rectangle), find "
      "point of maximum brightness and output index of "
      "the winner (0, 1 or 2). Such output will occupy 2 "
      "bits, and therefore it will need a special variant "
      "of Hamming distance, denoted as NORM_HAMMING2 (2 "
      "bits per bin). When WTA_K=4, we take 4 random "
      "points to compute each bin (that will also occupy "
      "2 bits with possible values 0, 1, 2 or 3).",
      2 ),

    PARAM_DEFAULT(
      score_type,
      int,
      "The default HARRIS_SCORE (value=" KWIVER_STRINGIFY(
        cv::ORB::HARRIS_SCORE ) ") "
                                "means that Harris algorithm is used to rank features (the score is "
                                "written to KeyPoint::score and is used to retain best n_features "
                                "features); FAST_SCORE (value="
      KWIVER_STRINGIFY( cv::ORB::FAST_SCORE ) ") is "
                                              "alternative value of the parameter that produces slightly less "
                                              "stable key-points, but it is a little faster to compute.",
      cv::ORB::HARRIS_SCORE ),

    PARAM_DEFAULT(
      patch_size, int,
      "Size of the patch used by the oriented BRIEF "
      "descriptor. Of course, on smaller pyramid layers "
      "the perceived image area covered by a feature will "
      "be larger.",
      31 )
    DETECT_FEATURES_ORB_EXTRA_PARAMETERS );

  /// Destructor
  virtual ~detect_features_ORB();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_detector_parameters() const override;
};

#undef DETECT_FEATURES_ORB_EXTRA_PARAMETERS

class KWIVER_ALGO_OCV_EXPORT extract_descriptors_ORB
  : public ocv::extract_descriptors
{
public:
#if KWIVER_OPENCV_VERSION_MAJOR >= 3
#define EXTRACT_DESCRIPTORS_ORB_EXTRA_PARAMETERS \
,                                                \
PARAM_DEFAULT( fast_threshold, int, "Undocumented", 20 )
#else
#define EXTRACT_DESCRIPTORS_ORB_EXTRA_PARAMETERS
#endif
  PLUGGABLE_IMPL(
    extract_descriptors_ORB,
    "OpenCV feature-point descriptor extraction via the ORB algorithm",

    PARAM_DEFAULT(
      n_features, int,
      "The maximum number of features to retain",
      500 ),

    PARAM_DEFAULT(
      scale_factor, float,
      "Pyramid decimation ratio, greater than 1. "
      "scaleFactor==2 means the classical pyramid, where each "
      "next level has 4x less pixels than the previous, but "
      "such a big scale factor will degrade feature matching "
      "scores dramatically. On the other hand, too close to 1 "
      "scale factor will mean that to cover certain scale "
      "range you will need more pyramid levels and so the "
      "speed will suffer.",
      1.2f ),

    PARAM_DEFAULT(
      n_levels, int,
      "The number of pyramid levels. The smallest level will "
      "have linear size equal to "
      "input_image_linear_size/pow(scale_factor, "
      "n_levels).",
      9 ),

    PARAM_DEFAULT(
      edge_threshold, int,
      "This is size of the border where the features are not "
      "detected. It should roughly match the patch_size "
      "parameter.",
      31 ),

    PARAM_DEFAULT(
      first_level, int,
      "It should be 0 in the current implementation.",
      0 ),

    PARAM_DEFAULT(
      wta_k, int,
      "The number of points that produce each element of the "
      "oriented BRIEF descriptor. The default value 2 "
      "means the BRIEF where we take a random point pair "
      "and compare their brightnesses, so we get 0/1 "
      "response. Other possible values are 3 and 4. For "
      "example, 3 means that we take 3 random points (of "
      "course, those point coordinates are random, but "
      "they are generated from the pre-defined seed, so "
      "each element of BRIEF descriptor is computed "
      "deterministically from the pixel rectangle), find "
      "point of maximum brightness and output index of "
      "the winner (0, 1 or 2). Such output will occupy 2 "
      "bits, and therefore it will need a special variant "
      "of Hamming distance, denoted as NORM_HAMMING2 (2 "
      "bits per bin). When WTA_K=4, we take 4 random "
      "points to compute each bin (that will also occupy "
      "2 bits with possible values 0, 1, 2 or 3).",
      2 ),

    PARAM_DEFAULT(
      score_type,
      int,
      "The default HARRIS_SCORE (value=" KWIVER_STRINGIFY(
        cv::ORB::HARRIS_SCORE ) ") "
                                "means that Harris algorithm is used to rank features (the score is "
                                "written to KeyPoint::score and is used to retain best n_features "
                                "features); FAST_SCORE (value="
      KWIVER_STRINGIFY( cv::ORB::FAST_SCORE ) ") is "
                                              "alternative value of the parameter that produces slightly less "
                                              "stable key-points, but it is a little faster to compute.",
      cv::ORB::HARRIS_SCORE ),

    PARAM_DEFAULT(
      patch_size, int,
      "Size of the patch used by the oriented BRIEF "
      "descriptor. Of course, on smaller pyramid layers "
      "the perceived image area covered by a feature will "
      "be larger.",
      31 )
    EXTRACT_DESCRIPTORS_ORB_EXTRA_PARAMETERS
  );

  /// Destructor
  virtual ~extract_descriptors_ORB();

  /// Check that the algorithm's configuration vital::config_block is valid
  bool check_configuration( vital::config_block_sptr config ) const override;

private:
  void initialize() override;
  void set_configuration_internal( vital::config_block_sptr config ) override;
  void update_extractor_parameters() const override;
};

} // end namespace ocv

} // end namespace arrows

} // end namespace kwiver

#undef DETECT_FEATURES_ORB_EXTRA_PARAMETERS
#endif
