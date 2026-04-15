// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/dbow2/algo/match_descriptor_sets.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

using kwiver::arrows::dbow2::match_descriptor_sets;


static constexpr double noisy_center_tolerance = 1e-8;
static constexpr double noisy_rotation_tolerance = 2e-9;
static constexpr double noisy_intrinsics_tolerance = 2e-6;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( match_descriptor_sets, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kwiver::vital::create_algorithm< kwiver::vital::algo::match_descriptor_sets >
    (
      "dbow2" ) );
}

TEST ( match_descriptor_sets, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    match_descriptor_sets,
    "Use DBoW2 for bag of words matching of descriptor sets. "
    "This is currently limited to OpenCV ORB descriptors.",
    PARAM_DEFAULT(
      max_num_candidate_matches_from_vocabulary_tree, int,
      "the maximum number of candidate matches to return from the vocabulary tree",
      10 ),
    PARAM(
      training_image_list_path, std::string,
      "path to the list of vocabulary training images" ),
    PARAM_DEFAULT(
      vocabulary_path, std::string,
      "path to the vocabulary file",
      "kwiver_voc.yml.gz" ),
    PARAM(
      image_io, vital::algo::image_io_sptr,
      "image_io" ),
    PARAM(
      detector, vital::algo::detect_features_sptr,
      "detector" ),
    PARAM(
      extractor, vital::algo::extract_descriptors_sptr,
      "extractor" )
  );
}
