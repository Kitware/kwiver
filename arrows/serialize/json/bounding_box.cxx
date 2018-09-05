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

#include "bounding_box.h"

#include <vital/types/bounding_box.h>
#include <vital/internal/cereal/cereal.hpp>
#include <vital/internal/cereal/archives/json.hpp>

#include <sstream>

namespace kwiver {
namespace arrows {
namespace serialize {
namespace json {

// ----------------------------------------------------------------------------
bounding_box::
bounding_box()
{
  m_element_names.insert( DEFAULT_ELEMENT_NAME );
}


bounding_box::
~bounding_box()
{ }

// ----------------------------------------------------------------------------
std::shared_ptr< std::string >
bounding_box::
serialize( const data_serializer::serialize_param_t& elements )
{
  kwiver::vital::bounding_box_d bbox =
    kwiver::vital::any_cast< kwiver::vital::bounding_box_d > ( elements.at( DEFAULT_ELEMENT_NAME ) );

  std::stringstream msg;
  msg << "bounding_box "; // add type tag
  {
    cereal::JSONOutputArchive ar( msg );
    save( ar, bbox );
  }

  return std::make_shared< std::string > ( msg.str() );
}


// ----------------------------------------------------------------------------
vital::algo::data_serializer::deserialize_result_t
bounding_box::
deserialize( std::shared_ptr< std::string > message )
{
  std::stringstream msg(*message);
  kwiver::vital::bounding_box_d bbox{ 0, 0, 0, 0 };
  std::string tag;
  msg >> tag;

  if (tag != "bounding_box" )
  {
    LOG_ERROR( logger(), "Invalid data type tag received. Expected \"bounding_box\", received \""
               << tag << "\". Message dropped." );
  }
  else
  {
    cereal::JSONInputArchive ar( msg );
    load( ar, bbox );
  }

  deserialize_result_t res;
  res[ DEFAULT_ELEMENT_NAME ] = kwiver::vital::any(bbox);

  return res;
}

// ----------------------------------------------------------------------------
void
bounding_box::
save( cereal::JSONOutputArchive& archive, const kwiver::vital::bounding_box_d& bbox )
{
  archive( cereal::make_nvp( "min_x", bbox.min_x() ),
           cereal::make_nvp( "min_y", bbox.min_y() ),
           cereal::make_nvp( "max_x", bbox.max_x() ),
           cereal::make_nvp( "max_y", bbox.max_y() ) );
}

// ----------------------------------------------------------------------------
void
bounding_box::
load( cereal::JSONInputArchive& archive, kwiver::vital::bounding_box_d& bbox )
{
  double min_x, min_y, max_x, max_y;

  archive( CEREAL_NVP( min_x ),
           CEREAL_NVP( min_y ),
           CEREAL_NVP( max_x ),
           CEREAL_NVP( max_y ) );

  bbox = kwiver::vital::bounding_box_d( min_x, min_y, max_x, max_y );
}

} } } }       // end namespace kwiver
