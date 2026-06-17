// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Register core algorithms implementation

#include <arrows/core/kwiver_algo_core_plugin_export.h>

#include <vital/plugin_management/plugin_manager.h>

// interface
#include <vital/algo/associate_detections_to_tracks.h>
#include <vital/algo/close_loops.h>
#include <vital/algo/compute_association_matrix.h>
#include <vital/algo/compute_ref_homography.h>
#include <vital/algo/convert_image.h>
#include <vital/algo/detect_features.h>
#include <vital/algo/detected_object_filter.h>
#include <vital/algo/detected_object_set_input.h>
#include <vital/algo/detected_object_set_output.h>
#include <vital/algo/dynamic_configuration.h>
#include <vital/algo/estimate_canonical_transform.h>
#include <vital/algo/feature_descriptor_io.h>
#include <vital/algo/filter_features.h>
#include <vital/algo/filter_tracks.h>
#include <vital/algo/handle_descriptor_request.h>
#include <vital/algo/image_filter.h>
#include <vital/algo/image_io.h>
#include <vital/algo/image_object_detector.h>
#include <vital/algo/initialize_object_tracks.h>
#include <vital/algo/interpolate_track.h>
#include <vital/algo/keyframe_selection.h>
#include <vital/algo/match_features.h>
#include <vital/algo/metadata_filter.h>
#include <vital/algo/metadata_map_io.h>
#include <vital/algo/read_object_track_set.h>
#include <vital/algo/read_track_descriptor_set.h>
#include <vital/algo/track_features.h>
#include <vital/algo/uv_unwrap_mesh.h>
#include <vital/algo/video_input.h>
#include <vital/algo/write_object_track_set.h>
#include <vital/algo/write_track_descriptor_set.h>

// implementation
#include <arrows/core/algo/associate_detections_to_tracks_threshold.h>
#include <arrows/core/algo/class_probability_filter.h>
#include <arrows/core/algo/close_loops_appearance_indexed.h>
#include <arrows/core/algo/close_loops_bad_frames_only.h>
#include <arrows/core/algo/close_loops_exhaustive.h>
#include <arrows/core/algo/close_loops_keyframe.h>
#include <arrows/core/algo/close_loops_multi_method.h>
#include <arrows/core/algo/compute_association_matrix_from_features.h>
#include <arrows/core/algo/compute_ref_homography_core.h>
#include <arrows/core/algo/convert_image_bypass.h>
#include <arrows/core/algo/create_detection_grid.h>
#include <arrows/core/algo/derive_metadata.h>
#include <arrows/core/algo/detect_features_filtered.h>
#include <arrows/core/algo/detected_object_set_input_csv.h>
#include <arrows/core/algo/detected_object_set_input_kw18.h>
#include <arrows/core/algo/detected_object_set_input_simulator.h>
#include <arrows/core/algo/detected_object_set_output_csv.h>
#include <arrows/core/algo/detected_object_set_output_kw18.h>
#include <arrows/core/algo/dynamic_config_none.h>
#include <arrows/core/algo/estimate_canonical_transform.h>
#include <arrows/core/algo/example_detector.h>
#include <arrows/core/algo/feature_descriptor_io.h>
#include <arrows/core/algo/filter_features_magnitude.h>
#include <arrows/core/algo/filter_features_nonmax.h>
#include <arrows/core/algo/filter_features_scale.h>
#include <arrows/core/algo/filter_tracks.h>
#include <arrows/core/algo/handle_descriptor_request_core.h>
#include <arrows/core/algo/image_filter_bypass.h>
#include <arrows/core/algo/image_io_tiled_multifile.h>
#include <arrows/core/algo/initialize_object_tracks_threshold.h>
#include <arrows/core/algo/interpolate_track_spline.h>
#include <arrows/core/algo/keyframe_selector_basic.h>
#include <arrows/core/algo/match_features_fundamental_matrix.h>
#include <arrows/core/algo/match_features_homography.h>
#include <arrows/core/algo/merge_metadata_streams.h>
#include <arrows/core/algo/metadata_map_io_csv.h>
#include <arrows/core/algo/read_object_track_set_kw18.h>
#include <arrows/core/algo/read_track_descriptor_set_csv.h>
#include <arrows/core/algo/texture_mesh.h>
#include <arrows/core/algo/track_features_augment_keyframes.h>
#include <arrows/core/algo/track_features_core.h>
#include <arrows/core/algo/transfer_bbox_with_depth_map.h>
#include <arrows/core/algo/transform_detected_object_set.h>
#include <arrows/core/algo/uv_unwrap_mesh.h>
#include <arrows/core/algo/video_input_buffered_metadata_filter.h>
#include <arrows/core/algo/video_input_filter.h>
#include <arrows/core/algo/video_input_image_list.h>
#include <arrows/core/algo/video_input_metadata_filter.h>
#include <arrows/core/algo/video_input_pos.h>
#include <arrows/core/algo/video_input_splice.h>
#include <arrows/core/algo/video_input_split.h>
#include <arrows/core/algo/write_object_track_set_kw18.h>
#include <arrows/core/algo/write_track_descriptor_set_csv.h>

namespace kwiver {

namespace arrows {

namespace core {

// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_CORE_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpl )
{
  using kvpf = ::kwiver::vital::plugin_factory;

  auto fact =
    vpl.add_factory< vital::algo::video_input, video_input_filter >( "filter" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::texture_mesh,
    texture_mesh >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::uv_unwrap_mesh,
    uv_unwrap_mesh >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::video_input,
    video_input_split >( "split" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::video_input,
    video_input_image_list >( "image_list" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::video_input,
    video_input_splice >( "splice" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::video_input,
    video_input_buffered_metadata_filter >( "buffered_metadata_filter" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::video_input,
    video_input_metadata_filter >( "metadata_filter" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::video_input, video_input_pos >( "pos" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::metadata_map_io,
    metadata_map_io_csv >( "csv" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::filter_features,
    filter_features_scale >( "scale" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::filter_features,
    filter_features_magnitude >( "magnitude" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::filter_features,
    filter_features_nonmax >( "nonmax" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::filter_tracks,
    filter_tracks >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_set_input,
    detected_object_set_input_kw18 >( "kw18" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_set_output,
    detected_object_set_output_kw18 >( "kw18" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_set_input,
    detected_object_set_input_csv >( "csv" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_set_input,
    detected_object_set_input_simulator >( "simulator" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_set_output,
    detected_object_set_output_csv >( "csv" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::dynamic_configuration,
    dynamic_config_none >( "none" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::read_track_descriptor_set,
    read_track_descriptor_set_csv >( "csv" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::read_object_track_set,
    read_object_track_set_kw18 >( "kw18" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::associate_detections_to_tracks,
    associate_detections_to_tracks_threshold >( "threshold" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::image_object_detector,
    create_detection_grid >( "create_detection_grid" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::convert_image,
    convert_image_bypass >( "bypass" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::image_filter,
    image_filter_bypass >( "bypass" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::metadata_filter,
    derive_metadata >( "derive_metadata" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detect_features,
    detect_features_filtered >( "filtered" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::estimate_canonical_transform,
    estimate_canonical_transform >( "core_pca" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::image_object_detector,
    example_detector >( "example_detector" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::feature_descriptor_io,
    feature_descriptor_io >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::handle_descriptor_request,
    handle_descriptor_request_core >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::interpolate_track,
    interpolate_track_spline >( "spline" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::track_features,
    track_features_core >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::write_object_track_set,
    write_object_track_set_kw18 >( "kw18" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::write_track_descriptor_set,
    write_track_descriptor_set_csv >( "csv" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::match_features,
    match_features_fundamental_matrix >( "fundamental_matrix_guided" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::match_features,
    match_features_homography >( "homography_guided" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::compute_ref_homography,
    compute_ref_homography_core >( "core" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::compute_association_matrix,
    compute_association_matrix_from_features >( "from_features" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::metadata_filter,
    merge_metadata_streams >( "merge_metadata_streams" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::keyframe_selection,
    keyframe_selector_basic >( "basic" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::track_features,
    track_features_augment_keyframes >( "augment_keyframes" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::initialize_object_tracks,
    initialize_object_tracks_threshold >( "threshold" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::close_loops,
    close_loops_keyframe >( "keyframe" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::close_loops,
    close_loops_exhaustive >( "exhaustive" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::close_loops,
    close_loops_bad_frames_only >( "bad_frames_only" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::close_loops,
    close_loops_appearance_indexed >( "appearance_indexed" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::close_loops,
    close_loops_appearance_indexed >( "multi_method" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_filter,
    transfer_bbox_with_depth_map >( "transfer_bbox_with_depth_map" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_filter,
    transform_detected_object_set >( "transform_detected_object_set" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::detected_object_filter,
    class_probability_filter >( "class_probability_filter" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );

  fact = vpl.add_factory< vital::algo::image_io,
    image_io_tiled_multifile >( "tiled_multifile" );
  fact->add_attribute( kvpf::PLUGIN_MODULE_NAME, "arrows_core" );
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
