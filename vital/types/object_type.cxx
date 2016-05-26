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

#include "object_type.h"
#include <stdexcept>
#include <limits>
#include <algorithm>

namespace kwiver {
namespace vital {

const double object_type::INVALID_SCORE = std::numeric_limits< double >::min();

object_type::iterator&
object_type::iterator::operator++()
{
  do
  {
    at_++;
  }
  while ( ! is_end() &&
          types_->get_score( desired_values_[at_] ) == object_type::INVALID_SCORE );
  return *this;
}


object_type::iterator
object_type::iterator::operator++( int )
{
  object_type::iterator tmp( *this ); // copy

  operator++(); // pre-increment
  return tmp;
}


std::string const&
object_type::iterator::get_label() const
{
  if ( at_ >= desired_values_.size() )
  {
    return object_labels::INVALID_LABEL;
  }
  return types_->get_label( desired_values_[at_] );
}


object_labels::key
object_type::iterator::get_key() const
{
  if ( at_ < desired_values_.size() )
  {
    return desired_values_[at_];
  }
  return object_labels::INVALID_KEY;
}


bool
object_type::iterator::is_end() const
{
  return at_ >= desired_values_.size();
}


double
object_type::iterator::get_score() const
{
  if ( at_ >= desired_values_.size() )
  {
    return object_type::INVALID_SCORE;
  }
  return types_->get_score( desired_values_[at_] );
}


object_type::iterator::iterator( object_type const*                 types,
                                 std::vector< object_labels::key >  desired_values )
  : at_( 0 ), types_( types ), desired_values_( desired_values )
{
}


object_type::object_type( object_labels_sptr    labels,
                          std::vector< double > likelyhoods )
  : labels_( labels ), likelyhoods_( likelyhoods ),
  logger_( kwiver::vital::get_logger( "vital.object_types" ) )
{
  if ( labels_ == NULL )
  {
    throw std::invalid_argument( "object_type::object_type: need valid labels" );
  }

  if ( likelyhoods.empty() )
  {
    likelyhoods_.resize( labels_->get_number_of_labels(), INVALID_SCORE );
  }

  if ( likelyhoods.size() != labels_->get_number_of_labels() )
  {
    throw std::invalid_argument( "object_type::object_type: Labels and "
                                 "likelyhoods do not have the same number of keys" );
  }
}


double
object_type::get_score( object_labels::key k ) const
{
  if ( k >= likelyhoods_.size() )
  {
    return INVALID_SCORE;
  }
  return likelyhoods_[k];
}


double
object_type::get_score( std::string const& label ) const
{
  return get_score( labels_->get_key( label ) );
}

double object_type::get_max_score(std::string & max_label) const
{
  double max = INVALID_SCORE;
  max_label = "";
  for( unsigned int i = 0; i < likelyhoods_.size(); ++i)
  {
    if ( max < likelyhoods_[i] )
    {
      max = likelyhoods_[i];
      max_label = labels_->get_label(i);
    }
  }
  return max;
}


bool
object_type::set_score( object_labels::key k, double d )
{
  if ( k < likelyhoods_.size() )
  {
    likelyhoods_[k] = d;
    return true;
  }
  return false;
}


bool
object_type::set_score( std::string const& label, double d )
{
  return set_score( labels_->get_key( label ), d );
}


bool
object_type::set_scores( std::vector< double > const& d )
{
  if ( d.size() != likelyhoods_.size() )
  {
    return false;
  }
  likelyhoods_ = d;
  return true;
}


std::string const&
object_type::get_label( object_labels::key k ) const
{
  return labels_->get_label( k );
}


object_labels::key
object_type::get_key( std::string const& label ) const
{
  return labels_->get_key( label );
}


object_type::iterator
object_type::get_iterator( bool sort, double threshold ) const
{
  std::vector< object_labels::key > valid_keys;

  for ( size_t i = 0; i < likelyhoods_.size(); ++i )
  {
    if ( likelyhoods_[i] > threshold )
    {
      valid_keys.push_back( i );
    }
  }
  if ( sort )
  {
    struct sort_function
    {
      std::vector< double > const* scores;
      bool operator()( object_labels::key const& l, object_labels::key const& r ) const
      {
        if ( ( l >= scores->size() ) || ( r >= scores->size() ) ) { return false; }
        return ( *scores )[l] > ( *scores )[r];
      }


    } sort_fun;

    sort_fun.scores = &likelyhoods_;
    std::sort( valid_keys.begin(), valid_keys.end(), sort_fun );
  }

  return iterator( this, valid_keys );
}


object_labels_sptr
object_type::labels() const
{
  return labels_;
}

} }
