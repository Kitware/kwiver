// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Register VXL algorithms implementation

#include <arrows/vxl/kwiver_algo_vxl_plugin_export.h>
#include <vital/plugin_management/plugin_manager.h>

// interface
#include <vital/algo/estimate_essential_matrix.h>
#include <vital/algo/estimate_similarity_transform.h>
#include <vital/algo/image_filter.h>
#include <vital/algo/image_io.h>
#include <vital/algo/nearest_neighbors.h>
#include <vital/algo/optimize_cameras.h>

// implementation
#include <arrows/vxl/aligned_edge_detection.h>
#include <arrows/vxl/average_frames.h>
#include <arrows/vxl/bundle_adjust.h>
#include <arrows/vxl/close_loops_homography_guided.h>
#include <arrows/vxl/color_commonality_filter.h>
#include <arrows/vxl/convert_image.h>
#include <arrows/vxl/estimate_canonical_transform.h>
#include <arrows/vxl/estimate_essential_matrix.h>
#include <arrows/vxl/estimate_fundamental_matrix.h>
#include <arrows/vxl/estimate_homography.h>
#include <arrows/vxl/estimate_similarity_transform.h>
#include <arrows/vxl/hashed_image_classifier_filter.h>
#include <arrows/vxl/high_pass_filter.h>
#include <arrows/vxl/image_io.h>
#include <arrows/vxl/kd_tree.h>
#include <arrows/vxl/match_features_constrained.h>
#include <arrows/vxl/morphology.h>
#include <arrows/vxl/optimize_cameras.h>
#include <arrows/vxl/pixel_feature_extractor.h>
#include <arrows/vxl/split_image.h>
#include <arrows/vxl/threshold.h>
#include <arrows/vxl/triangulate_landmarks.h>

namespace kwiver {

namespace arrows {

namespace vxl {

extern "C"
KWIVER_ALGO_VXL_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::image_filter,
    aligned_edge_detection >( "vxl_aligned_edge_detection" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    average_frames >( "vxl_average" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::bundle_adjust,
    bundle_adjust >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::close_loops,
    close_loops_homography_guided >( "vxl_homography_guided" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    color_commonality_filter >( "vxl_color_commonality" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    convert_image >( "vxl_convert_image" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::estimate_canonical_transform,
    estimate_canonical_transform >( "vxl_plane" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::estimate_essential_matrix,
    estimate_essential_matrix >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::estimate_fundamental_matrix,
    estimate_fundamental_matrix >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::estimate_homography,
    estimate_homography >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::estimate_similarity_transform,
    estimate_similarity_transform >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    hashed_image_classifier_filter >( "vxl_hashed_image_classifier_filter" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    high_pass_filter >( "vxl_high_pass_filter" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_io,
    image_io >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::nearest_neighbors,
    kd_tree >( "vxl_kd_tree" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::match_features,
    match_features_constrained >( "vxl_constrained" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    morphology >( "vxl_morphology" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::optimize_cameras,
    optimize_cameras >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    pixel_feature_extractor >( "vxl_pixel_feature_extractor" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::split_image,
    split_image >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::image_filter,
    threshold >( "vxl_threshold" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );

  fact = vpl.add_factory< vital::algo::triangulate_landmarks,
    triangulate_landmarks >( "vxl" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.vxl" );
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
