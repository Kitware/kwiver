// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "convert_image.h"

#include <arrows/vxl/image_container.h>
#include <arrows/vxl/image_statistics.h>

#include <vil/vil_convert.h>
#include <vil/vil_image_view.h>
#include <vil/vil_math.h>
#include <vil/vil_plane.h>

#include <cstdlib>
#include <limits>
#include <random>
#include <type_traits>

namespace kwiver {

namespace arrows {

namespace vxl {

namespace {

// ----------------------------------------------------------------------------
// Convert a floating point image to an intergral type by multiplying
// it by a scaling factor in addition to thresholding it in one operation.
// Performs rounding.
template < typename OutType, typename InType >
vil_image_view< OutType >
scale_image(
  vil_image_view< InType > const& src,
  double const& dp_scale )
{
  auto const ni = src.ni();
  auto const nj = src.nj();
  auto const np = src.nplanes();
  vil_image_view< OutType > dst{ ni, nj, np };

  constexpr OutType max_output_value = std::numeric_limits< OutType >::max();

  auto const max_input_value = static_cast< InType >(
    static_cast< double >( max_output_value ) / dp_scale );

  vil_transform(
    src, dst,
    [=]( InType pixel ){
      if( pixel <= max_input_value )
      {
        return static_cast< OutType >(
          static_cast< double >( pixel ) * dp_scale + 0.5 );
      }
      else
      {
        return max_output_value;
      }
    } );
  return dst;
}

// ----------------------------------------------------------------------------
template < typename Type >
void
combine_channels(
  vil_image_view< Type > const& src,
  vil_image_view< Type >& dst )
{
  if( src.nplanes() == 3 )
  {
    vil_convert_planes_to_grey( src, dst );
  }
  else
  {
    vil_math_mean_over_planes( src, dst );
  }
}

// ----------------------------------------------------------------------------
template < typename InputType, typename OutputType >
void
percentile_scale_image(
  vil_image_view< InputType > const& src, vil_image_view< OutputType >& dst,
  double lower, double upper, unsigned sampling_points,
  bool ignore_extremes = true )
{
  std::vector< double > percentiles( 2, 0.0 );
  percentiles[ 0 ] = lower;
  percentiles[ 1 ] = upper;

  std::vector< InputType > percentile_values =
    get_image_percentiles(
      src, percentiles, sampling_points,
      ignore_extremes );

  OutputType max_val = std::numeric_limits< OutputType >::max();

  InputType lower_bound = percentile_values[ 0 ];
  InputType upper_bound = percentile_values[ 1 ];

  double scale;

  if( percentile_values[ 1 ] - percentile_values[ 0 ] > 0 )
  {
    scale = ( static_cast< double >( max_val ) + 0.5 ) /
            static_cast< double >( percentile_values[ 1 ] -
                                   percentile_values[ 0 ] );
  }
  else
  {
    scale = static_cast< double >( max_val ) /
            static_cast< double >( std::numeric_limits< InputType >::max() );
  }

  auto const ni = src.ni();
  auto const nj = src.nj();
  auto const np = src.nplanes();
  dst.set_size( ni, nj, np );

  // Stretch image to upper and lower percentile bounds
  vil_transform(
    src, dst,
    [ lower_bound, upper_bound, scale ]( InputType pixel ){
      if( pixel < lower_bound )
      {
        return static_cast< OutputType >( 0 );
      }
      else if( pixel > upper_bound )
      {
        return static_cast< OutputType >(
          std::numeric_limits< OutputType >::max() );
      }
      else
      {
        if constexpr( std::is_same< InputType, bool >::value )
        {
          // avoiding a compile warning here
          return static_cast< OutputType >(
            static_cast< double >( pixel - lower_bound ) && scale );
        }
        else
        {
          return static_cast< OutputType >(
            static_cast< double >( pixel - lower_bound ) * scale );
        }
      }
    } );
}

} // namespace <anonoymous>

// ----------------------------------------------------------------------------
/// Private implementation class
class convert_image::priv
{
public:
  priv( convert_image& parent ) : parent( parent )
  {}

  convert_image& parent;

  // Convert a fraction of images to gray
  template < typename Type >
  vil_image_view< Type >
  random_gray_conversion(
    vil_image_view< Type > const& src,
    double const random_fraction );

  // Apply appropriate transforms
  template < typename ipix_t >
  vil_image_view< ipix_t >
  apply_transforms( vil_image_view_base_sptr& view );

  // Scale and convert the image
  template < typename opix_t, typename ipix_t >
  vil_image_view< opix_t >
  scale_and_convert( vil_image_view< ipix_t > const& input );

  const std::string&
  c_format() const { return parent.c_format; }
  bool
  c_single_channel() const { return parent.c_single_channel; }
  double
  c_scale_factor() const { return parent.c_scale_factor; }
  double
  c_random_grayscale() const { return parent.c_random_grayscale; }
  double
  c_percentile_norm() const { return parent.c_percentile_norm; }

  std::random_device random_device;
  std::mt19937 random_engine{ random_device() };
  std::uniform_real_distribution< double > random_dist{ 0.0, 1.0 };
};

// ----------------------------------------------------------------------------
template < typename Type >
vil_image_view< Type >
convert_image::priv
::random_gray_conversion(
  vil_image_view< Type > const& src,
  double const random_fraction )
{
  if( random_dist( random_engine ) < random_fraction )
  {
    vil_image_view< Type > compressed;
    combine_channels( src, compressed );

    vil_image_view< Type > dst{ src.ni(), src.nj(), src.nplanes() };

    for( unsigned p = 0; p < src.nplanes(); ++p )
    {
      vil_image_view< Type > output_plane = vil_plane( dst, p );
      vil_copy_reformat( compressed, output_plane );
    }
    return dst;
  }
  else
  {
    return src;
  }
}

// ----------------------------------------------------------------------------
template < typename ipix_t >
vil_image_view< ipix_t >
convert_image::priv
::apply_transforms( vil_image_view_base_sptr& view )
{
  if( c_single_channel() && view->nplanes() != 1 )
  {
    vil_image_view< ipix_t > output;
    combine_channels(
      static_cast< vil_image_view< ipix_t > >( view ),
      output );
    return output;
  }
  else if( c_random_grayscale() > 0.0 )
  {
    return random_gray_conversion(
      static_cast< vil_image_view< ipix_t > >( view ),
      c_random_grayscale() );
  }
  else
  {
    return view;
  }
}

// ----------------------------------------------------------------------------
template < typename opix_t, typename ipix_t >
vil_image_view< opix_t >
convert_image::priv
::scale_and_convert( vil_image_view< ipix_t > const& input )
{
  vil_image_view< opix_t > output;
  if( c_percentile_norm() >= 0.0 )
  {
    percentile_scale_image(
      input, output, c_percentile_norm(), 1.0 - c_percentile_norm(), 1e8 );
  }
  else if( c_scale_factor() == 0.0 || c_scale_factor() == 1.0 )
  {
    vil_convert_cast( input, output );
  }
  else
  {
    output = scale_image< opix_t >( input, c_scale_factor() );
  }
  return output;
}

// ----------------------------------------------------------------------------
void
convert_image
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.convert_image" );
}

// ----------------------------------------------------------------------------
bool
convert_image
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
convert_image
::filter( kwiver::vital::image_container_sptr image_data )
{
  // Perform Basic Validation
  if( !image_data )
  {
    return nullptr;
  }

  // Get input image
  vil_image_view_base_sptr view =
    vxl::image_container::vital_to_vxl( image_data->get_image() );

  // Perform different actions based on input type
#define HANDLE_OUTPUT_CASE( S, T )                              \
if( d->c_format() == S )                                        \
{                                                               \
  using opix_t = vil_pixel_format_type_of< T >::component_type; \
  auto const& output = d->scale_and_convert< opix_t >( input ); \
  return std::make_shared< vxl::image_container >( output );    \
}

#define HANDLE_INPUT_CASE( T )                                        \
  case T:                                                             \
    {                                                                 \
      using ipix_t = vil_pixel_format_type_of< T >::component_type;   \
      if( d->c_format() == "disable" )                                \
      {                                                               \
        return image_data;                                            \
      }                                                               \
      auto const& input = d->apply_transforms< ipix_t >( view );      \
      if( d->c_format() == "copy" )                                   \
      {                                                               \
        auto const& output = d->scale_and_convert< ipix_t >( input ); \
        return std::make_shared< vxl::image_container >( output );    \
      }                                                               \
                                                                      \
      HANDLE_OUTPUT_CASE( "byte", VIL_PIXEL_FORMAT_BYTE );            \
      HANDLE_OUTPUT_CASE( "sbyte", VIL_PIXEL_FORMAT_SBYTE );          \
      HANDLE_OUTPUT_CASE( "uint16", VIL_PIXEL_FORMAT_UINT_16 );       \
      HANDLE_OUTPUT_CASE( "int16", VIL_PIXEL_FORMAT_INT_16 );         \
      HANDLE_OUTPUT_CASE( "uint32", VIL_PIXEL_FORMAT_UINT_32 );       \
      HANDLE_OUTPUT_CASE( "int32", VIL_PIXEL_FORMAT_INT_32 );         \
      HANDLE_OUTPUT_CASE( "uint64", VIL_PIXEL_FORMAT_UINT_64 );       \
      HANDLE_OUTPUT_CASE( "int64", VIL_PIXEL_FORMAT_INT_64 );         \
      HANDLE_OUTPUT_CASE( "float", VIL_PIXEL_FORMAT_FLOAT );          \
      HANDLE_OUTPUT_CASE( "double", VIL_PIXEL_FORMAT_DOUBLE );        \
      break;                                                          \
    }

  switch( view->pixel_format() )
  {
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_BOOL );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_BYTE );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_SBYTE );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_UINT_16 );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_INT_16 );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_UINT_32 );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_INT_32 );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_UINT_64 );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_INT_64 );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_FLOAT );
  HANDLE_INPUT_CASE( VIL_PIXEL_FORMAT_DOUBLE );

    default:
      LOG_ERROR( logger(), "Invalid input format type received" );
      return nullptr;
  }

#undef HANDLE_INPUT_CASE
#undef HANDLE_OUTPUT_CASE

  // If we get here, one of the input type handling branches was hit, but did
  // not produce a result; the output type must have been invalid
  LOG_ERROR( logger(), "Invalid output format type received" );
  return nullptr;
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
