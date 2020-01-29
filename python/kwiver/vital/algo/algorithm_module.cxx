/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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
 * \file algorithm_implementation.cxx
 *
 * \brief python bindings for algorithm
 */


#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vital/algo/algorithm.h>
#include <vital/algo/analyze_tracks.h>
#include <vital/algo/image_object_detector.h>
#include <python/kwiver/vital/algo/trampoline/analyze_tracks_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/associate_detections_to_tracks_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/close_loops_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/compute_association_matrix_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/bundle_adjust_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/compute_stereo_depth_map_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/compute_ref_homography_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/compute_depth_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/convert_image_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/detected_object_filter_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/detected_object_set_input_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/detected_object_set_output_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/detect_features_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/detect_motion_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/draw_detected_object_set_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/draw_tracks_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/estimate_canonical_transform_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/estimate_essential_matrix_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/estimate_fundamental_matrix_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/estimate_homography_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/estimate_pnp_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/estimate_similarity_transform_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/extract_descriptors_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/feature_descriptor_io_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/filter_features_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/filter_tracks_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/image_io_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/image_object_detector_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/initialize_cameras_landmarks_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/initialize_object_tracks_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/integrate_depth_maps_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/interpolate_track_trampoline.txx>
#include <python/kwiver/vital/algo/trampoline/keyframe_selection_trampoline.txx>
#include <python/kwiver/vital/algo/algorithm.h>
#include <python/kwiver/vital/algo/analyze_tracks.h>
#include <python/kwiver/vital/algo/associate_detections_to_tracks.h>
#include <python/kwiver/vital/algo/close_loops.h>
#include <python/kwiver/vital/algo/compute_association_matrix.h>
#include <python/kwiver/vital/algo/bundle_adjust.h>
#include <python/kwiver/vital/algo/compute_depth.h>
#include <python/kwiver/vital/algo/compute_ref_homography.h>
#include <python/kwiver/vital/algo/compute_stereo_depth_map.h>
#include <python/kwiver/vital/algo/convert_image.h>
#include <python/kwiver/vital/algo/detected_object_filter.h>
#include <python/kwiver/vital/algo/detected_object_set_input.h>
#include <python/kwiver/vital/algo/detected_object_set_output.h>
#include <python/kwiver/vital/algo/detect_features.h>
#include <python/kwiver/vital/algo/detect_motion.h>
#include <python/kwiver/vital/algo/draw_detected_object_set.h>
#include <python/kwiver/vital/algo/draw_tracks.h>
#include <python/kwiver/vital/algo/estimate_canonical_transform.h>
#include <python/kwiver/vital/algo/estimate_essential_matrix.h>
#include <python/kwiver/vital/algo/estimate_fundamental_matrix.h>
#include <python/kwiver/vital/algo/estimate_homography.h>
#include <python/kwiver/vital/algo/estimate_pnp.h>
#include <python/kwiver/vital/algo/estimate_similarity_transform.h>
#include <python/kwiver/vital/algo/extract_descriptors.h>
#include <python/kwiver/vital/algo/feature_descriptor_io.h>
#include <python/kwiver/vital/algo/filter_features.h>
#include <python/kwiver/vital/algo/filter_tracks.h>
#include <python/kwiver/vital/algo/image_io.h>
#include <python/kwiver/vital/algo/image_object_detector.h>
#include <python/kwiver/vital/algo/initialize_cameras_landmarks.h>
#include <python/kwiver/vital/algo/initialize_object_tracks.h>
#include <python/kwiver/vital/algo/integrate_depth_maps.h>
#include <python/kwiver/vital/algo/interpolate_track.h>
#include <python/kwiver/vital/algo/keyframe_selection.h>
#include <sstream>

namespace py = pybind11;

PYBIND11_MODULE(algorithm, m)
{
  algorithm(m);
  register_algorithm<kwiver::vital::algo::analyze_tracks,
            algorithm_def_at_trampoline<>>(m, "analyze_tracks");
  register_algorithm<kwiver::vital::algo::associate_detections_to_tracks,
            algorithm_def_adtt_trampoline<>>(m, "associate_detections_to_tracks");
  register_algorithm<kwiver::vital::algo::bundle_adjust,
            algorithm_def_ba_trampoline<>>(m, "bundle_adjust");
  register_algorithm<kwiver::vital::algo::close_loops,
            algorithm_def_cl_trampoline<>>(m, "close_loops");
  register_algorithm<kwiver::vital::algo::compute_association_matrix,
            algorithm_def_cam_trampoline<>>(m, "compute_association_matrix");
  register_algorithm<kwiver::vital::algo::compute_depth,
            algorithm_def_cd_trampoline<>>(m, "compute_depth");
  register_algorithm<kwiver::vital::algo::compute_ref_homography,
            algorithm_def_crh_trampoline<>>(m, "compute_ref_homography");
  register_algorithm<kwiver::vital::algo::compute_stereo_depth_map,
            algorithm_def_csdm_trampoline<>>(m, "compute_stereo_depth_map");
  register_algorithm<kwiver::vital::algo::convert_image,
            algorithm_def_ci_trampoline<>>(m, "convert_image");
  register_algorithm<kwiver::vital::algo::detected_object_filter,
            algorithm_def_dof_trampoline<>>(m, "detected_object_filter");
  register_algorithm<kwiver::vital::algo::detected_object_set_input,
            algorithm_def_dosi_trampoline<>>(m, "detected_object_set_input");
  register_algorithm<kwiver::vital::algo::detected_object_set_output,
            algorithm_def_doso_trampoline<>>(m, "detected_object_set_output");
  register_algorithm<kwiver::vital::algo::detect_features,
            algorithm_def_df_trampoline<>>(m, "detect_features");
  register_algorithm<kwiver::vital::algo::detect_motion,
            algorithm_def_dm_trampoline<>>(m, "detect_motion");
  register_algorithm<kwiver::vital::algo::draw_detected_object_set,
            algorithm_def_ddos_trampoline<>>(m, "draw_detected_object_set");
  register_algorithm<kwiver::vital::algo::draw_tracks,
            algorithm_def_dt_trampoline<>>(m, "draw_tracks");
  register_algorithm<kwiver::vital::algo::estimate_canonical_transform,
            algorithm_def_ect_trampoline<>>(m, "estimate_canonical_transform");
  register_algorithm<kwiver::vital::algo::estimate_essential_matrix,
            algorithm_def_eem_trampoline<>>(m, "estimate_essential_matrix");
  register_algorithm<kwiver::vital::algo::estimate_fundamental_matrix,
            algorithm_def_efm_trampoline<>>(m, "estimate_fundamental_matrix");
  register_algorithm<kwiver::vital::algo::estimate_homography,
            algorithm_def_eh_trampoline<>>(m, "estimate_homography");
  register_algorithm<kwiver::vital::algo::estimate_pnp,
            algorithm_def_epnp_trampoline<>>(m, "estimate_pnp");
  register_algorithm<kwiver::vital::algo::estimate_similarity_transform,
            algorithm_def_est_trampoline<>>(m, "estimate_similarity_transform");
  register_algorithm<kwiver::vital::algo::extract_descriptors,
            algorithm_def_ed_trampoline<>>(m, "extract_descriptors");
  register_algorithm<kwiver::vital::algo::feature_descriptor_io,
            algorithm_def_fdio_trampoline<>>(m, "feature_descriptor_io");
  register_algorithm<kwiver::vital::algo::filter_features,
            algorithm_def_ff_trampoline<>>(m, "filter_features");
  register_algorithm<kwiver::vital::algo::filter_tracks,
            algorithm_def_ft_trampoline<>>(m, "filter_tracks");
  register_algorithm<kwiver::vital::algo::image_io,
            algorithm_def_iio_trampoline<>>(m, "image_io");
  register_algorithm<kwiver::vital::algo::image_object_detector,
            algorithm_def_iod_trampoline<>>(m, "image_object_detector");
  register_algorithm<kwiver::vital::algo::initialize_cameras_landmarks,
            algorithm_def_icl_trampoline<>>(m, "initialize_cameras_landmarks");
  register_algorithm<kwiver::vital::algo::initialize_object_tracks,
            algorithm_def_iot_trampoline<>>(m, "initialize_object_tracks");
  register_algorithm<kwiver::vital::algo::integrate_depth_maps,
            algorithm_def_idm_trampoline<>>(m, "integrate_depth_maps");
  register_algorithm<kwiver::vital::algo::interpolate_track,
            algorithm_def_it_trampoline<>>(m, "interpolate_track");
  register_algorithm<kwiver::vital::algo::keyframe_selection,
            algorithm_def_kf_trampoline<>>(m, "keyframe_selection");

  analyze_tracks(m);
  associate_detections_to_tracks(m);
  bundle_adjust(m);
  close_loops(m);
  compute_association_matrix(m);
  compute_depth(m);
  compute_ref_homography(m);
  compute_stereo_depth_map(m);
  convert_image(m);
  detected_object_filter(m);
  detected_object_set_input(m);
  detected_object_set_output(m);
  detect_features(m);
  detect_motion(m);
  draw_detected_object_set(m);
  draw_tracks(m);
  estimate_canonical_transform(m);
  estimate_essential_matrix(m);
  estimate_fundamental_matrix(m);
  estimate_homography(m);
  estimate_pnp(m);
  estimate_similarity_transform(m);
  extract_descriptors(m);
  feature_descriptor_io(m);
  filter_features(m);
  filter_tracks(m);
  image_io(m);
  image_object_detector(m);
  initialize_cameras_landmarks(m);
  initialize_object_tracks(m);
  integrate_depth_maps(m);
  interpolate_track(m);
  keyframe_selection(m);
}
