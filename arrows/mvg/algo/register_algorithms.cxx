// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Plugin algorithm registration for MVG Arrow

#include <arrows/mvg/kwiver_algo_mvg_plugin_export.h>

#include <vital/plugin_management/plugin_factory.h>

#include <arrows/mvg/algo/hierarchical_bundle_adjust.h>
#include <arrows/mvg/algo/initialize_cameras_landmarks.h>
#include <arrows/mvg/algo/initialize_cameras_landmarks_basic.h>
#include <arrows/mvg/algo/integrate_depth_maps.h>
#include <arrows/mvg/algo/triangulate_landmarks.h>

// TODO: These are files that should move to MVG from Core
// #include <arrows/mvg/close_loops_bad_frames_only.h>
// #include <arrows/mvg/close_loops_appearance_indexed.h>
// #include <arrows/mvg/close_loops_exhaustive.h>
// #include <arrows/mvg/close_loops_keyframe.h>
// #include <arrows/mvg/close_loops_multi_method.h>
// #include <arrows/mvg/compute_ref_homography_core.h>
// #include <arrows/mvg/estimate_canonical_transform.h>
// #include <arrows/mvg/feature_descriptor_io.h>
// #include <arrows/mvg/filter_features_magnitude.h>
// #include <arrows/mvg/filter_features_scale.h>
// #include <arrows/mvg/filter_tracks.h>
// #include <arrows/mvg/keyframe_selector_basic.h>
// #include <arrows/mvg/match_features_fundamental_matrix.h>
// #include <arrows/mvg/match_features_homography.h>
// #include <arrows/mvg/track_features_augment_keyframes.h>
// #include <arrows/mvg/track_features_core.h>

namespace kwiver {

namespace arrows {

namespace mvg {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_MVG_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact = vpl.add_factory< vital::algo::bundle_adjust,
    hierarchical_bundle_adjust >( "mvg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.mvg" );

  fact = vpl.add_factory< vital::algo::integrate_depth_maps,
    integrate_depth_maps >( "mvg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.mvg" );
  fact = vpl.add_factory< vital::algo::initialize_cameras_landmarks,
    initialize_cameras_landmarks >( "mvg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.mvg" );
  // plugin name must be different since it's using the same interface as
  // initialize_cameras_landmarks
  fact = vpl.add_factory< vital::algo::initialize_cameras_landmarks,
    initialize_cameras_landmarks_basic >( "mvg-basic" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.mvg" );
  fact = vpl.add_factory< vital::algo::triangulate_landmarks,
    triangulate_landmarks >( "mvg" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows.mvg" );
}

} // end namespace mvg

} // end namespace arrows

} // end namespace kwiver
