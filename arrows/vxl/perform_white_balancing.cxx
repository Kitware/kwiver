/*ckwg +5
 * Copyright 2015-2019 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "perform_white_balancing.h"

#include <arrows/vxl/image_container.h>

#include <vil/vil_pixel_format.h>
#include <vil/vil_image_view.h>
#include <vil/vil_copy.h>
#include <vil/vil_resample_bilin.h>
#include <vil/algo/vil_gauss_reduce.h>

#include <limits>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <exception>

namespace kwiver {
namespace arrows {
namespace vxl {

namespace {


// Definition of the type (default) definitions of the value of white
template <class T> struct default_white_point
{
  static T value(){ return 1; }
};
template <> struct default_white_point<vxl_byte>
{
  static vxl_byte value(){ return 255; }
};
template <> struct default_white_point<vxl_uint_16>
{
  static vxl_uint_16 value(){ return 65535; }
};
template <> struct default_white_point<unsigned int>
{
  static unsigned int value(){ return 65535; }
};
template <> struct default_white_point<int>
{
  static int value(){ return 65535; };
};


// All of the possible settings used in the below classes
struct auto_white_balancer_settings
{
  auto_white_balancer_settings()
   : white_point_value_( 0 ),
     white_traverse_factor( 0.95 ),
     white_ref_weight( 1.0 ),
     black_traverse_factor( 0.75 ),
     black_ref_weight( 1.0 ),
     exp_averaging_factor( 0.25 ),
     correction_matrix_res( 8 ),
     pixels_to_sample( 10000 )
  {}

  // Approximate definition of pure white, a value of 0 indcates to use the type default
  double white_point_value_;

  // Shift the whitest thing in the image this percentage towards pure white
  double white_traverse_factor;
  double white_ref_weight;

  // Shift the blackest thing in the image this percentage towards pure black
  double black_traverse_factor;
  double black_ref_weight;

  // Exponential averaging coefficient for averaging past correction factors
  double exp_averaging_factor;

  // Resolution of matrix (per channel) when storing correctional shifts
  unsigned correction_matrix_res;

  // When estimating a correction matrix for some image, the desired pixel count
  unsigned pixels_to_sample;
};


// A simple helper struct for storing reference points (some color and a
// vector in the direction of what that color should ideally map to)
template <typename PixType, unsigned int Channels = 3>
class awb_reference_point
{
public:

  awb_reference_point( const PixType* pt, const PixType* ref, double weight )
  {
    for( unsigned int i = 0; i < Channels; i++ )
    {
      loc_[i] = pt[i];
      vec_[i] = static_cast<double>(ref[i])-static_cast<double>(loc_[i]);
    }
    weight_ = weight;
  }

  void scale_vector( const double& factor )
  {
    for( unsigned int i = 0; i < Channels; i++ )
    {
      vec_[i] = factor * vec_[i];
    }
  }

  PixType loc_[Channels];
  double vec_[Channels];
  double weight_;
};


// Given multiple reference points, compute a correction matrix by interpolating
// an estimated shift for each bin in the color space
template <typename PixType>
void compute_correction_matrix( std::vector< awb_reference_point< PixType > >& ref,
                                const unsigned& resolution_per_chan,
                                std::vector< double >& output,
                                const double& white_point );


// Simple base class to support operations on multiple type
class auto_white_balancer_base
{
public:

  auto_white_balancer_base() {}
  virtual ~auto_white_balancer_base() {}

  // Reset recorded history/averages (should be called near shot breaks)
  virtual void reset() = 0;

  // Set new options for the filter
  virtual void configure( auto_white_balancer_settings& options ) = 0;

};

// A process-like class which can compute some color space warping and
// apply it to some group of images. A correction matrix covering the entire
// RGB color space is created by identifying potential reference points and
// what colors they should ideally map to, and then by interpolating an approx
// shift (a 3D vector) for each bin in this correction matrix sampling the color
// space.
template <typename PixType>
class auto_white_balancer : public auto_white_balancer_base
{

public:

  typedef vil_image_view< PixType > input_type;
  typedef std::vector< double > matrix_type;

  // Constructor/Destructor
  auto_white_balancer();
  ~auto_white_balancer();

  // Reset recorded history/averages (should be called near shot breaks)
  void reset();

  // Set new options for the filter
  void configure( auto_white_balancer_settings& options );

  // Creates a correction matrix from the image, and applies it to it
  void apply( input_type& image );

  // Creates a correction matrix from some reference image, and applies
  // it to the first input image
  void apply( input_type& image, const input_type& reference );

private:

  // Stored variables
  auto_white_balancer_settings options_;
  matrix_type correction_matrix_;
  matrix_type temp_matrix_;
  input_type downsized_image_;
  input_type smoothed_image_;
  bool is_first_matrix_;

  // Apply a correction matrix to some image using the internal settings
  void apply_correction_matrix( input_type& image, matrix_type& matrix );

  // Helper function which calculates the neareset point (L1) distance to some
  // reference point. L1 distance is used for efficiency and because we may often
  // have a noticable skew of just a single principle color.
  void calculate_nearest_point_l1( const input_type& image,
                                   const PixType* reference,
                                   PixType* nearest,
                                   unsigned sample_rate = 1 );
};


// Calculate the distance from some point in 3d space to some reference point
template< typename PixType, unsigned int Channels >
inline double awb_point_to_ref_dist( double *pt, const awb_reference_point< PixType >& ref )
{
  double dist = 0.0;
  for( unsigned i = 0; i < Channels; i++ )
  {
    double temp = pt[i] - ref.loc_[i];
    dist = dist + temp * temp;
  }
  return std::sqrt( dist );
}

// Given multiple reference points, compute a correction matrix by interpolating
// an estimated shift for each bin in the color space
template <typename PixType>
void compute_correction_matrix( std::vector< awb_reference_point< PixType > >& ref,
                                const unsigned& resolution_per_chan,
                                std::vector< double >& output,
                                const double& white_point )
{
  const int c3step = 3;
  const int c2step = c3step * resolution_per_chan;
  const int c1step = c2step * resolution_per_chan;

  assert( output.size() == c1step * resolution_per_chan );

  double max_dist = std::sqrt( 3 * white_point * white_point );
  double min_contrib = 0.05 * white_point;

  const double bin_spacing = white_point / resolution_per_chan;
  const double bin_half_width = bin_spacing / 2.0;

  for( unsigned i = 0; i < resolution_per_chan; i++ )
  {
    for( unsigned j = 0; j < resolution_per_chan; j++ )
    {
      for( unsigned k = 0; k < resolution_per_chan; k++ )
      {
        double* ptr = &output[0] + c1step * i + c2step * j + c3step * k;

        // Calculate the center value of the bin in 3D space
        double bin_center[3];
        bin_center[0] = (static_cast<double>(i) * bin_spacing ) + bin_half_width;
        bin_center[1] = (static_cast<double>(j) * bin_spacing ) + bin_half_width;
        bin_center[2] = (static_cast<double>(k) * bin_spacing ) + bin_half_width;

        // Take weighted average shift across all references
        double weight = 0.0;
        ptr[0] = 0.0;
        ptr[1] = 0.0;
        ptr[2] = 0.0;

        for( unsigned r = 0; r < ref.size(); r++ )
        {
          // Distance between this point and the reference point
          double dist = awb_point_to_ref_dist<PixType,3>( bin_center, ref[r] );

          // Associated weight to use in weighted average
          double ref_contrib = (min_contrib + max_dist - dist) * ref[r].weight_;
          weight += ref_contrib;
          ptr[0] += ref_contrib * ref[r].vec_[0];
          ptr[1] += ref_contrib * ref[r].vec_[1];
          ptr[2] += ref_contrib * ref[r].vec_[2];
        }

        if( weight > 0.0 )
        {
          ptr[0] = ptr[0] / weight;
          ptr[1] = ptr[1] / weight;
          ptr[2] = ptr[2] / weight;
        }
        else
        {
          ptr[0] = 0.0;
          ptr[1] = 0.0;
          ptr[2] = 0.0;
        }
      }
    }
  }
}

// Definition of video awb averaging class
template <typename PixType>
auto_white_balancer<PixType>
::auto_white_balancer()
{
  this->configure( options_ );
}

template <typename PixType>
auto_white_balancer<PixType>
::~auto_white_balancer()
{
}

template <typename PixType>
void auto_white_balancer<PixType>
::reset()
{
  is_first_matrix_ = true;
}

template <typename PixType>
void auto_white_balancer<PixType>
::configure( auto_white_balancer_settings& options )
{
  // Reset recorded history
  this->reset();

  // Set internal options
  options_ = options;

  // Allocate histogram (a 3d vector for every bin) in the color space
  unsigned int bin_count = 3 * options.correction_matrix_res
                             * options.correction_matrix_res
                             * options.correction_matrix_res;

  temp_matrix_.resize( bin_count, 0.0 );
  correction_matrix_.resize( bin_count, 0.0 );
}

template <typename PixType>
void auto_white_balancer<PixType>
::apply( input_type& image )
{
  // Validate input
  if( !image || image.nplanes() != 3 ) return;

  // Create a downsized, smoothed image to estimate correction mat from
  double pixel_area = static_cast<double>( image.ni() * image.nj() );
  double resize_factor = std::sqrt( static_cast<double>(options_.pixels_to_sample) ) /
                         std::sqrt( pixel_area );

  if( resize_factor < 1.0 )
  {
    unsigned new_ni = static_cast<unsigned>(resize_factor * image.ni() + 0.5);
    unsigned new_nj = static_cast<unsigned>(resize_factor * image.nj() + 0.5);

    if( new_ni < 5 || new_nj < 5 ) return;

    vil_resample_bilin( image, downsized_image_, new_ni, new_nj );
    vil_gauss_reduce_121( downsized_image_, smoothed_image_ );
    this->apply( image, smoothed_image_ );
  }
  else
  {
    this->apply( image, image );
  }
}

template <typename PixType>
void auto_white_balancer<PixType>
::apply( input_type& image, const input_type& reference )
{
  // Validate images, restrict to 3 channel ones for now
  if( !image || image.nplanes() != 3 )
  {
    this->reset();
    return;
  }
  if( !reference || reference.nplanes() != 3 )
  {
    this->reset();
    return;
  }

  // Identify potential reference points
  PixType white_point;

  if( options_.white_point_value_ != 0 )
  {
    white_point = static_cast<PixType>(options_.white_point_value_);
  }
  else
  {
    white_point = default_white_point<PixType>::value();
  }

  PixType black[3] = { 0, 0, 0 };
  PixType white[3] = { white_point, white_point, white_point };
  PixType closest[3] = { 0, 0, 0 };

  std::vector< awb_reference_point< PixType > > ref_list;

  calculate_nearest_point_l1( reference,
                              black,
                              closest,
                              1 );

  awb_reference_point< PixType > black_ref( closest, black, options_.black_ref_weight );
  black_ref.scale_vector( options_.black_traverse_factor );

  calculate_nearest_point_l1( reference,
                              white,
                              closest,
                              1 );

  awb_reference_point< PixType > white_ref( closest, white, options_.white_ref_weight );
  white_ref.scale_vector( options_.white_traverse_factor );

  ref_list.push_back( black_ref );
  ref_list.push_back( white_ref );

  // Create correction matrix
  compute_correction_matrix( ref_list, options_.correction_matrix_res, temp_matrix_, white_point );

  // Average correction matrix and apply it if required
  if( options_.exp_averaging_factor == 1.0 )
  {
    this->apply_correction_matrix( image, temp_matrix_ );
  }
  else
  {
    if( is_first_matrix_ )
    {
      correction_matrix_ = temp_matrix_;
      is_first_matrix_ = false;
    }
    else
    {
      const double exp = options_.exp_averaging_factor;
      const double invexp = 1.0 - exp;

      for( unsigned i = 0; i < correction_matrix_.size(); i++ )
      {
        correction_matrix_[i] = exp * temp_matrix_[i] + invexp * correction_matrix_[i];
      }
    }
    this->apply_correction_matrix( image, correction_matrix_ );
  }
}

// Cast and threshold double variables to some pixel type range
//
// Note - later on we probably want to add a vxl function which both
// casts the variable between two types, and thresholds the upper and
// lower bounds for an entire image (however using this helper function
// is still more efficient if it gets inlined due to other things happening
// in the main loop over the image).
template< typename PixType >
inline void awb_clamp( const double& value,
                       const double& threshold,
                       PixType& output )
{
  if( value > threshold )
  {
    output = static_cast<PixType>(threshold);
  }
  else if( value < 0 )
  {
    output = 0;
  }
  else
  {
    output = static_cast<PixType>(value);
  }
}

// Generalized version
template <typename PixType>
void auto_white_balancer<PixType>
::apply_correction_matrix( input_type& image, std::vector< double >& matrix )
{
  // Perform validation
  if( !image || image.nplanes() != 3 )
  {
    return;
  }

  // Create scanning parameters
  double type_threshold = static_cast<double>(default_white_point<vxl_byte>::value());

  unsigned ni = image.ni();
  unsigned nj = image.nj();

  const std::ptrdiff_t istep = image.istep();
  const std::ptrdiff_t jstep = image.jstep();
  const std::ptrdiff_t pstep = image.planestep();
  const std::ptrdiff_t p2step = 2*pstep;

  const unsigned bins_per_chan = options_.correction_matrix_res;
  const unsigned c3step = 3;
  const unsigned c2step = c3step * bins_per_chan;
  const unsigned c1step = c2step * bins_per_chan;

  double* matrix_top_left = &matrix[0];

  std::ptrdiff_t step;

  PixType* row = image.top_left_ptr();
  for (unsigned j=0;j<nj;++j,row+=jstep)
  {
    PixType* pixel = row;
    for (unsigned i=0;i<ni;++i,pixel+=istep)
    {
      // Lookup adjustment value for color in histogram
      step = c1step*bins_per_chan*pixel[0];
      step += c2step*bins_per_chan*pixel[pstep];
      step += c3step*bins_per_chan*pixel[p2step];

      const double* adj_vector = matrix_top_left + step;

      awb_clamp(adj_vector[0]+pixel[0], type_threshold, pixel[0]);
      awb_clamp(adj_vector[1]+pixel[pstep], type_threshold, pixel[pstep]);
      awb_clamp(adj_vector[2]+pixel[p2step], type_threshold, pixel[p2step]) ;
    }
  }
}

// Specialized for standard case (unsigned char input)
template <>
inline void auto_white_balancer<vxl_byte>
::apply_correction_matrix( vil_image_view<vxl_byte>& image, std::vector< double >& matrix )
{
  // Perform validation
  if( !image || image.nplanes() != 3 )
  {
    return;
  }

  // Create scanning parameters
  unsigned ni = image.ni();
  unsigned nj = image.nj();

  const std::ptrdiff_t istep = image.istep();
  const std::ptrdiff_t jstep = image.jstep();
  const std::ptrdiff_t pstep = image.planestep();
  const std::ptrdiff_t p2step = 2*pstep;

  const unsigned bins_per_chan = options_.correction_matrix_res;
  const unsigned c3step = 3;
  const unsigned c2step = c3step * bins_per_chan;
  const unsigned c1step = c2step * bins_per_chan;

  double* matrix_top_left = &matrix[0];

  std::ptrdiff_t step;

  vxl_byte* row = image.top_left_ptr();
  for (unsigned j=0;j<nj;++j,row+=jstep)
  {
    vxl_byte* pixel = row;
    for (unsigned i=0;i<ni;++i,pixel+=istep)
    {
      // Lookup adjustment value for color in histogram
      step = c1step*((bins_per_chan*static_cast<unsigned>(pixel[0])) >> 8 );
      step += c2step*((bins_per_chan*static_cast<unsigned>(pixel[pstep])) >> 8 );
      step += c3step*((bins_per_chan*static_cast<unsigned>(pixel[p2step])) >> 8 );

      double* adj_vector = matrix_top_left + step;

      // Apply adjustment factor
      awb_clamp(adj_vector[0]+pixel[0], 255.0, pixel[0]);
      awb_clamp(adj_vector[1]+pixel[pstep], 255.0, pixel[pstep]);
      awb_clamp(adj_vector[2]+pixel[p2step], 255.0, pixel[p2step]);
    }
  }
}


// Calculate the nearest color in the image to some reference point
template <typename PixType>
void auto_white_balancer<PixType>
::calculate_nearest_point_l1( const vil_image_view< PixType >& src,
                              const PixType* reference,
                              PixType* nearest,
                              unsigned sample_rate )
{
  // Create scanning parameters
  unsigned ni = 1+(src.ni()-1)/sample_rate;
  unsigned nj = 1+(src.nj()-1)/sample_rate;
  unsigned np = src.nplanes();

  std::ptrdiff_t istep = sample_rate * src.istep();
  std::ptrdiff_t jstep = sample_rate * src.jstep();
  std::ptrdiff_t pstep = src.planestep();

  const PixType* best_pos = src.top_left_ptr();

  // Floating point or all other cases
  double min_dist = std::numeric_limits<double>::infinity();

  const PixType* row = src.top_left_ptr();
  for (unsigned j=0;j<nj;++j,row+=jstep)
  {
    const PixType* pixel = row;
    for (unsigned i=0;i<ni;++i,pixel+=istep)
    {
      double dist = 0;

      const PixType* channel = pixel;

      for (unsigned p=0;p<np;++p,channel+=pstep)
      {
        dist += std::fabs( static_cast<double>(*channel) -
                          static_cast<double>(reference[p]) );
      }
      if( dist < min_dist )
      {
        best_pos = pixel;
        min_dist = dist;
      }
    }
  }

  // Set output
  for( unsigned p=0; p<np; p++ )
  {
    nearest[p] = best_pos[p*pstep];
  }
}

// Specialization of the above for the default (8-bit) case
template <>
inline void auto_white_balancer<vxl_byte>
::calculate_nearest_point_l1( const vil_image_view< vxl_byte >& src,
                              const vxl_byte* reference,
                              vxl_byte* nearest,
                              unsigned sample_rate )
{
  // Create scanning parameters
  unsigned ni = 1+(src.ni()-1)/sample_rate;
  unsigned nj = 1+(src.nj()-1)/sample_rate;
  unsigned np = src.nplanes();

  std::ptrdiff_t istep = sample_rate * src.istep();
  std::ptrdiff_t jstep = sample_rate * src.jstep();
  std::ptrdiff_t pstep = src.planestep();
  std::ptrdiff_t p2step = pstep + pstep;

  const vxl_byte* best_pos = src.top_left_ptr();

  // Perform scan
  int min_dist = std::numeric_limits<int>::max();

  // Specialization for 3 channel image
  if( np == 3 )
  {
    int ref[3] = { static_cast<int>(reference[0]),
                   static_cast<int>(reference[1]),
                   static_cast<int>(reference[2]) };

    const vxl_byte* row = src.top_left_ptr();
    for (unsigned j=0;j<nj;++j,row+=jstep)
    {
      const vxl_byte* pixel = row;
      for (unsigned i=0;i<ni;++i,pixel+=istep)
      {
        int dist = std::abs( static_cast<int>(*pixel)-ref[0] );
        dist += std::abs( static_cast<int>(pixel[pstep])-ref[1] );
        dist += std::abs( static_cast<int>(pixel[p2step])-ref[2] );

        if( dist < min_dist )
        {
          best_pos = pixel;
          min_dist = dist;
        }
      }
    }
  }
  // Specialization for any other channeled image
  else
  {
    const vxl_byte* row = src.top_left_ptr();
    for (unsigned j=0;j<nj;++j,row+=jstep)
    {
      const vxl_byte* pixel = row;
      for (unsigned i=0;i<ni;++i,pixel+=istep)
      {
        int dist = 0;
        const vxl_byte* channel = pixel;
        for (unsigned p=0;p<np;++p,channel+=pstep)
        {
          dist += static_cast<int>(std::abs( static_cast<int>(*channel) -
                                   static_cast<int>(reference[p]) ) );
        }
        if( dist < min_dist )
        {
          best_pos = pixel;
          min_dist = dist;
        }
      }
    }
  }

  // Set output
  for( unsigned p=0; p<np; p++ )
  {
    nearest[p] = best_pos[p*pstep];
  }
}

} // end anonoymous namespace


// --------------------------------------------------------------------------------------
/// Private implementation class
class perform_white_balancing::priv
{
public:

  priv() : m_settings()
  {
  }

  ~priv()
  {
  }

  // Internal parameters/settings
  std::unique_ptr< auto_white_balancer_base > m_balancer;
  auto_white_balancer_settings m_settings;
};

// --------------------------------------------------------------------------------------
perform_white_balancing
::perform_white_balancing()
: d( new priv() )
{
  attach_logger( "arrows.vxl.perform_white_balancing" );
}

perform_white_balancing
::~perform_white_balancing()
{
}

vital::config_block_sptr
perform_white_balancing
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value( "white_scale_factor",
    d->m_settings.white_traverse_factor,
    "A measure of how much to over or under correct white "
    "reference points. A value near 1.0 will attempt to make "
    "the whitest thing in the image very close to pure white." );
  config->set_value( "black_scale_factor",
    d->m_settings.black_traverse_factor,
    "A measure of how much to over or under correct black "
    "reference points. A value near 1.0 will attempt to make "
    "the blackest thing in the image very close to pure black." );
  config->set_value( "exp_history_factor",
    d->m_settings.exp_averaging_factor,
    "The exponential averaging factor for correction matrices" );
  config->set_value( "matrix_resolution",
    d->m_settings.correction_matrix_res,
    "The resolution of the correction matrix" );

  return config;
}

void
perform_white_balancing
::set_configuration( vital::config_block_sptr in_config )
{
  // Starting with our generated vital::config_block to ensure that assumed values
  // are present. An alternative is to check for key presence before performing a
  // get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  // Settings for averaging
  d->m_settings.white_traverse_factor =
    config->get_value< double >( "white_scale_factor" );
  d->m_settings.black_traverse_factor =
    config->get_value< double >( "black_scale_factor" );
  d->m_settings.exp_averaging_factor  =
    config->get_value< double >( "exp_history_factor" );
  d->m_settings.correction_matrix_res =
    config->get_value< unsigned> ( "matrix_resolution"  );

  // Validate parameters
  if( d->m_settings.exp_averaging_factor > 1.0 )
  {
    throw std::runtime_error( "Invalid exponential averaging weight" );
  }
  if( d->m_settings.correction_matrix_res > 200 )
  {
    throw std::runtime_error( "Correction matrix resolution is too large!" );
  }
}

bool
perform_white_balancing
::check_configuration( vital::config_block_sptr config ) const
{
  return true;
}

// Perform stitch operation
kwiver::vital::image_container_sptr
perform_white_balancing
::filter( kwiver::vital::image_container_sptr image_data )
{
  // Perform Basic Validation
  if( !image_data )
  {
    return image_data;
  }

  // Get input image
  vil_image_view_base_sptr view =
    vxl::image_container::vital_to_vxl( image_data->get_image() );

  // Perform different actions based on input type
#define HANDLE_CASE(T)                                                          \
  case T:                                                                       \
    {                                                                           \
      typedef vil_pixel_format_type_of<T >::component_type pix_t;               \
      vil_image_view< pix_t > input = view;                                     \
                                                                                \
      auto_white_balancer< pix_t >* balancer =                                  \
        dynamic_cast< auto_white_balancer< pix_t >* >( d->m_balancer.get() );   \
                                                                                \
      if( !balancer )                                                           \
      {                                                                         \
        balancer = new auto_white_balancer< pix_t >();                          \
        balancer->configure( d->m_settings );                                   \
        d->m_balancer = std::unique_ptr< auto_white_balancer_base >( balancer );\
      }                                                                         \
                                                                                \
      vil_image_view< pix_t > output;                                           \
      vil_copy_deep( input, output );                                           \
      balancer->apply( output );                                                \
      return std::make_shared< vxl::image_container >( output );                \
    }                                                                           \
    break;                                                                      \

  switch( view->pixel_format() )
  {
    HANDLE_CASE(VIL_PIXEL_FORMAT_BYTE);
    HANDLE_CASE(VIL_PIXEL_FORMAT_UINT_16);
    HANDLE_CASE(VIL_PIXEL_FORMAT_FLOAT);
    HANDLE_CASE(VIL_PIXEL_FORMAT_DOUBLE);
#undef HANDLE_CASE

  default:
    throw std::runtime_error( "Unsupported type received" );
  }

  // Code not reached, prevent warning
  return kwiver::vital::image_container_sptr();
}

} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver
