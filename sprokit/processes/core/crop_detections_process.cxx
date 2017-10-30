/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS [yas] elisp error!AS IS''
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

#include "crop_detections_process.h"

#include <vital/vital_types.h>

#include <vital/types/bounding_box.h>
#include <vital/types/detected_object_set.h>
#include <vital/types/image_container.h>
#include <vital/types/image_container_set.h>

#include <vital/algo/crop_chips.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>


namespace algo = kwiver::vital::algo;

namespace kwiver
{

//----------------------------------------------------------------
// Private implementation class
class crop_detections_process::priv
{
public:
  priv();
  ~priv();

  algo::crop_chips_sptr m_algo;
};

// ================================================================

crop_detections_process
::crop_detections_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new crop_detections_process::priv )
{
  attach_logger( kwiver::vital::get_logger( name() ) );

  make_ports();
  make_config();
}


crop_detections_process
::~crop_detections_process()
{
}


// ----------------------------------------------------------------
void crop_detections_process
::_configure()
{
  kwiver::vital::config_block_sptr algo_config = get_config();

  algo::crop_chips::set_nested_algo_configuration(
    "crop_chips", algo_config, d->m_algo );

  if( !d->m_algo )
  {
    throw sprokit::invalid_configuration_exception(
      name(), "Unable to create \"crop_chips\"" );
  }
  algo::crop_chips::get_nested_algo_configuration(
    "crop_chips", algo_config, d->m_algo );

  // Check config so it will give run-time diagnostic of config problems
  if( !algo::crop_chips::check_nested_algo_configuration(
        "crop_chips", algo_config ) )
  {
    throw sprokit::invalid_configuration_exception( name(),
      "Configuration check failed." );
  }
}


// ----------------------------------------------------------------
void
crop_detections_process
::_step()
{
  // Get inputs
  kwiver::vital::image_container_sptr in_img;
  kwiver::vital::detected_object_set_sptr in_detections;
  in_img = grab_from_port_using_trait( image );
  in_detections = grab_from_port_using_trait( detected_object_set );

  // Transform detections into a vector of bounding boxes
  std::vector<kwiver::vital::bounding_box_d> in_bboxes;
  //for (auto dobj : in_detections.get())
  //{
  //  in_bboxes.push_back(dobj.bounding_box())
  //}

  // Extract the chips
  kwiver::vital::image_container_set_sptr chips;
  chips = d->m_algo->crop( in_img, in_bboxes );

  // Push to the output port
  push_to_port_using_trait( image_set, chips );
}


// ----------------------------------------------------------------
void crop_detections_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  required.insert( flag_required );

  // -- input --
  declare_input_port_using_trait( image, required );
  declare_input_port_using_trait( detected_object_set, required );

  // -- output --
  declare_output_port_using_trait( image_set, optional );
}


// ----------------------------------------------------------------
void crop_detections_process
::make_config()
{
}


// ================================================================
crop_detections_process::priv
::priv()
{
}


crop_detections_process::priv
::~priv()
{
}

} // end namespace
