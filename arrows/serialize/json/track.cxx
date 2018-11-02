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

#include "track.h"
#include <arrows/serialize/json/load_save.h>
#include <vital/types/track.h>
#include <vital/types/object_track_set.h>
#include <vital/internal/cereal/cereal.hpp>
#include <vital/internal/cereal/types/vector.hpp>
#include <vital/internal/cereal/archives/json.hpp>
#include <vital/internal/cereal/types/utility.hpp>
#include <vital/internal/cereal/types/polymorphic.hpp>
#include <sstream>
#include <iostream>


namespace kwiver {
namespace arrows {
namespace serialize {
namespace json {

struct track_item
{
  kwiver::vital::track_sptr trk_sptr;

  track_item()
  {
    trk_sptr = kwiver::vital::track::create();
  }

  track_item(kwiver::vital::track_sptr& _trk_sptr )
  {
    trk_sptr = _trk_sptr;
  }

  kwiver::vital::track_sptr& get_track()
  {
    return trk_sptr;
  }

  template<class Archive>
  void save ( Archive& archive ) const
  {
    archive( cereal::make_nvp( "track_id", trk_sptr->id() ) );
    archive( cereal::make_nvp( "track_size", trk_sptr->size() ) );
    std::vector<kwiver::vital::track_state_sptr> trk;
    for ( auto trk_state_itr=trk_sptr->begin(); trk_state_itr!=trk_sptr->end();
              ++trk_state_itr)
    {
      auto trk_state = *trk_state_itr;
      trk.push_back(trk_state);
    }
    archive(cereal::make_nvp( "trk", trk) );
  }
  

  template<class Archive>
  void load ( Archive& archive ) 
  {
    size_t track_size;
    kwiver::vital::track_id_t track_id;
    archive( CEREAL_NVP( track_size ) );
    archive( CEREAL_NVP( track_id) );
    std::vector<kwiver::vital::track_state_sptr> trk;
    archive( CEREAL_NVP( trk ));
    trk_sptr->set_id(track_id);
    for (auto trk_state : trk)
    {
      bool trk_inserted = trk_sptr->insert(trk_state);
      if ( !trk_inserted ) 
      {
        LOG_ERROR( kwiver::vital::get_logger( "data_serializer" ),
                 "Failed to insert track state in track" );
      }
    }
  }
};

// ----------------------------------------------------------------------------
track::
track()
{ }


track::
~track()
{ }

// ----------------------------------------------------------------------------
std::shared_ptr< std::string >
track::
serialize( const vital::any& element )
{
  kwiver::vital::track_sptr trk_sptr =
    kwiver::vital::any_cast< kwiver::vital::track_sptr > ( element ); 
  kwiver::arrows::serialize::json::track_item trk_item(trk_sptr);
  std::stringstream msg;
  msg << "track "; // add type tag
  {
    cereal::JSONOutputArchive ar( msg );
    ar( trk_item );
  }
  return std::make_shared< std::string > ( msg.str() );
}


// ----------------------------------------------------------------------------
vital::any track::
deserialize( const std::string& message )
{
  std::stringstream msg(message);
  kwiver::arrows::serialize::json::track_item trk_item = track_item();
  std::string tag;
  msg >> tag;

  if (tag != "track" )
  {
    LOG_ERROR( logger(), "Invalid data type tag received. Expected \"track\", received \""
               << tag << "\". Message dropped, returning default object." );
  }
  else
  {
    cereal::JSONInputArchive ar( msg );
    ar( trk_item );
  }
  return kwiver::vital::any( trk_item.get_track() );
}


} } } }       // end namespace kwiver
