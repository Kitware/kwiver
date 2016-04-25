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
 * \brief Header for \link kwiver::vital::object_type object_type \endlink
 * class
 */

#ifndef VITAL_OBJECT_TYPE_H_
#define VITAL_OBJECT_TYPE_H_

#include <vital/vital_export.h>

#include <vector>
#include <memory>

#include <vital/types/object_labels.h>

namespace kwiver {
namespace vital {

/// forward declaration of object_type class
class object_type;
/// typedef for a object_type shared pointer
typedef std::shared_ptr< object_type > object_type_sptr;

// ----------------------------------------------------------------

class VITAL_EXPORT object_type
{
public:
  static const double INVALID_SCORE;

  class iterator
  {
    friend class object_type;
  public:
    iterator& operator++();
    iterator operator++(int);
    std::string const& get_label() const;
    object_labels::key get_key() const;
    bool is_end() const;
    double get_score() const;
  private:
    size_t at_;
    iterator(object_type const* types,
             std::vector<object_labels::key> desired_values);
    object_type const* types_;
    std::vector<object_labels::key> desired_values_;
  };

  object_type(object_labels_sptr labels,
              std::vector<double> likelyhoods = std::vector<double>());
  double get_score(object_labels::key k) const;
  double get_score(std::string const& label) const;
  bool set_score(object_labels::key k, double d);
  bool set_score(std::string const& label, double d);
  bool set_scores(std::vector<double> const& d);
  std::string const& get_label(object_labels::key k) const;
  object_labels::key get_key(std::string const& label) const;
  iterator get_iterator(bool sort = false, double threshold = INVALID_SCORE) const;
  object_labels_sptr labels() const;
private:
  object_labels_sptr labels_;
  std::vector<double> likelyhoods_;
  kwiver::vital::logger_handle_t logger_;
};

}
}

#endif
