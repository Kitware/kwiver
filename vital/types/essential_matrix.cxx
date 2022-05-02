// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief core essential matrix template implementations

#include "essential_matrix.h"

#include <cmath>

#include <vital/exceptions/math.h>

#include <Eigen/SVD>

namespace kwiver {
namespace vital {

/// Compute the twisted pair rotation from the rotation and translation
rotation_d
essential_matrix
::twisted_rotation() const
{
  // The quaternion representation of a 180 degree rotation about
  // unit vector [X,Y,Z] is simply [X, Y, Z, 0]
  vector_3d t = this->translation();
  return rotation_d(vector_4d(t.x(), t.y(), t.z(), 0.0)) * this->rotation();
}

/// Construct from a provided matrix
template <typename T>
essential_matrix_<T>
::essential_matrix_( Eigen::Matrix<T,3,3> const &mat )
{
  const matrix_t W = (matrix_t() << T(0), T(-1), T(0),
                                    T(1), T( 0), T(0),
                                    T(0), T( 0), T(1)).finished();
  Eigen::JacobiSVD<matrix_t> svd(mat, Eigen::ComputeFullU |
                                      Eigen::ComputeFullV);
  const matrix_t& U = svd.matrixU();
  const matrix_t& V = svd.matrixV();
  trans_ = U.col(2);
  matrix_t R = U*W*V.transpose();
  if( R.determinant() < T(0) )
  {
    R *= T(-1);
  }
  rot_ = rotation_<T>(R);
}

/// Construct from a rotation and translation
template <typename T>
essential_matrix_<T>
::essential_matrix_( rotation_<T> const &rot,
                     vector_t const &trans )
  : rot_( rot ),
    trans_( trans.normalized() )
{
}

/// Conversion Copy constructor -- float specialization
template <>
template <>
essential_matrix_<float>
::essential_matrix_( essential_matrix_<float> const &other )
  : rot_( other.rot_ ),
    trans_( other.trans_ )
{
}

/// Conversion Copy constructor -- double specialization
template <>
template <>
essential_matrix_<double>
::essential_matrix_( essential_matrix_<double> const &other )
  : rot_( other.rot_ ),
    trans_( other.trans_ )
{
}

/// Construct from a generic essential_matrix
template <typename T>
essential_matrix_<T>
::essential_matrix_( essential_matrix const &base )
  : rot_( static_cast<rotation_<T> >(base.rotation()) ),
    trans_( base.translation().template cast<T>() )
{
}

/// Construct from a generic essential_matrix -- double specialization
template <>
essential_matrix_<double>
::essential_matrix_( essential_matrix const &base )
  : rot_( base.rotation() ),
    trans_( base.translation() )
{
}

/// Create a clone of outself as a shared pointer
template <typename T>
essential_matrix_sptr
essential_matrix_<T>
::clone() const
{
  return std::make_shared<essential_matrix_<T>>( *this );
}

/// Get a double-typed copy of the underlying matrix
template <typename T>
Eigen::Matrix<double,3,3>
essential_matrix_<T>
::matrix() const
{
  return this->compute_matrix().template cast<double>();
}

/// Specialization for matrices with native double type
template <>
Eigen::Matrix<double,3,3>
essential_matrix_<double>
::matrix() const
{
  return this->compute_matrix();
}

/// Return the one of two possible 3D rotations that can parameterize E
template <typename T>
rotation_d
essential_matrix_<T>
::rotation() const
{
  return static_cast<rotation_d>(this->rot_);
}

/// Return the second possible rotation that can parameterize E
template <typename T>
rotation_d
essential_matrix_<T>
::twisted_rotation() const
{
  return static_cast<rotation_d>(this->compute_twisted_rotation());
}

/// Return a unit translation vector (up to a sign) that parameterizes E
template <typename T>
vector_3d
essential_matrix_<T>
::translation() const
{
  return this->trans_.template cast<double>();
}

/// Get the underlying matrix
template <typename T>
typename essential_matrix_<T>::matrix_t
essential_matrix_<T>
::compute_matrix() const
{
  matrix_t t_cross;
  t_cross << T(0), -trans_[2], trans_[1],
             trans_[2], T(0), -trans_[0],
            -trans_[1], trans_[0], T(0);
  return t_cross * matrix_t(rot_.matrix());
}

/// Compute the twisted pair rotation from the rotation and translation
template <typename T>
rotation_<T>
essential_matrix_<T>
::compute_twisted_rotation() const
{
  typedef Eigen::Matrix<T,4,1> vector_4;
  // The quaternion representation of a 180 degree rotation about
  // unit vector [X,Y,Z] is simply [X, Y, Z, 0]
  const vector_t& t = trans_;
  return rotation_<T>(vector_4(t.x(), t.y(), t.z(), T(0))) * rot_;
}

/// Get a const reference to the underlying rotation
template <typename T>
rotation_<T> const&
essential_matrix_<T>
::get_rotation() const
{
  return rot_;
}

/// Get a const reference to the underlying translation
template <typename T>
typename essential_matrix_<T>::vector_t const&
essential_matrix_<T>
::get_translation() const
{
  return trans_;
}

// ----------------------------------------------------------------------------
// Other Functions
// ----------------------------------------------------------------------------

/// essential_matrix_<T> output stream operator
template <typename T>
std::ostream&
operator<<( std::ostream &s, essential_matrix_<T> const &e )
{
  s << e.compute_matrix();
  return s;
}

/// Output stream operator for \p essential_matrix instances
std::ostream&
operator<<( std::ostream &s, essential_matrix const &e )
{
  s << e.matrix();
  return s;
}

// ----------------------------------------------------------------------------
// Template class instantiation
// ----------------------------------------------------------------------------
/// \cond DoxygenSuppress
#define INSTANTIATE_ESSENTIAL_MATRIX(T)                                 \
  template class essential_matrix_<T>;                                  \
  template VITAL_EXPORT std::ostream& operator<<( std::ostream &,       \
                        essential_matrix_<T> const & )

INSTANTIATE_ESSENTIAL_MATRIX(float);
INSTANTIATE_ESSENTIAL_MATRIX(double);
#undef INSTANTIATE_ESSENTIAL_MATRIX
/// \endcond

} } // end vital namespace
