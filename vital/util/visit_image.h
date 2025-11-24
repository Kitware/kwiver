// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utility methods for performing pixelwise operations.

#ifndef KWIVER_VITAL_UTIL_VISIT_IMAGE_H_
#define KWIVER_VITAL_UTIL_VISIT_IMAGE_H_

#include <vital/types/image.h>

#include <type_traits>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Calls \p visitor with appropriate typing for \p img.
///
/// For example, if \p img is a `vital::image const` containing \c uint8_t,
/// this function will call
/// `visitor.operator()< uint8_t const >( /* args... */ )`.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts a single
///   \c typename template parameter and arguments of types \c Args.
/// \param img Image object determining which type to pass to \p visitor.
/// \param args Arguments to forward to \p visitor.
template < class Visitor, class Image, class... Args >
void visit_pixel_traits( Visitor&& visitor, Image&& img, Args&&... args );

// ----------------------------------------------------------------------------
/// Calls \p visitor with appropriate typing for \p img1 and \p img2.
///
/// For example, if \p img1 is a `vital::image const` containing \c uint8_t and
/// \p img2 is a `vital::image` containing \c float, this function will call
/// `visitor.operator()< uint8_t const, float >( /* args... */ )`.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts a single
///   \c typename template parameter and arguments of types \c Args.
/// \param img1
///   Image object determining which type to pass first to \p visitor.
/// \param img2
///   Image object determining which type to pass second to \p visitor.
/// \param args Arguments to forward to \p visitor.
template < class Visitor, class Image1, class Image2, class... Args >
void visit_pixel_traits2(
  Visitor&& visitor, Image1&& img1, Image2&& img2, Args&&... args );

// ----------------------------------------------------------------------------
/// Calls \p visitor on every scalar in \p img.
///
/// No particular order of execution is guaranteed.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts a single template
///   parameter \p T and an argument of type \c T&.
/// \param img Image to call \p visitor on.
/// \tparam T
///   If \c void, will call \p visitor with the datatype of \p img determined
///   at runtime. If a scalar type, \p img must contain that type.
/// \tparam Vistor Type of \p visitor.
/// \tparam Image Type of \p img - `vital::image` or `vital::image const`.
template < class T = void, class Visitor = void, class Image = void >
void visit_image_scalars( Visitor&& visitor, Image&& img );

// ----------------------------------------------------------------------------
/// Calls \p visitor on each pair of scalars in \p img1 and \p img2.
///
/// No particular order of execution is guaranteed. \p img1 and \p img2 must
/// be the same shape.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts two template
///   parameters \p T2 and \p T2 and arguments of type \c T1& and \c T2&.
/// \param img1 First image to call \p visitor on.
/// \param img2 Second image to call \p visitor on.
/// \tparam T1
///   If \c void, will call \p visitor with the datatype of \p img1 determined
///   at runtime. If a scalar type, \p img1 must contain that type.
/// \tparam T2
///   If \c void, will call \p visitor with the datatype of \p img2 determined
///   at runtime. If a scalar type, \p img2 must contain that type.
/// \tparam TypesMatch
///   If \c true and either of \p T1 and \p T2 are \c void, assumes \p img1 and
///   \p img2 contain the same type.
/// \tparam Vistor Type of \p visitor.
/// \tparam Image1 Type of \p img1 - `vital::image` or `vital::image const`.
/// \tparam Image2 Type of \p img2 - `vital::image` or `vital::image const`.
template <
  class T1 = void, class T2 = void, bool TypesMatch = true,
  class Visitor = void, class Image1 = void, class Image2 = void >
void visit_image_scalars2( Visitor&& visitor, Image1&& img1, Image2&& img2 );

// ----------------------------------------------------------------------------
/// Creates an uninitialized image \c img2 with the same shape as \p img1, then
/// calls \p visitor on each pair of scalars in \p img1 and \c img2.
///
/// No particular order of execution is guaranteed.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts two template
///   parameters \p T1 and \p T2 and arguments of type \c T1& and \c T2&.
/// \param img1 Image to copy and call \p visitor on.
/// \tparam T1
///   If \c void, will call \p visitor with the datatype of \p img1 determined
///   at runtime. If a scalar type, \p img1 must contain that type.
/// \tparam T2
///   If \c void, the created \c img2 will hve the same type as \p img1. If a
///   scalar type, \c img2 will be created with that type.
/// \tparam Vistor Type of \p visitor.
/// \tparam Image1 Type of \p img1 - `vital::image` or `vital::image const`.
template <
  class T1 = void, class T2 = void, class Visitor = void, class Image1 = void >
image visit_image_scalars2_create( Visitor&& visitor, Image1&& img1 );

// ----------------------------------------------------------------------------
/// Calls \p visitor on every pixel in \p img.
///
/// No particular order of execution is guaranteed.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts a single template
///   parameter \p T and a tuple of \p Depth `T&`s (i.e. if \p Depth is 2,
///   `std::tuple< T&, T& >`).
/// \param img Image to call \p visitor on.
/// \tparam Depth Number of channels in \p img1.
/// \tparam T
///   If \c void, will call \p visitor with the datatype of \p img determined
///   at runtime. If a scalar type, \p img must contain that type.
/// \tparam Vistor Type of \p visitor.
/// \tparam Image Type of \p img - `vital::image` or `vital::image const`.
template <
  size_t Depth, class T = void, class Visitor = void, class Image = void >
void visit_image_pixels( Visitor&& visitor, Image&& img );

// ----------------------------------------------------------------------------
/// Calls \p visitor on each pair of pixels in \p img1 and \p img2.
///
/// No particular order of execution is guaranteed. \p img1 and \p img2 must
/// be the same shape.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts two template
///   parameters \p T1 and \p T2, and for arguments a tuple of \p Depth1 `T1&`s
///   (i.e. if \p Depth1 is 2, `std::tuple< T1&, T1& >`) and a tuple of
///   \p Depth2 `T2&`s.
/// \param img1 First image to call \p visitor on.
/// \param img2 Second image to call \p visitor on.
/// \tparam Depth1 Number of channels in \p img1.
/// \tparam Depth2 Number of channels in \p img2.
/// \tparam T1
///   If \c void, will call \p visitor with the datatype of \p img1 determined
///   at runtime. If a scalar type, \p img1 must contain that type.
/// \tparam T2
///   If \c void, will call \p visitor with the datatype of \p img2 determined
///   at runtime. If a scalar type, \p img2 must contain that type.
/// \tparam TypesMatch
///   If \c true and either of \p T1 and \p T2 are \c void, assumes \p img1 and
///   \p img2 contain the same type.
/// \tparam Vistor Type of \p visitor.
/// \tparam Image1 Type of \p img1 - `vital::image` or `vital::image const`.
/// \tparam Image2 Type of \p img2 - `vital::image` or `vital::image const`.
template <
  size_t Depth1, size_t Depth2,
  class T1 = void, class T2 = void, bool TypesMatch = true,
  class Visitor = void, class Image1 = void, class Image2 = void >
void visit_image_pixels2( Visitor&& visitor, Image1&& img1, Image2&& img2 );

// ----------------------------------------------------------------------------
/// Creates an uninitialized image \c img2 with the same shape as \p img1, then
/// calls \p visitor on each pair of scalars in \p img1 and \c img2.
///
/// No particular order of execution is guaranteed.
///
/// \param visitor
///   Functor to call. Must have \c operator() which accepts two template
///   parameters \p T1 and \p T2, and for arguments a tuple of \p Depth1 `T1&`s
///   (i.e. if \p Depth1 is 2, `std::tuple< T1&, T1& >`) and a tuple of
///   \p Depth2 `T2&`s.
/// \param img1 Image to copy and call \p visitor on.
/// \tparam Depth1 Number of channels in \p img1.
/// \tparam Depth2 Number of channels in \p img2.
/// \tparam T1
///   If \c void, will call \p visitor with the datatype of \p img1 determined
///   at runtime. If a scalar type, \p img1 must contain that type.
/// \tparam T2
///   If \c void, the created \c img2 will hve the same type as \p img1. If a
///   scalar type, \c img2 will be created with that type.
/// \tparam TypesMatch
///   If \c true and either of \p T1 and \p T2 are \c void, assumes \p img1 and
///   \p img2 contain the same type.
/// \tparam Vistor Type of \p visitor.
/// \tparam Image1 Type of \p img1 - `vital::image` or `vital::image const`.
template <
  size_t Depth1, size_t Depth2,
  class T1 = void, class T2 = void, bool TypesMatch = true,
  class Visitor = void, class Image1 = void >
image visit_image_pixels2_create( Visitor&& visitor, Image1&& img1 );

} // namespace vital

} // namespace kwiver

#endif
