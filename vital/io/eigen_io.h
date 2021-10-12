/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
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
 * \brief Missing istream operator for Eigen fixed sized matrices
 */

#ifndef VITAL_EIGEN_IO_H_
#define VITAL_EIGEN_IO_H_


#include <iostream>
#include <cstring>

#include <Eigen/Core>

#include <vital/exceptions/io.h>


namespace Eigen {

/// input stream operator for an Eigen matrix
/**
 * \throws vital::invalid_data
 *    Throws an invalid data exception when the data being read is either not
 *    in the valid form or format, e.g. read a character where a double should
 *    be..
 *
 * \param s an input stream
 * \param m a matrix to stream into
 */
template < typename T, int M, int N >
std::istream&
operator>>( std::istream& s, Matrix< T, M, N >& m )
{
  for ( int i = 0; i < M; ++i )
  {
    for ( int j = 0; j < N; ++j )
    {
      if ( ! ( s >> std::skipws >> m( i, j ) ) )
      {
        VITAL_THROW( kwiver::vital::invalid_data, "Encountered a non-numeric value while "
                                    "parsing an Eigen::Matrix" );
      }
    }
  }
  return s;
}


/// Serialization of fixed Eigen matrices
template < typename Archive, typename T, int M, int N, int O, int MM, int NN >
void serialize(Archive & archive, Matrix< T, M, N, O, MM, NN >& m)
{
  for ( int i = 0; i < M; ++i )
  {
    for ( int j = 0; j < N; ++j )
    {
      archive( m( i, j ) );
    }
  }
}

} // end namespace Eigen

#endif // VITAL_EIGEN_IO_H_
