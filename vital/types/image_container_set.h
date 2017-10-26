/*ckwg +29
 * Copyright 2013-2014 by Kitware, Inc.
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
 * \brief core image_container_set interface
 */

#ifndef VITAL_IMAGE_CONTAINER_SET_H_
#define VITAL_IMAGE_CONTAINER_SET_H_


#include "image_container.h"
#include <vital/vital_config.h>
#include <vital/logger/logger.h>
#include <vital/noncopyable.h>

namespace kwiver {
namespace vital {

/// An abstract ordered collection of feature images.
/**
 * The base class image_container_set is abstract and provides an interface
 * for returning a vector of images.  There is a simple derived class
 * that stores the data as a vector of images and returns it.  Other
 * derived classes can store the data in other formats and convert on demand.
 */
class image_container_set : private noncopyable
{
public:
  /// Destructor
  virtual ~image_container_set() = default;

  /// Return the number of images in the set
  virtual size_t size() const = 0;

  /// Return a vector of image shared pointers
  virtual std::vector< image_container_sptr > images() const = 0;

protected:
  image_container_set()
   : m_logger( kwiver::vital::get_logger( "vital.image_container_set" ) )
  {}

  kwiver::vital::logger_handle_t m_logger;
};

/// Shared pointer for base image_container_set type
typedef std::shared_ptr< image_container_set > image_container_set_sptr;


/// A concrete image set that simply wraps a vector of images.
class simple_image_container_set :
  public image_container_set
{
public:
  /// Default Constructor
  simple_image_container_set() { }

  /// Constructor from a vector of images
  explicit simple_image_container_set( std::vector< image_container_sptr > const& images )
    : data_( images ) { }

  /// Return the number of items
  virtual size_t size() const { return data_.size(); }

  /// Return the underlying vector container
  virtual std::vector< image_container_sptr > images() const { return data_; }


protected:
  /// The vector of images
  std::vector< image_container_sptr > data_;
};

} } // end namespace vital

#endif // VITAL_IMAGE_CONTAINER_SET_H_
