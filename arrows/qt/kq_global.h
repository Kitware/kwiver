/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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

#ifndef KWIVER_ARROWS_QT_KQ_GLOBAL_H_
#define KWIVER_ARROWS_QT_KQ_GLOBAL_H_

/// Implement d-function with aliased name of private class.
///
/// This is equivalent to #KQ_IMPLEMENT_D_FUNC, but allows the type name of the
/// private class to be specified with \p private_name, which is necessary if
/// the private class is not named <i>Base</i>Private.
#define KQ_IMPLEMENT_ALIASED_D_FUNC( public_name, private_name ) \
  inline private_name* public_name::d_func() \
  { return static_cast< private_name* >( qGetPtrHelper( this->d_ptr ) ); } \
  inline private_name const* public_name::d_func() const \
  { return static_cast< private_name const* >( qGetPtrHelper( this->d_ptr ) ); }

/// Implement d-function.
///
/// This implements the d-function (<code>d_func()</code>) of a class using
/// \ref KQ_D.
#define KQ_IMPLEMENT_D_FUNC( class_name ) \
  KQ_IMPLEMENT_ALIASED_D_FUNC( class_name, class_name##Private )

/// Declare accessor functions for private class.
///
/// This declares normal (\c const and non-\c const) accessor functions for the
/// private class of \p class_name. The private class is also declared as a
/// friend.
#define KQ_DECLARE_PRIVATE( class_name ) \
  inline class_name##Private* d_func(); \
  inline class_name##Private const* d_func() const; \
  friend class class_name##Private;

/// Define accessor functions for public class.
///
/// This declares and defines accessor functions for the public class
/// \p class_name of a corresponding private class. The public class is also
/// declared as a friend.
#define KQ_DECLARE_PUBLIC( class_name ) \
  inline class_name* q_func() \
  { return static_cast< class_name* >( q_ptr ); } \
  inline class_name const* q_func() const \
  { return static_cast< class_name* >( q_ptr ); } \
  friend class class_name;

/// Declare simple pointer to private class.
///
/// This declares a simple pointer (<code>Private* const</code>) to the private
/// class of \p class_name. The private class must be freed when the public
/// class is destroyed to avoid a memory leak.
///
/// In most cases, you should use #KQ_DECLARE_PRIVATE_RPTR instead, to reduce
/// the risk of errors due to the need to perform manual memory management.
#define KQ_DECLARE_PRIVATE_PTR( class_name ) \
  class_name##Private* const d_ptr;

/// Declare scoped pointer to private class.
///
/// This declares a scoped pointer
/// (<code>QScopedPointer\<Private\> const</code>) to the private class of
/// \p class_name. The header for QScopedPointer must be included, and
/// QScopedPointer will automatically free the private class when the public
/// class is destroyed.
///
/// The use of QScopedPointer does not change the semantics of the d-pointer
/// beyond the convenience of ensuring clean-up for you. Therefore, this is
/// preferred over #KQ_DECLARE_PRIVATE_PTR in most cases.
#define KQ_DECLARE_PRIVATE_RPTR( class_name ) \
  QScopedPointer< class_name##Private > const d_ptr;

/// Declare pointer to public class.
///
/// This declares a simple pointer (<code>Public* const</code>) to the public
/// class \p class_name of a private class.
#define KQ_DECLARE_PUBLIC_PTR( class_name ) \
  class_name* const q_ptr;

/// Get pointer to private class.
///
/// This declares a local variable \c d, which is a pointer to the private
/// class of the caller. The pointer itself is immutable (that is, cannot be
/// reassigned).
#define KQ_D() auto* const d = d_func()

/// Get pointer to public class.
///
/// This declares a local variable \c q, which is a pointer to the public class
/// of the caller. The pointer itself is immutable (that is, cannot be
/// reassigned).
#define KQ_Q() auto* const q = q_func()

#endif
