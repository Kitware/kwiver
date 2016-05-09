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

/**
 * \file
 * \brief Header for \link kwiver::vital::detected_object_set detected_object_set \endlink class
 */

#ifndef VITAL_DETECTED_OBJECT_SET_H_
#define VITAL_DETECTED_OBJECT_SET_H_

#include <vector>

#include <vital/vital_export.h>
#include <vital/vital_config.h>

#include <vital/types/detected_object.h>
#include <vital/types/object_labels.h>

namespace kwiver {
namespace vital {

/// forward declaration of detected_object class
class detected_object_set;
/// typedef for a detected_object shared pointer
typedef std::shared_ptr< detected_object_set > detected_object_set_sptr;

// ----------------------------------------------------------------

class VITAL_EXPORT detected_object_set
{
public:
  class iterator
  {
    friend class detected_object_set;
  public:
    iterator& operator++();
    iterator operator++(int);
    bool is_end() const;
    detected_object_sptr get_object() const;
    size_t at() const;
    size_t size() const;
    detected_object_sptr operator[](size_t i) const;
  private:
    size_t at_;
    iterator(detected_object_set const& set,
             std::vector<size_t> desired);
    detected_object_set const& set_;
    std::vector<size_t> desired_value_;
  };
  detected_object_set(std::vector<detected_object_sptr> const& objs,
                      object_labels_sptr labels = NULL);
  detected_object_sptr operator[](size_t i) const;
  size_t size() const;
  iterator get_iterator(bool sorted = false) const;
  iterator get_iterator(object_labels::key label, bool sorted = false,
                        double threshold = object_type::INVALID_SCORE) const;
  iterator get_iterator(std::string label, bool sorted = false,
                        double threshold = object_type::INVALID_SCORE) const;
  object_labels::iterator get_labels() const;
private:
  object_labels_sptr labels_;
  std::vector<detected_object_sptr> objects_;
};

}
}
#endif
