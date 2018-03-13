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

#include "max_count_filter.h"

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

// Implementation of https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
// from https://stackoverflow.com/questions/9345087/choose-m-elements-randomly-from-a-vector-containing-n-elements
template<class bidiiter>
bidiiter random_unique( bidiiter begin, bidiiter end, size_t num_random ) {
    size_t left = std::distance(begin, end);
    while ( num_random-- ) {
        bidiiter r = begin;
        std::advance( r, rand()%left );
        std::swap( *begin, *r );
        ++begin;
        --left;
    }
    return begin;
}


// ------------------------------------------------------------------
max_count_filter::max_count_filter()
  : m_randomize( false )
  , m_max_count( 1 )
{
}


// ------------------------------------------------------------------
vital::config_block_sptr
max_count_filter::get_configuration() const
{
  // Get base config from base class
  vital::config_block_sptr config = vital::algorithm::get_configuration();

  config->set_value( "max_count", m_max_count,
                     "A maximum of this many detections are passed through the filter" );


  config->set_value( "randomize", m_randomize,
                     "Items are selected randomly up to max_count if this is true." );

  return config;
}


// ------------------------------------------------------------------
void
max_count_filter::
set_configuration( vital::config_block_sptr config_in )
{
  vital::config_block_sptr config = this->get_configuration();

  config->merge_config( config_in );
  this->m_max_count = config->get_value< int > ( "max_count" );
  this->m_randomize = config->get_value< bool > ( "randomize" );
}


// ------------------------------------------------------------------
bool
max_count_filter::
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
max_count_filter::
filter( const vital::detected_object_set_sptr input_set ) const
{

  // If we have fewer than or exactly max_count detections, no-op
  if ( this->m_max_count >= input_set->size() )
    return input_set;

  // Get list of all detections from the set.
  // Select returns items sorted by descending_confidence
  auto working_set = input_set->clone();
  auto detections = working_set->select();

  if ( this->m_randomize ) {
		// Shuffle max_count random elements to the top
    random_unique( detections.begin(),detections.end(),this->m_max_count );
  }

  detections.resize( this->m_max_count );

  auto ret_set = std::make_shared<vital::detected_object_set>( detections );

  return ret_set;

} // class_probability_filter::filter

} } }     // end namespace
