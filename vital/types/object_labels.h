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
 * \brief Header for \link kwiver::vital::object_labels object_labels \endlink 
 * class
 */

#ifndef VITAL_OBJECT_LABELS_H_
#define VITAL_OBJECT_LABELS_H_

#include <vital/vital_export.h>

#include <vector>
#include <map>
#include <string>
#include <memory>

#include <vital/logger/logger.h>

namespace kwiver {
namespace vital {

/// forward declaration of object_labels class
class object_labels;
/// typedef for a object_labels shared pointer
typedef std::shared_ptr< object_labels > object_labels_sptr;

// ----------------------------------------------------------------
/** Stores object labels
 *
 * Stores the string representation for the object types stored in 
 * \link kwiver::vital::object_type \endlink
 */

class VITAL_EXPORT object_labels
{
public:
  typedef size_t key;
  static const std::string INVALID_LABEL;
  static const key INVALID_KEY;

  class iterator
  {
    friend object_labels;
  public:
    iterator& operator++();
    iterator operator++(int);
    std::string const& get_label() const;
    key get_key() const;
    bool is_end() const;
  private:
    size_t at_;
    iterator(std::vector<std::string> const& labels);
    std::vector<std::string> const& labels_;
  };

  /** Constructs a label set based on vector.  Each label must be
   *  unique and not equal to invalid.
   */
  object_labels(std::vector<std::string> labels);

  /** Constructs a label set based on map. Keys must be unique.*/
  object_labels(std::map<std::string, key> labels);

  std::string const& get_label(key k) const;

  key get_key(std::string const& label) const;

  size_t get_number_of_labels() const;

  ///To iterate over labels
  iterator get_iterator() const;

private:
  std::map<std::string, key> string_id_to_key_;
  std::vector<std::string> key_to_string_;

  kwiver::vital::logger_handle_t logger_;
};

}
}
#endif
