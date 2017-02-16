/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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

#include "non_maximual_supression.h"

#include <vital/types/detected_object_type.h>
#include <vital/vital_foreach.h>

namespace kwiver {
namespace arrows {
namespace core {


// ------------------------------------------------------------------
non_maximual_supression::
non_maximual_supression()
  : m_overlap_threshold(0.3)
{ }


// ------------------------------------------------------------------
vital::config_block_sptr non_maximual_supression::
get_configuration() const
{
   // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value("overlap_threshold", m_overlap_threshold,
                    "The threshold to consider the bounding box is potentially the same object.");

  return config;
}


// ------------------------------------------------------------------
void non_maximual_supression::
set_configuration(vital::config_block_sptr config_in)
{
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config(config_in);
  this->m_overlap_threshold = config->get_value<double>("overlap_threshold");
}


// ------------------------------------------------------------------
bool non_maximual_supression::
check_configuration(vital::config_block_sptr config) const
{
  return true;
}


// ------------------------------------------------------------------
vital::detected_object_set_sptr
non_maximual_supression::
filter( const vital::detected_object_set_sptr input_set ) const
{
  // Need to clone input because we are modifying the set in this filter.
  auto output_set = input_set->clone();
  auto det_list = output_set->select();

  // Depending on the application, the first detection *should* have all
  // the class names for a set.
  auto class_name_list = kwiver::vital::detected_object_type::all_class_names();

  VITAL_FOREACH( auto current_class, class_name_list )
  {
    const size_t det_list_limit( det_list.size() );
    for ( size_t i = 0; i < det_list_limit; ++i )
    {
      if ( ! det_list[i]->type()->has_class_name( current_class ) )
      {
        // This detection does not have current class name
        continue;
      }

      const auto bbox_i = det_list[i]->bounding_box();
      const auto ai = bbox_i.area();

      for (size_t j = i+1; j < det_list_limit; ++j)
      {
        // does this [j] detection have the current label
        if ( ! det_list[j]->type()->has_class_name( current_class ) )
        {
          continue;
        }

        // real do-it code
        const auto bbox_j = det_list[j]->bounding_box();
        auto inter = intersection( bbox_i, bbox_j ); // determine overlap box
        const double aj = bbox_j.area();
        const double interS = inter.area();
        const double t = interS / ( ai + aj - interS );

        // If over the threshold, remove this class-name
        if ( t >= this->m_overlap_threshold )
        {
          det_list[j]->type()->delete_score( current_class );
        }
      } // end for j (over rest of list)
    } // end for i (over full det list)
  } // end for class-names

  return output_set;
} // non_maximual_supression::filter

} } }  // end namespace
