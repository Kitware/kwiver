/*ckwg +29
 * Copyright 2019 by Kitware, Inc.
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
 * \brief This file contains the interface to a simple point.
 *
 * This point class pairs a eigen vector with a covariance matrix.
 */

#ifndef KWIVER_VITAL_POINT_H_
#define KWIVER_VITAL_POINT_H_

#include <vital/vital_config.h>
#include <vital/vital_export.h>
#include <vital/types/vector.h>
#include <vital/types/covariance.h>

namespace kwiver {
namespace vital {

template < unsigned N, typename T >
class VITAL_EXPORT point
{
public:
  typedef Eigen::Matrix< T, N, 1 > vector_type;

  point()
  {
    m_value = vector_type::Zero();
  }
  point(vector_type const& v) { m_value=v; }

  virtual ~point() = default;

  vector_type& value() { return m_value; }
  const vector_type& value() const { return m_value; }

  covariance_<N, float>& covariance() { return m_covariance; }
  const covariance_<N, float> covariance() const
  {
    return m_covariance;
  }

protected:

  vector_type m_value;
  covariance_<N,float> m_covariance;
};

// Define for common types.
typedef point< 2, int >    point_2i;
typedef point< 2, double > point_2d;
typedef point< 2, float >  point_2f;
typedef point< 3, double > point_3d;
typedef point< 3, float >  point_3f;
typedef point< 4, double > point_4d;
typedef point< 4, float >  point_4f;

typedef std::shared_ptr< point_2i > point_2i_sptr;
typedef std::shared_ptr< point_2d > point_2d_sptr;
typedef std::shared_ptr< point_2f > point_2f_sptr;
typedef std::shared_ptr< point_3d > point_3d_sptr;
typedef std::shared_ptr< point_3f > point_3f_sptr;
typedef std::shared_ptr< point_4d > point_4d_sptr;
typedef std::shared_ptr< point_4f > point_4f_sptr;

typedef std::shared_ptr< point_2i const > point_2i_cptr;
typedef std::shared_ptr< point_2d const > point_2d_cptr;
typedef std::shared_ptr< point_2f const > point_2f_cptr;
typedef std::shared_ptr< point_3d const > point_3d_cptr;
typedef std::shared_ptr< point_3f const > point_3f_cptr;
typedef std::shared_ptr< point_4d const > point_4d_cptr;
typedef std::shared_ptr< point_4f const > point_4f_cptr;

VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_2i const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_2d const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_2f const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_3d const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_3f const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_4d const& obj );
VITAL_EXPORT ::std::ostream& operator<< ( ::std::ostream& str, point_4f const& obj );


} } // end namespace

#endif /* KWIVER_VITAL_POINT_H_ */
