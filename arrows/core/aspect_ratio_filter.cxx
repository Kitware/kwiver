/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#include "aspect_ratio_filter.h"

#include <vital/vital_foreach.h>
#include <vital/config/config_difference.h>
#include <vital/util/string.h>

namespace kwiver {
namespace arrows {
namespace core {
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>


// ------------------------------------------------------------------
aspect_ratio_filter::aspect_ratio_filter()
  : m_min_aspect_ratio( -1.0 )
  , m_max_aspect_ratio( -1.0 )
{
}


// ------------------------------------------------------------------
vital::config_block_sptr
aspect_ratio_filter::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "min_aspect_ratio", m_min_aspect_ratio,
                     "Only detections with a bounding box aspect ratio greater than this will pass." );


  config->set_value( "max_aspect_ratio", m_max_aspect_ratio,
                     "Only detections with a bounding box aspect ratio less than this will pass." );

  return config;
}


// ------------------------------------------------------------------
void
aspect_ratio_filter::
set_configuration( vital::config_block_sptr config_in )
{
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );
  this->m_min_aspect_ratio = config->get_value< double > ( "min_aspect_ratio" );
  this->m_max_aspect_ratio = config->get_value< double > ( "max_aspect_ratio" );
}


// ------------------------------------------------------------------
bool
aspect_ratio_filter::
check_configuration( vital::config_block_sptr config ) const
{
  kwiver::vital::config_difference cd( this->get_configuration(), config );
  const auto key_list = cd.extra_keys();

  if ( ! key_list.empty() )
  {
    LOG_WARN( logger(), "Additional parameters found in config block that are not required or desired: "
              << kwiver::vital::join( key_list, ", " ) );
    return false;
  }

  return true;
}


// ------------------------------------------------------------------
vital::detected_object_set_sptr
aspect_ratio_filter::
filter( const vital::detected_object_set_sptr input_set ) const
{
  auto ret_set = std::make_shared<vital::detected_object_set>();

  // Get list of all detections from the set.
  auto detections = input_set->select();

  // loop over all detections
  VITAL_FOREACH( auto det, detections )
  {
    bool det_selected( false );
    auto bbox = det->bounding_box();

    double aspect_ratio = 0.0;
    if ( bbox.height() != 0 )
    {
      aspect_ratio = bbox.height() / bbox.width();
    }

    // Invalid bboxes don't pass
    if ( aspect_ratio != 0.0 )
    {
      // Check if there's a setting and if this box passes it 
      // for min and max
      if ( ( m_min_aspect_ratio < 0 ||
            aspect_ratio > m_min_aspect_ratio ) &&
           ( m_max_aspect_ratio < 0 ||
             aspect_ratio < m_max_aspect_ratio )
         )
        det_selected = true;
    }

    if ( det_selected )
    {
      auto out_det = det->clone();
      ret_set->add( out_det );
    }
  }

  return ret_set;

}

} } }     // end namespace
