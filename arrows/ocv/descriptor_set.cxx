/*ckwg +29
 * Copyright 2013-2015 by Kitware, Inc.
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
 * \brief OCV descriptor_set implementation
 */

#include "descriptor_set.h"

#include <vital/exceptions.h>

/// This macro applies another macro to all of the types listed below.
#define APPLY_TO_TYPES(MACRO) \
    MACRO(kwiver::vital::byte); \
    MACRO(float); \
    MACRO(double)


namespace kwiver {
namespace arrows {
namespace ocv {

namespace
{

/// Convert OpenCV type number into a string
std::string cv_type_to_string(int number)
{
    // find type
    int type_int = number % 8;
    std::string type_str;

    switch (type_int)
    {
        case 0:
            type_str = "8U";
            break;
        case 1:
            type_str = "8S";
            break;
        case 2:
            type_str = "16U";
            break;
        case 3:
            type_str = "16S";
            break;
        case 4:
            type_str = "32S";
            break;
        case 5:
            type_str = "32F";
            break;
        case 6:
            type_str = "64F";
            break;
        default:
            break;
    }

    // find channel
    int channel = (number / 8) + 1;

    std::stringstream type;
    type << "CV_" << type_str << "C" << channel;

    return type.str();
}


/// Templated helper function to convert matrix row into a descriptor
template <typename T>
vital::descriptor_sptr
ocv_to_vital_descriptor(const cv::Mat& v)
{
  vital::descriptor_array_of<T>* d = NULL;
  switch(v.cols)
  {
  case 64:
    d = new vital::descriptor_fixed<T,64>();
    break;
  case 128:
    d = new vital::descriptor_fixed<T,128>();
    break;
  case 256:
    d = new vital::descriptor_fixed<T,256>();
    break;
  default:
    d = new vital::descriptor_dynamic<T>(v.cols);
  }
  std::copy(v.begin<T>(), v.end<T>(), d->raw_data());
  return vital::descriptor_sptr(d);
}


/**
 * Convert a row in a cv::Mat into a vital::descriptor_sptr
 *
 * See the other ocv_to_vital_descriptor function that takes a single row
 * matix.
 *
 * @param m ocv::descriptor_set matrix to pull a row out of.
 * @param row Row to convert from the matrix :ref: v.
 * @return New vital::descriptor_sptr composed of the specified row in the
 *         given matrix.
 * @throws std::out_of_range Row specified is out of the range of the given
 *                           matrix.
 */
vital::descriptor_sptr
ocv_to_vital_descriptor( const cv::Mat& m, size_t row )
{
  if( row >= static_cast<size_t>( m.rows ) )
  {
    std::stringstream ss;
    ss << row;
    throw std::out_of_range( ss.str() );
  }

  // Convert the specified row into a vital::descriptor_sptr based on the
  // current cv::Mat data type.
#define CONVERT_CASE( T ) \
    case cv::DataType<T>::type: \
      return ocv_to_vital_descriptor<T>( m.row(row) ); \
      break;

  switch( m.type() )
  {
    APPLY_TO_TYPES( CONVERT_CASE );

    default:
      throw vital::invalid_value("No case to handle OpenCV descriptors of "
                                 "type " + cv_type_to_string(m.type()));
  }
#undef CONVERT_CASE
}


/// Templated helper function to convert descriptors into a cv::Mat
template <typename T>
cv::Mat
vital_descriptors_to_ocv(const vital::descriptor_set& desc)
{
  const unsigned int num = static_cast<unsigned int>(desc.size());
  const unsigned int dim = static_cast<unsigned int>(desc.at(0)->size());
  cv::Mat_<T> mat(num,dim);
  for( unsigned int i=0; i<num; ++i )
  {
    const vital::descriptor_array_of<T>* d =
        dynamic_cast<const vital::descriptor_array_of<T>*>(desc.at(i).get());
    if( !d || d->size() != dim )
    {
      throw vital::invalid_value("mismatch type or size when converting "
                                 "descriptors to OpenCV");
    }
    cv::Mat_<T> row = mat.row(i);
    std::copy(d->raw_data(), d->raw_data() + dim, row.begin());
  }
  return mat;
}

} // end anonymous namespace


/// Return a vector of descriptor shared pointers
std::vector<vital::descriptor_sptr>
descriptor_set
::descriptors() const
{
  std::vector<vital::descriptor_sptr> desc;
  const unsigned num_desc = data_.rows;
  /// \cond DoxygenSuppress
#define CONVERT_CASE(T) \
  case cv::DataType<T>::type: \
  for( unsigned i=0; i<num_desc; ++i ) \
  { \
    desc.push_back(ocv_to_vital_descriptor<T>(data_.row(i))); \
  } \
  break

  switch(data_.type())
  {
  APPLY_TO_TYPES(CONVERT_CASE);
  default:
    throw vital::invalid_value("No case to handle OpenCV descriptors of type "
                               + cv_type_to_string(data_.type()));
  }
#undef CONVERT_CASE
  /// \endcond
  return desc;
}


/// Constructor from an OpenCV descriptor matrix
descriptor_set
::descriptor_set( cv::Mat const & descriptor_matrix )
  : data_( descriptor_matrix )
{}

/// Get the number of descriptors in the set
size_t
descriptor_set
::size() const
{
  return data_.rows;
}

/// Whether or not this set is empty.
bool
descriptor_set
::empty() const
{
  return size() == 0;
}

/// Return the descriptor at the specified index
vital::descriptor_sptr
descriptor_set
::at( size_t index )
{
  return ocv_to_vital_descriptor( data_, index );
}

/// Return the descriptor at the specified index (const)
vital::descriptor_sptr const
descriptor_set
::at( size_t index ) const
{
  return ocv_to_vital_descriptor( data_, index );
}

/// Next-descriptor generation funciton.
descriptor_set::iterator::next_value_func_t
descriptor_set
::get_iter_next_func()
{
  size_t row = 0;
  // Variable for copy into the lambda instance to hold the current row
  // descriptor reference.
  vital::descriptor_sptr d_sptr;
  return [row,d_sptr,this] () mutable ->iterator::reference {
    if( row >= size() )
    {
      throw vital::stop_iteration_exception();
    }
    d_sptr = ocv_to_vital_descriptor( data_, row++ );
    return d_sptr;
  };
}

/// Next-descriptor generation funciton. (const)
descriptor_set::const_iterator::next_value_func_t
descriptor_set
::get_const_iter_next_func() const
{
  size_t row = 0;
  // Variable for copy into the lambda instance to hold the current row
  // descriptor reference.
  vital::descriptor_sptr d_sptr;
  return [row,d_sptr,this] () mutable ->const_iterator::reference {
    if( row >= size() )
    {
      throw vital::stop_iteration_exception();
    }
    d_sptr = ocv_to_vital_descriptor( data_, row++ );
    return d_sptr;
  };
}

/// Convert any descriptor set to an OpenCV cv::Mat
cv::Mat
descriptors_to_ocv_matrix(const vital::descriptor_set& desc_set)
{
  // if the descriptor set already contains a cv::Mat representation
  // then return the existing matrix
  if( const ocv::descriptor_set* d =
          dynamic_cast<const ocv::descriptor_set*>(&desc_set) )
  {
    return d->ocv_desc_matrix();
  }
  if( desc_set.empty() || !desc_set.at(0) )
  {
    return cv::Mat();
  }
  /// \cond DoxygenSuppress
#define CONVERT_CASE(T) \
  if( dynamic_cast<const vital::descriptor_array_of<T>*>(desc_set.at(0).get()) ) \
  { \
    return vital_descriptors_to_ocv<T>(desc_set); \
  }
  APPLY_TO_TYPES(CONVERT_CASE);
#undef CONVERT_CASE
  /// \endcond
  return cv::Mat();
}

} // end namespace ocv
} // end namespace arrows
} // end namespace kwiver
