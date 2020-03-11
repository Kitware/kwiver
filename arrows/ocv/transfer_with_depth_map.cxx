/*ckwg +29
 * Copyright 2020 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include "transfer_with_depth_map.h"

#include <iostream>
#include <vital/io/camera_io.h>
#include <vital/config/config_difference.h>
#include <vital/util/string.h>
#include <Eigen/Core>
#include <arrows/ocv/image_container.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace kwiver::vital;

namespace kwiver {
namespace arrows {
namespace ocv {

// ---------------------------------------------------------------------------
transfer_with_depth_map::
transfer_with_depth_map()
  : src_camera_krtd_file_name( "" )
  , dest_camera_krtd_file_name( "" )
  , src_camera_depth_map_file_name( "" )
{
}

// ---------------------------------------------------------------------------
transfer_with_depth_map::
transfer_with_depth_map(kwiver::vital::camera_perspective_sptr src_cam,
                        kwiver::vital::camera_perspective_sptr dest_cam,
                        kwiver::vital::image_container_sptr src_cam_depth_map)
  : src_camera( src_cam )
  , dest_camera( dest_cam )
  , depth_map( src_cam_depth_map )
{
}

// ---------------------------------------------------------------------------
vital::config_block_sptr
transfer_with_depth_map::
get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "src_camera_krtd_file_name", src_camera_krtd_file_name,
                     "Source camera KRTD file name path" );

  config->set_value( "dest_camera_krtd_file_name", dest_camera_krtd_file_name,
                     "Destination camera KRTD file name path" );

  config->set_value( "src_camera_depth_map_file_name",
                     src_camera_depth_map_file_name,
                     "Source camera depth map file name path" );

  return config;
}

// ---------------------------------------------------------------------------
void
transfer_with_depth_map::
set_configuration( vital::config_block_sptr config_in )
{
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );
  this->src_camera_krtd_file_name =
    config->get_value< std::string > ( "src_camera_krtd_file_name" );
  this->dest_camera_krtd_file_name =
    config->get_value< std::string > ( "dest_camera_krtd_file_name" );
  this->src_camera_depth_map_file_name =
    config->get_value< std::string > ( "src_camera_depth_map_file_name" );


  this->src_camera =
    kwiver::vital::read_krtd_file( this->src_camera_krtd_file_name );
  this->dest_camera =
    kwiver::vital::read_krtd_file( this->dest_camera_krtd_file_name );

  cv::Mat src_cam_depth_map = cv::imread(src_camera_depth_map_file_name.c_str(), -1);
  this->depth_map = kwiver::vital::image_container_sptr
    (new arrows::ocv::image_container(src_cam_depth_map, arrows::ocv::image_container::OTHER_COLOR));
}


// ---------------------------------------------------------------------------
bool
transfer_with_depth_map::
check_configuration( vital::config_block_sptr config ) const
{
  kwiver::vital::config_difference cd( this->get_configuration(), config );
  const auto key_list = cd.extra_keys();

  if ( ! key_list.empty() )
  {
    LOG_WARN( logger(), "Additional parameters found in config block that are "
                        "not required or desired: "
                        << kwiver::vital::join( key_list, ", " ) );
  }

  return true;
}


// ---------------------------------------------------------------------------
vector_3d
transfer_with_depth_map::
backproject_to_depth_map(kwiver::vital::camera_perspective_sptr const camera,
                         kwiver::vital::image_container_sptr const depth_map,
                         vector_2d const& img_pt) const
{
  return vector_3d(0.0, 0.0, 0.0);
}

// ---------------------------------------------------------------------------
vital::detected_object_set_sptr
transfer_with_depth_map::
filter( vital::detected_object_set_sptr const input_set ) const
{
  auto ret_set = std::make_shared<vital::detected_object_set>();

  // loop over all detections
  for ( auto det : *input_set )
  {
    auto out_det = det->clone();
    // auto out_box = out_det->bounding_box();
    // auto new_out_box = this->transform_bounding_box(out_box);
    // out_det->set_bounding_box( new_out_box );
    ret_set->add( out_det );
  } // end foreach detection

  return ret_set;
} // transform_detected_object_set::filter

}}} // end namespace
