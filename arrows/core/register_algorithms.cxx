/*ckwg +29
 * Copyright 2014-2019 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 * \brief Defaults plugin algorithm registration interface impl
 */

#include <arrows/core/kwiver_algo_core_plugin_export.h>

#include <vital/algo/algorithm_factory.h>

#include <arrows/core/associate_detections_to_tracks_threshold.h>
#include <arrows/core/class_probablity_filter.h>
#include <arrows/core/close_loops_bad_frames_only.h>
#include <arrows/core/close_loops_appearance_indexed.h>
#include <arrows/core/close_loops_exhaustive.h>
#include <arrows/core/close_loops_keyframe.h>
#include <arrows/core/close_loops_multi_method.h>
#include <arrows/core/compute_association_matrix_from_features.h>
#include <arrows/core/compute_ref_homography_core.h>
#include <arrows/core/convert_image_bypass.h>
#include <arrows/core/create_detection_grid.h>
#include <arrows/core/detect_features_filtered.h>
#include <arrows/core/detected_object_set_input_csv.h>
#include <arrows/core/detected_object_set_input_kw18.h>
#include <arrows/core/detected_object_set_input_simulator.h>
#include <arrows/core/detected_object_set_output_csv.h>
#include <arrows/core/detected_object_set_output_kw18.h>
#include <arrows/core/dynamic_config_none.h>
#include <arrows/core/estimate_canonical_transform.h>
#include <arrows/core/example_detector.h>
#include <arrows/core/feature_descriptor_io.h>
#include <arrows/core/filter_features_magnitude.h>
#include <arrows/core/filter_features_nonmax.h>
#include <arrows/core/filter_features_scale.h>
#include <arrows/core/filter_tracks.h>
#include <arrows/core/handle_descriptor_request_core.h>
#include <arrows/core/hierarchical_bundle_adjust.h>
#include <arrows/core/initialize_cameras_landmarks.h>
#include <arrows/core/initialize_cameras_landmarks_keyframe.h>
#include <arrows/core/initialize_object_tracks_threshold.h>
#include <arrows/core/interpolate_track_spline.h>
#include <arrows/core/keyframe_selector_basic.h>
#include <arrows/core/match_features_fundamental_matrix.h>
#include <arrows/core/match_features_homography.h>
#include <arrows/core/read_object_track_set_kw18.h>
#include <arrows/core/read_track_descriptor_set_csv.h>
#include <arrows/core/track_features_augment_keyframes.h>
#include <arrows/core/track_features_core.h>
#include <arrows/core/transfer_bbox_with_depth_map.h>
#include <arrows/core/transform_detected_object_set.h>
#include <arrows/core/triangulate_landmarks.h>
#include <arrows/core/uv_unwrap_mesh.h>
#include <arrows/core/video_input_filter.h>
#include <arrows/core/video_input_image_list.h>
#include <arrows/core/video_input_pos.h>
#include <arrows/core/video_input_splice.h>
#include <arrows/core/video_input_split.h>
#include <arrows/core/write_object_track_set_kw18.h>
#include <arrows/core/write_track_descriptor_set_csv.h>


namespace kwiver {
namespace arrows {
namespace core {


// ----------------------------------------------------------------------------
extern "C"
KWIVER_ALGO_CORE_PLUGIN_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  kwiver::vital::algorithm_registrar reg( vpm, "arrows.core" );

  if (reg.is_module_loaded())
  {
    return;
  }

  reg.register_algorithm< associate_detections_to_tracks_threshold >();
  reg.register_algorithm< class_probablity_filter >();
  reg.register_algorithm< close_loops_appearance_indexed >();
  reg.register_algorithm< close_loops_bad_frames_only >();
  reg.register_algorithm< close_loops_exhaustive >();
  reg.register_algorithm< close_loops_keyframe >();
  reg.register_algorithm< close_loops_multi_method >();
  reg.register_algorithm< compute_association_matrix_from_features >();
  reg.register_algorithm< compute_ref_homography_core >();
  reg.register_algorithm< convert_image_bypass >();
  reg.register_algorithm< create_detection_grid >();
  reg.register_algorithm< detect_features_filtered >();
  reg.register_algorithm< detected_object_set_input_csv >();
  reg.register_algorithm< detected_object_set_input_kw18 >();
  reg.register_algorithm< detected_object_set_input_simulator >();
  reg.register_algorithm< detected_object_set_output_csv >();
  reg.register_algorithm< detected_object_set_output_kw18 >();
  reg.register_algorithm< dynamic_config_none >();
  reg.register_algorithm< estimate_canonical_transform >();
  reg.register_algorithm< example_detector >();
  reg.register_algorithm< feature_descriptor_io >();
  reg.register_algorithm< filter_features_magnitude >();
  reg.register_algorithm< filter_features_nonmax >();
  reg.register_algorithm< filter_features_scale >();
  reg.register_algorithm< filter_tracks >();
  reg.register_algorithm< handle_descriptor_request_core >();
  reg.register_algorithm< hierarchical_bundle_adjust >();
  reg.register_algorithm< initialize_cameras_landmarks >();
  reg.register_algorithm< initialize_cameras_landmarks_keyframe >();
  reg.register_algorithm< initialize_object_tracks_threshold >();
  reg.register_algorithm< interpolate_track_spline >();
  reg.register_algorithm< keyframe_selector_basic >();
  reg.register_algorithm< match_features_fundamental_matrix >();
  reg.register_algorithm< match_features_homography >();
  reg.register_algorithm< read_object_track_set_kw18 >();
  reg.register_algorithm< read_track_descriptor_set_csv >();
  reg.register_algorithm< track_features_augment_keyframes >();
  reg.register_algorithm< track_features_core >();
  reg.register_algorithm< transfer_bbox_with_depth_map >();
  reg.register_algorithm< transform_detected_object_set >();
  reg.register_algorithm< triangulate_landmarks >();
  reg.register_algorithm< uv_unwrap_mesh >();
  reg.register_algorithm< video_input_filter >();
  reg.register_algorithm< video_input_image_list >();
  reg.register_algorithm< video_input_pos >();
  reg.register_algorithm< video_input_splice >();
  reg.register_algorithm< video_input_split >();
  reg.register_algorithm< write_object_track_set_kw18 >();
  reg.register_algorithm< write_track_descriptor_set_csv >();

  reg.mark_module_as_loaded();
}

} // end namespace core
} // end namespace arrows
} // end namespace kwiver
