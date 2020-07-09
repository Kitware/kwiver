/*ckwg +29
 * Copyright 2018-2020 by Kitware, Inc.
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

#include "detected_object.h"

#include "class_map.h"
#include "bounding_box.h"

#include <vital/types/detected_object.h>
#include <vital/internal/cereal/cereal.hpp>
#include <vital/internal/cereal/archives/json.hpp>

#include <sstream>

namespace kasj = kwiver::arrows::serialize::json;

namespace kwiver {
namespace arrows {
namespace serialize {
namespace json {

// ----------------------------------------------------------------------------
detected_object::
detected_object()
{ }


detected_object::
~detected_object()
{ }

// ----------------------------------------------------------------------------
std::shared_ptr< std::string >
detected_object::
serialize( const vital::any& element )
{
  // Get native data type from any
  kwiver::vital::detected_object_sptr obj =
    kwiver::vital::any_cast< kwiver::vital::detected_object_sptr > ( element );

  std::stringstream msg;
  msg << "detected_object ";
  {
    cereal::JSONOutputArchive ar( msg );
    save( ar, *obj );
  }

  return std::make_shared< std::string > ( msg.str() );
}

// ----------------------------------------------------------------------------
vital::any detected_object::
deserialize( const std::string& message )
{
  std::stringstream msg(message);
  auto obj = std::make_shared< kwiver::vital::detected_object >( kwiver::vital::bounding_box_d { 0, 0, 0, 0 } );

  std::string tag;
  msg >> tag;

  if (tag != "detected_object" )
  {
    LOG_ERROR( logger(), "Invalid data type tag received. Expected \"detected_object\", received \""
               << tag << "\". Message dropped. Default object returned." );
  }
  else
  {
    cereal::JSONInputArchive ar( msg );
    load( ar, *obj );
  }

  return kwiver::vital::any(obj);
}

} } } }       // end namespace kwiver
