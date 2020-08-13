/*ckwg +29
 * Copyright 2020 by Kitware, Inc.
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
 * \brief Interface for detected_object_type and activity_type classes
 */


#ifndef VITAL_CLASS_MAP_TYPES_H_
#define VITAL_CLASS_MAP_TYPES_H_

#include <vital/signal.h>
#include <vital/vital_export.h>
#include <vital/types/class_map.h>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

namespace kwiver {
namespace vital  {


class VITAL_EXPORT detected_object_type : public class_map<detected_object_type>
{
public:


  /**
   * @brief Create an empty object.
   *
   * An object is created without class_names or scores.
   */
  detected_object_type() : class_map<detected_object_type>() {};

  /**
   * @brief Create new object type class.
   *
   * Create a new object type instance with a set of labels and
   * likelyhoods. The parameters have corresponding ordering, which
   * means that the first label is for the first likelyhood , and so
   * on.
   *
   * The number of elements in the parameter vectors must be the same.
   *
   * @param class_names List of names for the possible classes.
   * @param scores Vector of scores for this object.*
   * @throws std::invalid_argument if the vector lengths differ
   */
  detected_object_type( const std::vector< std::string >& class_names,
             const std::vector< double >& scores ) : 
             class_map<detected_object_type>(class_names, scores) {};

  /**
   * @brief Create new object type class.
   *
   * Create a new object type instance from a single class name
   * and label.
   *
   * @param class_name Class name
   * @param score Probability score for the class
   */
  detected_object_type( const std::string& class_name,
             double score ) : 
             class_map<detected_object_type>(class_name, score) {};
};
// typedef for a detected_object_type shared pointer
using detected_object_type_sptr = std::shared_ptr< detected_object_type >;
using detected_object_type_scptr = std::shared_ptr< detected_object_type const >;

class VITAL_EXPORT activity_type : public class_map<activity_type>
{
public:
    /**
   * @brief Create an empty object.
   *
   * An object is created without class_names or scores.
   */
  activity_type() : class_map<activity_type>() {};

  /**
   * @brief Create new object type class.
   *
   * Create a new object type instance with a set of labels and
   * likelyhoods. The parameters have corresponding ordering, which
   * means that the first label is for the first likelyhood , and so
   * on.
   *
   * The number of elements in the parameter vectors must be the same.
   *
   * @param class_names List of names for the possible classes.
   * @param scores Vector of scores for this object.*
   * @throws std::invalid_argument if the vector lengths differ
   */
  activity_type( const std::vector< std::string >& class_names,
             const std::vector< double >& scores ) 
             : class_map<activity_type>(class_names, scores) {};

  /**
   * @brief Create new object type class.
   *
   * Create a new object type instance from a single class name
   * and label.
   *
   * @param class_name Class name
   * @param score Probability score for the class
   */
  activity_type( const std::string& class_name,
             double score ) 
             : class_map<activity_type>(class_name, score) {};
};
// typedef for a class_map shared pointer
using activity_type_sptr = std::shared_ptr< activity_type >;
using activity_type_scptr = std::shared_ptr< activity_type const >;

}
}

#endif //VITAL_CLASS_MAP_TYPES_H_
