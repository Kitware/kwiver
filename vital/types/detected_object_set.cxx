/* Copyright 2016 by Kitware, Inc.
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

#include "detected_object_set.h"
#include <stdexcept>
#include <limits>

namespace kwiver {
namespace vital {


detected_object_set::iterator& detected_object_set::iterator::operator++()
{
  at_++;
  return *this;
}

detected_object_set::iterator detected_object_set::iterator::operator++(int)
{
  detected_object_set::iterator tmp(*this); //copy
  operator++();
  return tmp;
}

bool detected_object_set::iterator::is_end() const
{
  return at_ >= desired_value_.size();
}

detected_object_sptr detected_object_set::iterator::get_object() const
{
  if(is_end()) return NULL;
  return set_[desired_value_[at_]];
}

size_t detected_object_set::iterator::at() const
{
  return at_;
}

size_t detected_object_set::iterator::size() const
{
  return desired_value_.size();
}

detected_object_sptr detected_object_set::iterator::operator[](size_t i) const
{
  return set_[desired_value_[i]];
}

detected_object_set::iterator::iterator(detected_object_set const& set,
                                        std::vector<size_t> desired)
: at_(0), set_(set), desired_value_(desired)
{
}


detected_object_set::detected_object_set(std::vector<detected_object_sptr> const& objs, object_labels_sptr labels)
:labels_(labels), objects_(objs)
{
  if(labels != NULL)
  {
    for(unsigned int i = 0; i < objs.size(); ++i)
    {
      object_type_sptr obj = objs[i]->get_classifications();
      if(obj == NULL)
      {
        throw std::invalid_argument("detected_object_set::detected_object_set: Because we have labels, object type cannot be null");
      }
      if(obj->labels() != labels)
      {
        throw std::invalid_argument("detected_object_set::detected_object_set: labels needs to match the labels of all the objects");
      }
    }
  }
}

detected_object_sptr detected_object_set::operator[](size_t i) const
{
  if(i > this->objects_.size())
  {
    throw std::out_of_range("detected_object_set::operator[]");
  }
  return this->objects_[i];
}

size_t detected_object_set::size() const
{
  return this->objects_.size();
}

detected_object_set::iterator detected_object_set::get_iterator(bool sorted) const
{
  std::vector<size_t> desired;
  desired.reserve(this->size());
  for(size_t i = 0; i < this->size(); ++i)
  {
    if( objects_[i]->get_confidence() !=  object_type::INVALID_SCORE )
    {
      desired.push_back(i);
    }
  }

  if(sorted)
  {
    struct sort_function
    {
      detected_object_set const* set;
      bool operator()(size_t const& l, size_t const& r) const
      {
        if(l >= set->size() || r >= set->size()) return false;
        return (*set)[l]->get_confidence() >= (*set)[r]->get_confidence();
      }
    } sort_fun;
    sort_fun.set = this;
    std::sort(desired.begin(), desired.end(), sort_fun);
  }
  return iterator(*this, desired);
}

detected_object_set::iterator detected_object_set::get_iterator(object_labels::key key, bool sorted, double threshold) const
{
  if(labels_ == NULL)
  {
    throw std::runtime_error("detected_object_set::get_iterator: cannot create iterator when labels are NULL");
  }
  std::vector<size_t> desired;
  desired.reserve(this->size());
  std::vector<double> scores;
  scores.reserve(this->size());
  for(unsigned int i = 0; i < this->size(); ++i)
  {
    object_type_sptr ots = this->objects_[i]->get_classifications();
    if(ots->get_score(key) !=  object_type::INVALID_SCORE && ots->get_score(key) > threshold)
    {
      desired.push_back(i);
      scores.push_back(ots->get_score(key));
    }
    else
    {
       scores.push_back(object_type::INVALID_SCORE);
    }
  }

  if(sorted)
  {
    struct sort_function
    {
      std::vector<double> const* scores;
      bool operator()(size_t const& l, size_t const& r) const
      {
        if(l >= scores->size() || r >= scores->size()) return false;
        return (*scores)[l] > (*scores)[r] || ((*scores)[l] == (*scores)[r] && l < r); //if the scores are equal, we preserve the original order
      }
    } sort_fun;
    sort_fun.scores = &scores;
    std::sort(desired.begin(), desired.end(), sort_fun);
  }
  return iterator(*this, desired);
}

detected_object_set::iterator detected_object_set::get_iterator(std::string label, bool sorted, double threshold) const
{
  if(labels_ == NULL)
  {
    throw std::runtime_error("detected_object_set::get_iterator: cannot create iterator when labels are NULL");
  }
  return get_iterator(labels_->get_key(label), sorted, threshold);
}

object_labels::iterator detected_object_set::get_labels() const
{
  if(labels_ == NULL)
  {
    throw std::runtime_error("detected_object_set::get_labels: get label iterator when labels are NULL");
  }
  return labels_->get_iterator();
}

}
}
