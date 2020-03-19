/*ckwg +29
 * Copyright 2017, 2020 by Kitware, Inc.
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

#include <vital/types/covariance.h>

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

namespace py = pybind11;
namespace kv = kwiver::vital;

// Easy way to automate bindings of templated classes.
// For more information, see below link
// https://stackoverflow.com/questions/47487888/pybind11-template-class-of-many-types
template< unsigned N, typename T >
void declare_covariance( py::module &m, std::string const& typestr )
{
  using Class = kv::covariance_< N, T >;
  const std::string pyclass_name = std::string( "Covar" ) + typestr;

  py::class_< Class, std::shared_ptr< Class > >( m, pyclass_name.c_str() )
  .def( py::init<>() )
  .def( py::init< const T& >() )
  .def( py::init< const Eigen::Matrix< T, N, N >& >() )
  .def( "matrix", &Class::matrix )
  .def( "__setitem__", []( Class& self, py::tuple idx, T value )
                      {
                        // Casting values to unsigned removes compiler
                        // warning when comparing int and unsigned int.
                        // Converting right to unsigned in the .cast<>() call
                        // results in a misleading runtime error if negative
                        // values for i or j are used, thus the implicit cast
                        unsigned int i = idx[0].cast<int>();
                        unsigned int j = idx[1].cast<int>();
                        if( i >= N || j >= N )
                        {
                          throw py::index_error( "Index out of range!" );
                        }
                        self( i, j ) = value;
                      })
  .def( "__getitem__", []( Class& self, py::tuple idx )
                      {
                        unsigned int i = idx[0].cast<int>();
                        unsigned int j = idx[1].cast<int>();
                        if( i >= N || j >= N )
                        {
                          throw py::index_error( "Index out of range!" );
                        }
                        return self( i, j );
                      })
  ;
}


PYBIND11_MODULE(covariance, m)
{
  declare_covariance< 2, double >( m, "2d" );
  declare_covariance< 2, float  >( m, "2f" );
  declare_covariance< 3, double >( m, "3d" );
  declare_covariance< 3, float  >( m, "3f" );
  declare_covariance< 4, double >( m, "4d" );
  declare_covariance< 4, float  >( m, "4f" );
}
