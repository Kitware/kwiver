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
 * \brief core feature interface
 */

#ifndef VITAL_FEATURE_H_
#define VITAL_FEATURE_H_

#include "color.h"
#include "covariance.h"
#include "vector.h"

#include <vital/vital_export.h>
#include <vital/vital_config.h>

#include <iostream>
#include <typeinfo>
#include <memory>

namespace kwiver {
namespace vital {

/// A representation of a 2D image feature point.
/**
 * The base class of features is abstract and provides a
 * double precision interface.  The templated derived class
 * can store values in either single or double precision.
 */
class feature
{
public:
  /// Destructor
  virtual ~feature() VITAL_DEFAULT_DTOR

  /// Access the type info of the underlying data (double or float)
  virtual std::type_info const& data_type() const = 0;

  /// Accessor for the image coordinates
  virtual vector_2d loc() const = 0;
  /// Accessor for the feature magnitude
  virtual double magnitude() const = 0;
  /// Accessor for the feature scale
  virtual double scale() const = 0;
  /// Accessor for the feature angle
  virtual double angle() const = 0;
  /// Accessor for the covariance
  virtual covariance_2d covar() const = 0;
  /// Accessor for the RGB color
  virtual rgb_color color() const = 0;
};

/// Shared pointer for base feature type
typedef std::shared_ptr< feature > feature_sptr;

/// output stream operator for base class feature
/**
 * \param s output stream
 * \param f feature to stream
 */
VITAL_EXPORT std::ostream& operator<<( std::ostream& s, feature const& f );


// ------------------------------------------------------------------
/// A concrete 2D image feature point.
/**
 * Templated over real number type (double or float).
 */
template < typename T >
class VITAL_EXPORT feature_ :
  public feature
{
public:
  /// Default Constructor
  feature_< T > ( );

  /// Constructor for a feature
  feature_< T > ( Eigen::Matrix< T, 2, 1 > const& loc, T mag = 0.0,
                  T scale = 1.0, T angle = 0.0,
                  rgb_color const& color = rgb_color());

  /// Constructor for a feature_ from a base class feature
  explicit feature_< T > ( feature const& f );

  /// Access statically available type of underlying data (double or float)
  static std::type_info const& static_data_type() { return typeid( T ); }

  /// Access the type info of the underlying data (double or float)
  virtual std::type_info const& data_type() const { return typeid( T ); }

  /// Accessor for the image coordinates using underlying data type
  Eigen::Matrix< T, 2, 1 > const& get_loc() const { return loc_; }

  /// Accessor for the image coordinates
  virtual vector_2d loc() const { return loc_.template cast< double > (); }

  /// Accessor for the feature magnitude using underlying data type
  T get_magnitude() const { return magnitude_; }

  /// Accessor for the feature magnitude
  virtual double magnitude() const { return static_cast< double > ( magnitude_ ); }

  /// Accessor for the feature scale using underlying data type
  T get_scale() const { return scale_; }

  /// Accessor for the feature scale
  virtual double scale() const { return static_cast< double > ( scale_ ); }

  /// Accessor for the feature angle using underlying data type
  T get_angle() const { return angle_; }

  /// Accessor for the feature angle
  virtual double angle() const { return static_cast< double > ( angle_ ); }

  /// Accessor for the covariance using underlying data type
  covariance_< 2, T > const& get_covar() const { return covar_; }

  /// Accessor for the covariance
  virtual covariance_2d covar() const { return static_cast< covariance_2d > ( covar_ ); }

  /// Accessor for a const reference to the RGB color
  virtual rgb_color const& get_color() const { return color_; }

  /// Accessor for the RGB color
  virtual rgb_color color() const { return color_; }


  /// Set the feature position in image space
  void set_loc( Eigen::Matrix< T, 2, 1 > const& loc ) { loc_ = loc; }

  /// Set the magnitude of the feature response
  void set_magnitude( T magnitude ) { magnitude_ = magnitude; }

  /// Set the scale of the feature
  void set_scale( T scale ) { scale_ = scale; }

  /// Set the angle of the feature
  void set_angle( T angle ) { angle_ = angle; }

  /// Set the covariance matrix of the feature
  void set_covar( covariance_< 2, T > const& covar ) { covar_ = covar; }

  // Set the RGB color of the landmark
  void set_color( rgb_color const& color ) { color_ = color; }


protected:
  /// location of feature
  Eigen::Matrix< T, 2, 1 > loc_;
  /// magnitude of feature
  T magnitude_;
  /// scale of feature
  T scale_;
  /// angle of feature
  T angle_;
  /// covariance matrix of feature
  covariance_< 2, T > covar_;
  /// RGB color of feature
  rgb_color color_;
};


// ------------------------------------------------------------------
/// Double-precision feature_ type
typedef feature_< double > feature_d;
/// Single-precision feature_ type
typedef feature_< float > feature_f;

/// output stream operator for a feature
template < typename T >
VITAL_EXPORT std::ostream& operator<<( std::ostream& s, feature_< T > const& f );

/// input stream operator for a feature
template < typename T >
VITAL_EXPORT std::istream& operator>>( std::istream& s, feature_< T >& f );

} } // end namespace vital

#endif // VITAL_FEATURE_H_
