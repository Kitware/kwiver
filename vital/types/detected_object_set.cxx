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

#include "detected_object_set.h"

#include <vital/vital_foreach.h>

#include <algorithm>
#include <stdexcept>

namespace kwiver {
namespace vital {

// ==================================================================
namespace {

struct descending_confidence
{
  bool operator()( detected_object_sptr const& a, detected_object_sptr const& b ) const
  {
    if( a && !b )
    {
      return true;
    }
    else if( !a )
    {
      return false;
    }
    return a->confidence() > b->confidence();
  }
};

template < typename T1, typename T2 >
struct more_first
{
  typedef std::pair< T1, T2 > type;
  bool operator()( type const& a, type const& b ) const
  {
    return a.first > b.first;
  }
};

} // end namespace


// ------------------------------------------------------------------
detected_object_set::
detected_object_set()
{ }


// ------------------------------------------------------------------
detected_object_set::
detected_object_set( detected_object::vector_t const& objs )
  : m_detected_objects( objs )
{
  // sort objects based on confidence
  std::sort( m_detected_objects.begin(), m_detected_objects.end(),
             descending_confidence() );
}


// ------------------------------------------------------------------
detected_object_set_sptr
detected_object_set::
clone() const
{
  auto new_obj = std::make_shared<detected_object_set>();

  const auto det_list = const_cast< detected_object_set* >(this)->select();
  VITAL_FOREACH( const auto det, det_list)
  {
    // copy detection
    new_obj->add( det->clone() );
  }

  // duplicate attributes
  if ( this->m_attrs )
  {
    new_obj->m_attrs = this->m_attrs->clone();
  }

  return new_obj;
}


// ------------------------------------------------------------------
void
detected_object_set::
add( detected_object_sptr object )
{
  // keep list ordered
  m_detected_objects.insert (
      std::upper_bound( m_detected_objects.begin(), m_detected_objects.end(),
                        object, descending_confidence() ),
      object
    );
}


// ------------------------------------------------------------------
size_t
detected_object_set::
size() const
{
  return m_detected_objects.size();
}


// ------------------------------------------------------------------
detected_object::vector_t
detected_object_set::
select( double threshold )
{
  // The main list can get out of order if somebody updates the
  // confidence value of a detection directly
  detected_object::vector_t vect;

  VITAL_FOREACH( auto i, m_detected_objects )
  {
    if ( i && i->confidence() >= threshold )
    {
      vect.push_back( i );
    }
  }

  std::sort( vect.begin(), vect.end(), descending_confidence() );
  return vect;
}


// ------------------------------------------------------------------
detected_object::vector_t
detected_object_set::
select( const std::string& class_name, double threshold )
{
  // Intermediate sortable data structure
  std::vector< std::pair< double, detected_object_sptr > > data;

  // Create a sortable list by selecting
  VITAL_FOREACH( auto i, m_detected_objects )
  {
    auto obj_type = i->type();
    if ( ! obj_type )
    {
      continue;  // Must have a type assigned
    }

    double score(0);
    try
    {
      score = obj_type->score( class_name );
    }
    catch (const std::runtime_error& )
    {
      // Object did not have the desired class_name. This not fatal,
      // but since we are looking for that name, there is some
      // expectation that it is present.

      //+ maybe log something?
      continue;
    }

    // Select those not below threshold
    if ( score >= threshold )
    {
      data.push_back( std::pair< double, detected_object_sptr >( score, i ) );
    }
  } // end foreach

  // Sort on score
  std::sort( data.begin(), data.end(), more_first< double,  detected_object_sptr >() );

  // Create new vector for return
  detected_object::vector_t vect;

  VITAL_FOREACH( auto i, data )
  {
    vect.push_back( i.second );
  }

  return vect;
}


// ------------------------------------------------------------------
kwiver::vital::attribute_set_sptr
detected_object_set::
attributes()
{
  return m_attrs;
}


// ------------------------------------------------------------------
void
detected_object_set::
set_attributes( attribute_set_sptr attrs )
{
  m_attrs = attrs;
}

} } // end namespace
