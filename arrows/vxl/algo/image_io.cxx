// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief VXL image_io implementation

#include "image_io.h"

#include <vital/exceptions/image.h>
#include <vital/io/eigen_io.h>
#include <vital/types/metadata_traits.h>
#include <vital/vital_config.h>

#include <arrows/vxl/image_container.h>

#include <vil/vil_convert.h>
#include <vil/vil_load.h>
#include <vil/vil_plane.h>
#include <vil/vil_save.h>

#include <kwiversys/SystemTools.hxx>

#include <sstream>
#include <string>

using namespace kwiver::vital;
using ST = kwiversys::SystemTools;

namespace kwiver {

namespace arrows {

namespace vxl {

namespace {

// Helper function to convert images based on configuration
template < typename inP, typename outP >
void
convert_image_helper(
  const vil_image_view< inP >& src,
  vil_image_view< outP >& dest,
  VITAL_UNUSED bool force_byte, bool auto_stretch,
  bool manual_stretch, const array2 intensity_range )
{
  vil_image_view< double > temp;
  // The maximum value is extended by almost one such that dest_maxv still
  // truncates
  // to the outP maximimum value after casting.  The purpose for this is to more
  // evenly
  // distribute the values across the dynamic range.
  const double almost_one = 1 - 1e-6;
  double dest_minv =
    static_cast< double >( std::numeric_limits< outP >::min() );
  double dest_maxv =
    static_cast< double >( std::numeric_limits< outP >::max() ) + almost_one;
  if( !std::numeric_limits< outP >::is_integer )
  {
    dest_minv = outP( 0 );
    dest_maxv = outP( 1 );
  }
  if( auto_stretch )
  {
    vil_convert_stretch_range( src, temp, dest_minv, dest_maxv );
    vil_convert_cast( temp, dest );
  }
  else if( manual_stretch )
  {
    inP minv = static_cast< inP >( intensity_range[ 0 ] );
    inP maxv = static_cast< inP >( intensity_range[ 1 ] );
    vil_convert_stretch_range_limited(
      src, temp, minv, maxv, dest_minv,
      dest_maxv );
    vil_convert_cast( temp, dest );
  }
  else
  {
    vil_convert_cast( src, dest );
  }
}

// Helper function to convert images based on configuration - specialized for
// byte output
template < typename inP >
void
convert_image_helper(
  const vil_image_view< inP >& src,
  vil_image_view< vxl_byte >& dest,
  VITAL_UNUSED bool force_byte, bool auto_stretch,
  bool manual_stretch, const array2 intensity_range )
{
  if( auto_stretch )
  {
    vil_convert_stretch_range( src, dest );
  }
  else if( manual_stretch )
  {
    inP minv = static_cast< inP >( intensity_range[ 0 ] );
    inP maxv = static_cast< inP >( intensity_range[ 1 ] );
    vil_convert_stretch_range_limited( src, dest, minv, maxv );
  }
  else
  {
    vil_convert_cast( src, dest );
  }
}

// Helper function to convert images based on configuration - specialization for
// bool
template < typename outP >
void
convert_image_helper(
  const vil_image_view< bool >& src,
  vil_image_view< outP >& dest,
  VITAL_UNUSED bool force_byte, bool auto_stretch,
  bool manual_stretch,
  VITAL_UNUSED const array2 intensity_range )
{
  // special case for bool because manual stretching limits do not
  // make sense and trigger compiler warnings on some platforms.
  if( auto_stretch || manual_stretch )
  {
    vil_convert_stretch_range( src, dest );
  }
  else
  {
    vil_convert_cast( src, dest );
  }
}

// Helper function to convert images based on configuration - resolve
// specialization ambiguity
void
convert_image_helper(
  const vil_image_view< bool >& src,
  vil_image_view< vxl_byte >& dest,
  bool force_byte, bool auto_stretch,
  bool manual_stretch, const array2 intensity_range )
{
  convert_image_helper< vxl_byte >(
    src, dest, force_byte, auto_stretch,
    manual_stretch, intensity_range );
}

// ----------------------------------------------------------------------------
// Helper function to convert images based on configuration - specialization for
// bool/bool
void
convert_image_helper(
  const vil_image_view< bool >& src,
  vil_image_view< bool >& dest,
  VITAL_UNUSED bool force_byte,
  VITAL_UNUSED bool auto_stretch,
  VITAL_UNUSED bool manual_stretch,
  VITAL_UNUSED const array2 intensity_range )
{
  // special case for bool because stretch does not make sense for bool to bool
  // conversion
  dest = src;
}

// ----------------------------------------------------------------------------
// Construct a plane filename given the basename and plane index
std::string
plane_filename( std::string const& filename, unsigned p )
{
  auto const parent_directory =
    ST::GetParentDirectory( filename );
  auto const file_name_with_ext =
    ST::GetFilenameName( filename );

  auto const file_name_no_ext =
    ST::GetFilenameWithoutLastExtension( file_name_with_ext );
  auto const file_extension =
    ST::GetFilenameLastExtension( file_name_with_ext );

  std::vector< std::string > full_path;
  auto const plane_id = ( p > 0 ? "_" + std::to_string( p ) : "" );
  full_path.push_back( "" );
  full_path.push_back( parent_directory );
  full_path.push_back( file_name_no_ext + plane_id + file_extension );
  return ST::JoinPath( full_path );
}

// ----------------------------------------------------------------------------
// Save image as either single file or multiple plane files
template < typename inP >
void
save_image(
  vil_image_view< inP > const& src,
  std::string const& filename,
  bool split_planes = false )
{
  if( !split_planes || src.nplanes() == 1 )
  {
    vil_save( src, filename.c_str() );
  }
  else
  {
    for( decltype( src.nplanes() ) i{ 0 }; i < src.nplanes(); ++i )
    {
      vil_save( vil_plane( src, i ), plane_filename( filename, i ).c_str() );
    }
  }
}

// ----------------------------------------------------------------------------
// Create a list of filenames representing the non-initial plane files
std::vector< std::string >
construct_plane_filenames( std::string const& filename )
{
  std::vector< std::string > plane_files;

  for( unsigned p{ 1 };; ++p )
  {
    std::string plane_file = plane_filename( filename, p );

    if( ST::FileExists( plane_file ) )
    {
      plane_files.push_back( plane_file );
    }
    else
    {
      break;
    }
  }
  return plane_files;
}

// ----------------------------------------------------------------------------
// Helper function to load images when they are saved out in above format
template < typename Type >
vil_image_view< Type >
load_external_planes(
  std::string const& filename,
  vil_image_view< Type >& first_plane )
{
  std::vector< vil_image_view< Type > > images( 1, first_plane );

  size_t p = 1;
  auto total_p = first_plane.nplanes();

  auto const plane_filenames = construct_plane_filenames( filename );

  for( auto const& plane_file : plane_filenames )
  {
    vil_image_view< Type > plane = vil_load( plane_file.c_str() );

    if( plane.ni() != first_plane.ni() || plane.nj() != first_plane.nj() )
    {
      VITAL_THROW(
        vital::image_type_mismatch_exception,
        "Input channel size difference" );
    }

    images.push_back( plane );
    total_p += plane.nplanes();
    ++p;
  }

  vil_image_view< Type > output{ first_plane.ni(), first_plane.nj(), total_p };

  for( decltype( total_p ) img_id{ 0 }, out_pln{ 0 };
       out_pln < total_p; ++img_id )
  {
    for( unsigned img_pln{ 0 }; img_pln < images[ img_id ].nplanes();
         ++img_pln, ++out_pln )
    {
      vil_image_view< Type > src = vil_plane( images[ img_id ], img_pln );
      vil_image_view< Type > dst = vil_plane( output, out_pln );

      vil_copy_reformat( src, dst );
    }
  }

  return output;
}

} // namespace

// Private implementation class
class image_io::priv
{
public:
  // Constructor
  priv( image_io& parent )
    : parent( parent )
  {}

  image_io& parent;

  bool
  c_force_byte() const { return parent.c_force_byte; }
  bool
  c_auto_stretch() const { return parent.c_auto_stretch; }
  bool
  c_manual_stretch() const { return parent.c_manual_stretch; }
  array2
  c_intensity_range() const { return parent.c_intensity_range; }
  bool
  c_split_channels() const { return parent.c_split_channels; }

  template < typename inP, typename outP >
  void
  convert_image(
    const vil_image_view< inP >& src,
    vil_image_view< outP >& dest )
  {
    convert_image_helper(
      src, dest,
      c_force_byte(),
      c_auto_stretch(),
      c_manual_stretch(),
      c_intensity_range() );
  }

  // Load a single image, potentially saved as individual planes
  template < typename pix_t >
  image_container_sptr
  load_image(
    vil_image_view< pix_t >& img_pix_t,
    std::shared_ptr< metadata > md,
    std::string const& filename );
  // Convert an image to the appropriate type and write to disk
  template < typename pix_t >
  void
  convert_and_save(
    vil_image_view< pix_t >& img_pix_t,
    std::string const& filename );
};

// ----------------------------------------------------------------------------
template < typename pix_t >
image_container_sptr
image_io::priv
::load_image(
  vil_image_view< pix_t >& img_pix_t,
  std::shared_ptr< metadata > md,
  std::string const& filename )
{
  if( c_split_channels() )
  {
    img_pix_t = load_external_planes( filename, img_pix_t );
  }
  if( c_force_byte() )
  {
    vil_image_view< vxl_byte > img;
    convert_image( img_pix_t, img );

    auto img_ptr = image_container_sptr( new vxl::image_container( img ) );
    img_ptr->set_metadata( md );
    return img_ptr;
  }
  else
  {
    vil_image_view< pix_t > img;
    convert_image( img_pix_t, img );

    auto img_ptr = image_container_sptr( new vxl::image_container( img ) );
    img_ptr->set_metadata( md );
    return img_ptr;
  }
}

// ----------------------------------------------------------------------------
template < typename pix_t >
void
image_io::priv
::convert_and_save(
  vil_image_view< pix_t >& img_pix_t,
  std::string const& filename )
{
  if( c_force_byte() )
  {
    vil_image_view< vxl_byte > img;
    convert_image( img_pix_t, img );
    save_image( img, filename, c_split_channels() );
  }
  else
  {
    vil_image_view< pix_t > img;
    convert_image( img_pix_t, img );
    save_image( img, filename, c_split_channels() );
  }
}

void
image_io
::initialize()
{
  KWIVER_INITIALIZE_UNIQUE_PTR( priv, d_ );
  attach_logger( "arrows.vxl.image_io" );
}

// Destructor
image_io
::~image_io()
{}

// ----------------------------------------------------------------------------
// Check that the algorithm's currently configuration is valid
bool
image_io
::check_configuration( vital::config_block_sptr config ) const
{
  double auto_stretch = config->get_value< bool >(
    "auto_stretch",
    c_auto_stretch );
  double manual_stretch = config->get_value< bool >(
    "manual_stretch",
    c_manual_stretch );
  if( auto_stretch && manual_stretch )
  {
    LOG_ERROR( logger(), "can not enable both manual and auto stretching" );
    return false;
  }
  if( c_manual_stretch )
  {
    array2 range = config->get_value< array2 >(
      "intensity_range",
      c_intensity_range );
    if( range[ 0 ] >= range[ 1 ] )
    {
      LOG_ERROR(
        logger(), "stretching range minimum not less than maximum"
          << " (" << range[ 0 ] << ", " << range[ 1 ] << ")" );
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
// Load image image from the file
image_container_sptr
image_io
::load_( const std::string& filename ) const
{
  LOG_DEBUG( logger(), "Loading image from file: " << filename );

  auto md =
    std::shared_ptr< kwiver::vital::metadata >( new kwiver::vital::metadata() );
  md->add< kwiver::vital::VITAL_META_IMAGE_URI >( filename );

  vil_image_resource_sptr img_rsc = vil_load_image_resource( filename.c_str() );

#define DO_CASE( T )                                               \
  case T:                                                          \
    {                                                              \
      using pix_t = vil_pixel_format_type_of< T >::component_type; \
      vil_image_view< pix_t > img_pix_t = img_rsc->get_view();     \
      return d_->load_image( img_pix_t, md, filename );            \
    }                                                              \
    break;                                                         \

  switch( img_rsc->pixel_format() )
  {
  DO_CASE( VIL_PIXEL_FORMAT_BOOL );
  DO_CASE( VIL_PIXEL_FORMAT_BYTE );
  DO_CASE( VIL_PIXEL_FORMAT_SBYTE );
  DO_CASE( VIL_PIXEL_FORMAT_UINT_16 );
  DO_CASE( VIL_PIXEL_FORMAT_INT_16 );
  DO_CASE( VIL_PIXEL_FORMAT_UINT_32 );
  DO_CASE( VIL_PIXEL_FORMAT_INT_32 );
  DO_CASE( VIL_PIXEL_FORMAT_UINT_64 );
  DO_CASE( VIL_PIXEL_FORMAT_INT_64 );
  DO_CASE( VIL_PIXEL_FORMAT_FLOAT );
  DO_CASE( VIL_PIXEL_FORMAT_DOUBLE );

#undef DO_CASE

    default:
      if( c_auto_stretch )
      {
        // automatically stretch to fill the byte range using the
        // minimum and maximum pixel values
        vil_image_view< vxl_byte > img;
        img = vil_convert_stretch_range( vxl_byte(), img_rsc->get_view() );

        auto img_ptr = image_container_sptr( new vxl::image_container( img ) );
        img_ptr->set_metadata( md );
        return img_ptr;
      }
      else if( c_manual_stretch )
      {
        std::stringstream msg;
        msg << "Unable to manually stretch pixel type: "
            << img_rsc->pixel_format();
        VITAL_THROW( vital::image_type_mismatch_exception, msg.str() );
      }
      else
      {
        vil_image_view< vxl_byte > img;
        img = vil_convert_cast( vxl_byte(), img_rsc->get_view() );

        auto img_ptr = image_container_sptr( new vxl::image_container( img ) );
        img_ptr->set_metadata( md );
        return img_ptr;
      }
  }

  return image_container_sptr();
}

// ----------------------------------------------------------------------------
// Save image image to a file
void
image_io
::save_(
  const std::string& filename,
  image_container_sptr data ) const
{
  vil_image_view_base_sptr view =
    vxl::image_container::vital_to_vxl( data->get_image() );

#define DO_CASE( T )                                               \
  case T:                                                          \
    {                                                              \
      typedef vil_pixel_format_type_of< T >::component_type pix_t; \
      vil_image_view< pix_t > img_pix_t = view;                    \
      d_->convert_and_save( img_pix_t, filename );                 \
    }                                                              \
    break;                                                         \

  switch( view->pixel_format() )
  {
  DO_CASE( VIL_PIXEL_FORMAT_BOOL );
  DO_CASE( VIL_PIXEL_FORMAT_BYTE );
  DO_CASE( VIL_PIXEL_FORMAT_SBYTE );
  DO_CASE( VIL_PIXEL_FORMAT_UINT_16 );
  DO_CASE( VIL_PIXEL_FORMAT_INT_16 );
  DO_CASE( VIL_PIXEL_FORMAT_UINT_32 );
  DO_CASE( VIL_PIXEL_FORMAT_INT_32 );
  DO_CASE( VIL_PIXEL_FORMAT_UINT_64 );
  DO_CASE( VIL_PIXEL_FORMAT_INT_64 );
  DO_CASE( VIL_PIXEL_FORMAT_FLOAT );
  DO_CASE( VIL_PIXEL_FORMAT_DOUBLE );
#undef DO_CASE

    default:
      if( c_auto_stretch )
      {
        // automatically stretch to fill the byte range using the
        // minimum and maximum pixel values
        vil_image_view< vxl_byte > img;
        img = vil_convert_stretch_range( vxl_byte(), view );
        save_image( img, filename, c_split_channels );
        return;
      }
      else if( c_manual_stretch )
      {
        std::stringstream msg;
        msg <<  "Unable to manually stretch pixel type: "
            << view->pixel_format();
        VITAL_THROW( vital::image_type_mismatch_exception, msg.str() );
      }
      else
      {
        vil_image_view< vxl_byte > img;
        img = vil_convert_cast( vxl_byte(), view );
        save_image( img, filename, c_split_channels );
        return;
      }
  }
}

// ----------------------------------------------------------------------------
/// Load image metadata from the file
kwiver::vital::metadata_sptr
image_io
::load_metadata_( const std::string& filename ) const
{
  auto md = std::make_shared< kwiver::vital::metadata >();
  md->add< kwiver::vital::VITAL_META_IMAGE_URI >( filename );
  return md;
}

// ----------------------------------------------------------------------------
std::vector< std::string >
image_io
::plane_filenames( std::string const& filename ) const
{
  std::vector< std::string > output{ filename };
  auto additional_plane_filenames = construct_plane_filenames( filename );
  output.insert(
    output.end(),
    additional_plane_filenames.begin(),
    additional_plane_filenames.end() );
  return output;
}

} // end namespace vxl

} // end namespace arrows

} // end namespace kwiver
