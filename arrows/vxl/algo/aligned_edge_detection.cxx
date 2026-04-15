// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "aligned_edge_detection.h"

#include <arrows/vxl/image_container.h>

#include <vil/algo/vil_gauss_filter.h>
#include <vil/algo/vil_sobel_3x3.h>
#include <vil/vil_convert.h>
#include <vil/vil_image_view.h>
#include <vil/vil_math.h>
#include <vil/vil_plane.h>

#include <limits>
#include <type_traits>

#include <cstdlib>

namespace kwiver {

namespace arrows {

namespace vxl {

// ----------------------------------------------------------------------------
/// Private implementation class
class aligned_edge_detection::priv
{
public:
  priv( aligned_edge_detection& parent ) : parent{ parent } {}

  // Calculate potential edges
  template < typename PixType >
  void
  calculate_aligned_edges(
    vil_image_view< PixType > const& input,
    vil_image_view< PixType >& output_i,
    vil_image_view< PixType >& output_j );
  // Perform NMS on the input gradient images in horizontal and vertical
  // directions only
  template < typename OutputType, typename InputType >
  void
  nonmax_suppression(
    vil_image_view< InputType > const& grad_i,
    vil_image_view< InputType > const& grad_j,
    vil_image_view< OutputType >& output_i,
    vil_image_view< OutputType >& output_j );
  // Compute axis-aligned edges and perform non-max suppression on them
  template < typename pix_t >
  vil_image_view< pix_t >
  filter( vil_image_view< pix_t > const& input_image );

  aligned_edge_detection& parent;

  float
  c_threshold() const { return parent.c_threshold; }
  bool
  c_produce_joint_output() const { return parent.c_produce_joint_output; }
  double
  c_smoothing_sigma() const { return parent.c_smoothing_sigma; }
  unsigned
  c_smoothing_half_step() const { return parent.c_smoothing_half_step; }
};

// ----------------------------------------------------------------------------
template < typename OutputType, typename InputType >
void
aligned_edge_detection::priv
::nonmax_suppression(
  vil_image_view< InputType > const& grad_i,
  vil_image_view< InputType > const& grad_j,
  vil_image_view< OutputType >& output_i,
  vil_image_view< OutputType >& output_j )
{
  if( grad_i.ni() != grad_j.ni() || grad_i.nj() != grad_j.nj() )
  {
    LOG_ERROR(
      parent.logger(),
      "Input gradient image dimensions must be equivalent" );
  }

  auto const ni = grad_i.ni();
  auto const nj = grad_i.nj();

  output_i.fill( 0 );
  output_j.fill( 0 );

  // Perform non-maximum suppression
  for( unsigned j = 1; j < nj - 1; ++j )
  {
    for( unsigned i = 1; i < ni - 1; ++i )
    {
      InputType const val_i = grad_i( i, j );
      InputType const val_j = grad_j( i, j );

      if( val_i > c_threshold() )
      {
        if( val_i >= grad_i( i - 1, j ) && val_i >= grad_i( i + 1, j ) )
        {
          output_i( i, j ) = static_cast< OutputType >( val_i );
        }
      }
      if( val_j > c_threshold() )
      {
        if( val_j >= grad_j( i, j - 1 ) && val_j >= grad_j( i, j + 1 ) )
        {
          output_j( i, j ) = static_cast< OutputType >( val_j );
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------
template < typename PixType >
void
aligned_edge_detection::priv
::calculate_aligned_edges(
  vil_image_view< PixType > const& input,
  vil_image_view< PixType >& output_i,
  vil_image_view< PixType >& output_j )
{
  auto const source_ni = input.ni();
  auto const source_nj = input.nj();

  vil_image_view< float > grad_i{ source_ni, source_nj };
  vil_image_view< float > grad_j{ source_ni, source_nj };

  // Calculate sobel approx in x/y directions
  vil_sobel_3x3( input, grad_i, grad_j );

  // Take the absolute value of gradients in place
  vil_transform( grad_i, std::abs< float > );
  vil_transform( grad_j, std::abs< float > );

  // Perform NMS in vertical/horizonal directions and threshold magnitude
  nonmax_suppression< PixType >( grad_i, grad_j, output_i, output_j );
}

// ----------------------------------------------------------------------------
template < typename pix_t >
vil_image_view< pix_t >
aligned_edge_detection::priv
::filter( vil_image_view< pix_t > const& input_image )
{
  auto const source_ni = input_image.ni();
  auto const source_nj = input_image.nj();

  vil_image_view< pix_t > aligned_edges;
  if( c_produce_joint_output() )
  {
    aligned_edges.set_size( source_ni, source_nj, 3 );
  }
  else
  {
    aligned_edges.set_size( source_ni, source_nj, 2 );
  }
  aligned_edges.fill( 0 );

  auto i_response = vil_plane( aligned_edges, 0 );
  auto j_response = vil_plane( aligned_edges, 1 );

  calculate_aligned_edges< pix_t >( input_image, i_response, j_response );

  // Perform extra op if enabled
  if( c_produce_joint_output() )
  {
    auto combined_response = vil_plane( aligned_edges, 2 );

    // Add vertical and horizontal edge planes together and smooth
    vil_math_image_sum(
      i_response,
      j_response,
      combined_response );

    auto half_step = c_smoothing_half_step();
    auto const min_dim = std::min( source_ni, source_nj );

    if( 2 * half_step + 1 >= min_dim )
    {
      half_step = ( min_dim - 1 ) / 2;
    }

    if( half_step != 0 )
    {
      vil_image_view< pix_t > smoothed_response;
      // Smooth the combined response
      vil_gauss_filter_2d(
        combined_response, smoothed_response, c_smoothing_sigma(), half_step );
      combined_response.deep_copy( smoothed_response );
    }
  }
  return aligned_edges;
}

// ----------------------------------------------------------------------------
void
aligned_edge_detection
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.aligned_edge_detection" );
}

// ----------------------------------------------------------------------------
bool
aligned_edge_detection
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
aligned_edge_detection
::filter( kwiver::vital::image_container_sptr source_image_ptr )
{
  if( !source_image_ptr )
  {
    LOG_ERROR( logger(), "Image pointer was not valid." );
    return nullptr;
  }

  // Get input image
  vil_image_view_base_sptr source_image =
    vxl::image_container::vital_to_vxl( source_image_ptr->get_image() );

  // Perform Basic Validation
  if( !source_image )
  {
    LOG_ERROR( logger(), "Image was not valid." );
    return nullptr;
  }

  // Perform Basic Validation
  if( source_image->nplanes() == 3 )
  {
    source_image = vil_convert_to_grey_using_average( source_image );
  }
  else if( source_image->nplanes() != 1 )
  {
    LOG_ERROR(
      logger(), "Input must have either 1 or 3 channels but has "
        << source_image->nplanes() );
    return nullptr;
  }

#define HANDLE_CASE( T )                                            \
  case T:                                                           \
    {                                                               \
      using ipix_t = vil_pixel_format_type_of< T >::component_type; \
      auto filtered = d->filter< ipix_t >( source_image );          \
      return std::make_shared< vxl::image_container >( filtered );  \
    }

  switch( source_image->pixel_format() )
  {
  HANDLE_CASE( VIL_PIXEL_FORMAT_BYTE );
  HANDLE_CASE( VIL_PIXEL_FORMAT_UINT_16 );
  HANDLE_CASE( VIL_PIXEL_FORMAT_FLOAT );
#undef HANDLE_CASE
    default:
      LOG_ERROR( logger(), "Invalid input format type received" );
      return nullptr;
  }

  LOG_ERROR( logger(), "Invalid output format type received" );
  return nullptr;
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
