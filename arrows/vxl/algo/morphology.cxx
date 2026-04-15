// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "morphology.h"

#include <arrows/vxl/image_container.h>

#include <vital/range/iota.h>

#include <vil/algo/vil_binary_closing.h>
#include <vil/algo/vil_binary_dilate.h>
#include <vil/algo/vil_binary_erode.h>
#include <vil/algo/vil_binary_opening.h>
#include <vil/algo/vil_structuring_element.h>
#include <vil/vil_plane.h>
#include <vil/vil_transform.h>

namespace kwiver {

namespace arrows {

namespace vxl {

namespace {

// ----------------------------------------------------------------------------
inline bool
union_functor( bool x1, bool x2 )
{
  return x1 || x2;
}

// ----------------------------------------------------------------------------
inline bool
intersection_functor( bool x1, bool x2 )
{
  return x1 && x2;
}

} // namespace

// ----------------------------------------------------------------------------
class morphology::priv
{
public:
  using morphology_func_t = void ( * )(
    vil_image_view< bool > const&,
    vil_image_view< bool >&,
    vil_structuring_element const& );

  priv( morphology& parent ) : parent( parent ) {}

  morphology& parent;

  // Set up structuring elements
  void setup_internals();

  // Compute the morphological operation on an image
  void apply_morphology(
    vil_image_view< bool > const& input,
    vil_image_view< bool >& output );
  void apply_morphology(
    vil_image_view< bool > const& input,
    vil_image_view< bool >& output,
    morphology_func_t func );

  // Perform a morphological operation and optionally combine across channels
  vil_image_view< bool >
  perform_morphological_operations( vil_image_view< bool > const& input );

  bool configured{ false };
  vil_structuring_element morphological_element;

  const std::string&
  c_morphology() const { return parent.c_morphology; }
  const std::string&
  c_element_shape() const { return parent.c_element_shape; }
  const std::string&
  c_channel_combination() const { return parent.c_channel_combination; }
  double
  c_kernel_radius() const { return parent.c_kernel_radius; }
};

// ----------------------------------------------------------------------------
void
morphology::priv
::setup_internals()
{
  if( !configured )
  {
    switch( element_converter().from_string( c_element_shape() ) )
    {
      case ELEMENT_disk:
      {
        morphological_element.set_to_disk( c_kernel_radius() );
        break;
      }
      case ELEMENT_iline:
      {
        morphological_element.set_to_line_i(
          -static_cast< int >( c_kernel_radius() ),
          static_cast< int >( c_kernel_radius() ) );
        break;
      }
      case ELEMENT_jline:
      {
        morphological_element.set_to_line_j(
          -static_cast< int >( c_kernel_radius() ),
          static_cast< int >( c_kernel_radius() ) );
        break;
      }
    }

    configured = true;
  }
}

// ----------------------------------------------------------------------------
void
morphology::priv
::apply_morphology(
  vil_image_view< bool > const& input,
  vil_image_view< bool >& output,
  morphology_func_t func )
{
  for( auto plane_index : vital::range::iota( input.nplanes() ) )
  {
    auto input_plane = vil_plane( input, plane_index );
    auto output_plane = vil_plane( output, plane_index );
    func( input_plane, output_plane, morphological_element );
  }
}

// ----------------------------------------------------------------------------
void
morphology::priv
::apply_morphology(
  vil_image_view< bool > const& input,
  vil_image_view< bool >& output )
{
  switch( morphology_converter().from_string( c_morphology() ) )
  {
    case MORPHOLOGY_erode:
    {
      this->apply_morphology( input, output, vil_binary_erode );
      break;
    }
    case MORPHOLOGY_dilate:
    {
      this->apply_morphology( input, output, vil_binary_dilate );
      break;
    }
    case MORPHOLOGY_close:
    {
      this->apply_morphology( input, output, vil_binary_closing );
      break;
    }
    case MORPHOLOGY_open:
    {
      this->apply_morphology( input, output, vil_binary_opening );
      break;
    }
    case MORPHOLOGY_none:
    {
      output.deep_copy( input );
      break;
    }
  }
}

// ----------------------------------------------------------------------------
vil_image_view< bool >
morphology::priv
::perform_morphological_operations( vil_image_view< bool > const& input )
{
  setup_internals();

  vil_image_view< bool > output{ input.ni(), input.nj(), input.nplanes() };

  apply_morphology( input, output );

  int channel_combination =
    combine_converter().from_string( c_channel_combination() );

  if( channel_combination == COMBINE_none )
  {
    // Don't combine across channels
    return output;
  }

  // Select whether to do pixel-wise union or intersection
  auto functor =
    ( channel_combination == COMBINE_union
      ? union_functor : intersection_functor );

  auto accumulator = vil_plane( output, 0 );
  for( unsigned i = 1; i < output.nplanes(); ++i )
  {
    auto current_plane = vil_plane( output, i );
    // Union or intersect the current plane with the accumulator
    vil_transform( accumulator, current_plane, accumulator, functor );
  }
  return accumulator;
}

// ----------------------------------------------------------------------------
void
morphology
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d );
  attach_logger( "arrows.vxl.morphology" );
}

// ----------------------------------------------------------------------------
bool
morphology
::check_configuration( vital::config_block_sptr config ) const
{
  auto const kernel_radius = config->get_value< double >( "kernel_radius" );
  if( kernel_radius < 0 )
  {
    LOG_ERROR(
      logger(),
      "Config item kernel_radius should have been non-negative but was" <<
        kernel_radius );
  }
  return true;
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
morphology
::filter( kwiver::vital::image_container_sptr image_data )
{
  // Perform basic validation
  if( !image_data )
  {
    return nullptr;
  }

  // Get input image
  vil_image_view_base_sptr view =
    vxl::image_container::vital_to_vxl( image_data->get_image() );

  if( view->pixel_format() != VIL_PIXEL_FORMAT_BOOL )
  {
    LOG_ERROR( logger(), "Input format must be a bool" );
    return nullptr;
  }

  auto filtered =
    d->perform_morphological_operations(
      static_cast< vil_image_view< bool > >( view ) );

  return std::make_shared< vxl::image_container >( filtered );
}

} // namespace vxl

} // namespace arrows

} // namespace kwiver
