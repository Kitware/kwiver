/*ckwg +29
 * Copyright 2013-2017 by Kitware, Inc.
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
 * \brief Typedefs for Eigen matrices
 */

#ifndef VITAL_MATRIX_H_
#define VITAL_MATRIX_H_

#include <Eigen/Core>

namespace kwiver {
namespace vital {

/// \cond DoxygenSuppress
typedef Eigen::MatrixXd matrix_d;
typedef Eigen::MatrixXf matrix_f;

typedef Eigen::Matrix< double, 2, 2 > matrix_2x2d;
typedef Eigen::Matrix< float, 2, 2 >  matrix_2x2f;
typedef Eigen::Matrix< double, 2, 3 > matrix_2x3d;
typedef Eigen::Matrix< float, 2, 3 >  matrix_2x3f;
typedef Eigen::Matrix< double, 3, 2 > matrix_3x2d;
typedef Eigen::Matrix< float, 3, 2 >  matrix_3x2f;
typedef Eigen::Matrix< double, 3, 3 > matrix_3x3d;
typedef Eigen::Matrix< float, 3, 3 >  matrix_3x3f;
typedef Eigen::Matrix< double, 3, 4 > matrix_3x4d;
typedef Eigen::Matrix< float, 3, 4 >  matrix_3x4f;
typedef Eigen::Matrix< double, 4, 3 > matrix_4x3d;
typedef Eigen::Matrix< float, 4, 3 >  matrix_4x3f;
typedef Eigen::Matrix< double, 4, 4 > matrix_4x4d;
typedef Eigen::Matrix< float, 4, 4 >  matrix_4x4f;
/// \endcond

} } // end namespace vital

#endif // VITAL_MATRIX_H_
