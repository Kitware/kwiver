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

#include "object_labels.h"
#include <stdexcept>
#include <limits>

namespace kwiver {
namespace vital {

const std::string object_labels::INVALID_LABEL("");
const object_labels::key object_labels::INVALID_KEY = std::numeric_limits<size_t>::max();

object_labels::iterator&
object_labels::iterator::operator++()
{
  do
  {
    at_++;
  }while(!is_end() && labels_[at_] == object_labels::INVALID_LABEL);
  return *this;
}

object_labels::iterator
object_labels::iterator::operator++(int)
{
  object_labels::iterator tmp(*this); // copy
  operator++(); // pre-increment
  return tmp;
}

std::string const&
object_labels::iterator::get_label() const
{
  if(is_end())
  {
    return INVALID_LABEL;
  }
  return labels_[at_];
}

object_labels::key
object_labels::iterator::
get_key() const
{
  if(is_end())
  {
    return INVALID_KEY;
  }
  return at_;
}

bool
object_labels::iterator::
is_end() const
{
  return at_ >= labels_.size();
}

object_labels::iterator::iterator(std::vector<std::string> const& labels)
: at_(0), labels_(labels)
{
}

object_labels::
object_labels(std::vector<std::string> labels)
: key_to_string_(labels),
  logger_( kwiver::vital::get_logger( "vital.object_labels" ) )
{
  for(size_t i = 0; i < labels.size(); ++i)
  {
    if(labels[i] == INVALID_LABEL)
    {
      LOG_WARN( logger_, "::object_labels: user used an INVALID label in input" );
    }
    else if(string_id_to_key_.find(labels[i]) != string_id_to_key_.end())
    {
      throw std::invalid_argument("object_labels::object_labels: provided duplicate label");
    }
    else
    {
      string_id_to_key_[labels[i]] = i;
    }
  }
}

object_labels::
object_labels(std::map<std::string, object_labels::key> labels)
{
  for(std::map<std::string, object_labels::key>::const_iterator iter = labels.begin();
      iter != labels.end(); ++iter)
  {
    if(iter->second == INVALID_KEY)
    {
      LOG_WARN( logger_, "::object_labels: user used an INVALID key in input, ingoring" );
      continue;
    }

    if(iter->first == INVALID_LABEL)
    {
      LOG_WARN( logger_, "::object_labels: user used an INVALID label in input, ignoring" );
      continue;
    }

    if(iter->second >= key_to_string_.size())
    {
      key_to_string_.resize(iter->second+1, INVALID_LABEL);
    }

    if(key_to_string_[iter->second] != INVALID_LABEL)
    {
      throw std::invalid_argument("object_labels::object_labels: provided duplicate label");
    }
    key_to_string_[iter->second] = iter->first;
  }
}

std::string const&
object_labels::
get_label(object_labels::key k) const
{
  if(k >= key_to_string_.size()) return INVALID_LABEL;
  return key_to_string_[k];
}

object_labels::key
object_labels::
get_key(std::string const& label) const
{
  std::map<std::string, object_labels::key>::const_iterator iter = string_id_to_key_.find(label);
  if(iter == string_id_to_key_.end()) return INVALID_KEY;
  return iter->second;
}

size_t
object_labels::
get_number_of_labels() const
{
  return key_to_string_.size();
}

object_labels::iterator
object_labels::
get_iterator() const
{
  return iterator(key_to_string_);
}

}
}
