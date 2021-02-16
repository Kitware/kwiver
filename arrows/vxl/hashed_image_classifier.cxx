// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "hashed_image_classifier.h"

#include <vil/vil_plane.h>

#include <boost/lexical_cast.hpp>

#include <cmath>

namespace vidtk {

// ----------------------------------------------------------------------------
// Classify a chain of hashed feature images
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::classify_images( input_image_t const& input_features,
                   weight_image_t& output_image,
                   weight_t const offset ) const
{
  feature_vector_t input_features_vector;
  auto num_planes = input_features.nplanes();
  for( unsigned i{ 0 }; i < num_planes; ++i )
  {
    input_features_vector.push_back( vil_plane( input_features, i ) );
  }
  classify_images( &input_features_vector[ 0 ],
                   input_features_vector.size(),
                   output_image,
                   offset );
}

// ----------------------------------------------------------------------------
// Classify a chain of hashed feature images
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::classify_images( feature_vector_t const& input_features,
                   weight_image_t& output_image,
                   weight_t const offset ) const
{
  classify_images( &input_features[ 0 ],
                   input_features.size(),
                   output_image,
                   offset );
}

// ----------------------------------------------------------------------------
// Classify a chain of hashed feature images
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::classify_images( input_image_t const* input_features,
                   unsigned const features,
                   weight_image_t& output_image,
                   weight_t const offset ) const
{
  if( !model_->is_valid() )
  {
    std::cout << "Internal classifier invalid" << std::endl;
  }

  if( features != feature_count() )
  {
    std::cout   << "Feature counts don't match, features: "
                << features << ", feature_count(): " << feature_count()
                << std::endl;
  }

  output_image.set_size( input_features[ 0 ].ni(), input_features[ 0 ].nj() );
  output_image.fill( offset );

  weight_t** const feature_weights = &( model_->feature_weights[ 0 ] );

  std::vector< std::ptrdiff_t > sisteps( features );
  std::vector< input_t const* > spixels( features );

  for( unsigned f = 0; f < features; ++f )
  {
    sisteps[ f ] = input_features[ f ].istep();
  }

  auto const distep = output_image.istep();

  for( unsigned j = 0; j < output_image.nj(); ++j )
  {
    weight_t* dpixel = &output_image( 0, j );

    for( unsigned f = 0; f < features; ++f )
    {
      spixels[ f ] = &input_features[ f ]( 0, j );
    }

    for( unsigned i = 0; i < output_image.ni(); ++i, dpixel += distep )
    {
      for( unsigned f = 0; f < features; ++f )
      {
        *dpixel += feature_weights[ f ][ *spixels[ f ] ];
        spixels[ f ] += sisteps[ f ];
      }
    }
  }
}

// ----------------------------------------------------------------------------
// Classify some chain of hashed input images, but only on specific pixels
// as decided by the given mask
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::classify_images( feature_vector_t const& input_features,
                   mask_image_t const& mask,
                   weight_image_t& output_image,
                   weight_t const offset ) const
{
  classify_images( &input_features[ 0 ],
                   input_features.size(),
                   mask,
                   output_image,
                   offset );
}

// ----------------------------------------------------------------------------
// Classify some chain of hashed input images, but only on specific pixels
// as decided by the given mask
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::classify_images( input_image_t const* input_features,
                   unsigned const features,
                   mask_image_t const& mask,
                   weight_image_t& output_image,
                   weight_t const offset ) const
{
  if( !model_->is_valid() )
  {
    std::cerr << "Internal classifier is invalid" << std::endl;
  }
  if( features != model_->num_features )
  {
    std::cerr << "Feature counts don't match" << std::endl;
  }

  output_image.set_size( input_features[ 0 ].ni(), input_features[ 0 ].nj() );

  weight_t** const feature_weights = &model_->feature_weights[ 0 ];

  for( unsigned j = 0; j < output_image.nj(); ++j )
  {
    for( unsigned i = 0; i < output_image.ni(); ++i )

    {
      if( mask( i, j ) )
      {
        weight_t& output = output_image( i, j );

        output = offset;

        for( unsigned f = 0; f < features; ++f )
        {
          output += feature_weights[ f ][ input_features[ f ]( i, j ) ];
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------
// Load a model from file
template < typename FeatureType, typename OutputType >
bool
hashed_image_classifier< FeatureType, OutputType >
::load_from_file( std::string const& file )
{
  model_.reset( new model_t() );

  // Open model file for reading
  std::ifstream input( file.c_str() );

  if( !input.is_open() )
  {
    std::cout << "Unable to open input file: " << file << std::endl;
    return false;
  }

  // Create a new vector to initially read our loaded model into
  std::vector< std::vector< weight_t > > weights;

  // Read each feature entry
  unsigned entry = 0;
  bool size_read_ = false;

  std::string line;
  while( std::getline( input, line ) )
  {
    // Parse line
    std::vector< std::string > parsed;
    std::istringstream iss( line );

    while( iss )
    {
      std::string word;
      iss >> word;
      parsed.push_back( word );
    }

    // Make sure last entry is not an empty char
    if( parsed.size() > 0 )
    {
      std::string last = parsed[ parsed.size() - 1 ];
      if( last == " " || last == "" || last == "\n" )
      {
        parsed.pop_back();
      }
    }

    // Skip empty lines
    if( parsed.size() == 0 )
    {
      continue;
    }

    // Skip comments indicated by pound symbol
    if( parsed[ 0 ].size() > 0 && parsed[ 0 ][ 0 ] == '#' )
    {
      continue;
    }

    // Read feature count first
    if( !size_read_ )
    {
      // Read number of features
      model_->num_features =
        boost::lexical_cast< unsigned int >( parsed[ 0 ] );

      // Make sure the model file has at least 1 input feature
      if( model_->num_features == 0 )
      {
        std::cout << "Number of input features to use must be > 1" <<
          std::endl;
        return false;
      }

      weights.resize( model_->num_features, std::vector< weight_t >() );

      size_read_ = true;
      continue;
    }

    // Validate row size
    if( parsed.size() < 2 )
    {
      continue;
    }

    // Read number of possible values for this feature
    unsigned int num_values =
      boost::lexical_cast< unsigned int >( parsed[ 0 ] );
    weights[ entry ].resize( num_values, 0.0 );

    // Data corruption, model file ill formatted
    if( parsed.size() != num_values + 1  || entry >= model_->num_features )
    {
      std::cout << "Number of weights (" << parsed.size() - 1
                << ") does not match " << num_values;
      return false;
    }

    // Copy parsed weight values
    for( unsigned i = 1; i < num_values + 1; ++i )
    {
      weights[ entry ][ i -
                        1 ] = boost::lexical_cast< weight_t >( parsed[ i ] );
    }
    ++entry;
  }

  // Another check to make sure that the reported file correctly contained
  // the specified number of features specified
  if( weights.size() != model_->num_features )
  {
    std::cout << "Weight vector size does not match " << model_->num_features;
    return false;
  }

  // Copy values from loaded vector into our internal storage. Why do we
  // not just use the vector from the previous operation as our internal
  // data storage device you ask? Because (a) this loaded data will exist
  // for the duration of the program and we don't want any additional memory
  // overhead and (b) so that the entire model is stored as a contigious
  // buffer in memory to reduce the chance of a cache miss.
  model_->max_feature_value.resize( model_->num_features );
  model_->feature_weights.resize( model_->num_features );

  unsigned int total_weight_bins = 0;

  for( unsigned i = 0; i < weights.size(); ++i )
  {
    model_->max_feature_value[ i ] = weights[ i ].size();
    total_weight_bins += model_->max_feature_value[ i ];
  }

  model_->weights.resize( total_weight_bins );

  unsigned int position = 0;

  for( unsigned i = 0; i < weights.size(); ++i )
  {
    model_->feature_weights[ i ] = &model_->weights[ 0 ] + position;

    for( unsigned j = 0; j < weights[ i ].size(); ++j )
    {
      model_->weights[ position++ ] = weights[ i ][ j ];
    }
  }

  std::cout << "model->num_features " << model_->num_features << std::endl;
  return true;
}

// ----------------------------------------------------------------------------
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::generate_weight_image( input_image_t const& src,
                         weight_image_t& dst,
                         unsigned const& feature_id ) const
{
  dst.set_size( src.ni(), src.nj() );
  dst.fill( 0 );

  weight_t** const feature_weights = &model_->feature_weights[ 0 ];

  for( unsigned j = 0; j < src.nj(); ++j )
  {
    for( unsigned i = 0; i < src.ni(); ++i )
    {
      dst( i, j ) = feature_weights[ feature_id ][ src( i, j ) ];
    }
  }
}

// ----------------------------------------------------------------------------
template < typename FeatureType, typename OutputType >
void
hashed_image_classifier< FeatureType, OutputType >
::set_model( model_sptr_t external_model )
{
  if( !external_model->is_valid() )
  {
    std::cout << "Input model invalid" << std::endl;
  }

  model_ = external_model;
}

// ----------------------------------------------------------------------------
template < typename FloatType >
hashed_image_classifier_model< FloatType >
::hashed_image_classifier_model( self_t const& other )
{
  *this = other;
}

// ----------------------------------------------------------------------------
template < typename FloatType >
bool
hashed_image_classifier_model< FloatType >
::is_valid() const
{
  if( num_features > 0 &&
      num_features == max_feature_value.size() &&
      num_features == feature_weights.size() &&
      weights.size() > num_features )
  {
    return true;
  }
  std::cout << "num_features: " << num_features << std::endl;

  return false;
}

// ----------------------------------------------------------------------------
template < typename FloatType >
void
hashed_image_classifier_model< FloatType >
::reset( unsigned feature_count, unsigned entries_per_feature )
{
  this->num_features = feature_count;

  this->max_feature_value =
    std::vector< unsigned >( feature_count, entries_per_feature );

  this->weights =
    std::vector< weight_t >( feature_count * entries_per_feature, 0.0 );

  this->feature_weights =
    std::vector< weight_t* >( feature_count );

  for( unsigned i = 0; i < feature_count; ++i )
  {
    this->feature_weights[ i ] = &( this->weights[ 0 ] ) + i *
                                 entries_per_feature;
  }
}

// ----------------------------------------------------------------------------
template < typename FloatType >
void
hashed_image_classifier_model< FloatType >
::normalize( weight_t total_weight )
{
  weight_t norm_factor = 0;

  for( unsigned i = 0; i < this->weights.size(); ++i )
  {
    norm_factor += std::fabs( this->weights[ i ] );
  }

  if( norm_factor != 0 )
  {
    norm_factor = 1.0 / ( norm_factor * total_weight );

    for( unsigned i = 0; i < this->weights.size(); ++i )
    {
      this->weights[ i ] *= norm_factor;
    }
  }
}

// ----------------------------------------------------------------------------
template < typename FloatType >
hashed_image_classifier_model< FloatType >&
hashed_image_classifier_model< FloatType >
::operator=( self_t const& other )
{
  num_features = other.num_features;
  max_feature_value = other.max_feature_value;
  weights = other.weights;

  if( other.feature_weights.size() > 0 )
  {
    feature_weights.resize( other.feature_weights.size() );

    weight_t const* const other_start = &( other.weights[ 0 ] );
    weight_t* this_start = &weights[ 0 ];

    for( unsigned i = 0; i < feature_weights.size(); ++i )
    {
      std::ptrdiff_t const offset = other.feature_weights[ i ] - other_start;
      feature_weights[ i ] = this_start + offset;
    }
  }
  else
  {
    feature_weights.clear();
  }

  return *this;
}

// ----------------------------------------------------------------------------
template < typename FloatType >
std::ostream&
operator<<( std::ostream& os,
            hashed_image_classifier_model< FloatType > const& obj )
{
  if( !obj.is_valid() )
  {
    os << "[Invalid Model]" << std::endl;
    return os;
  }

  os << obj.num_features << std::endl;
  for( unsigned i = 0; i < obj.num_features; ++i )
  {
    os << obj.max_feature_value[ i ];
    for( unsigned j = 0; j < obj.max_feature_value[ i ]; ++j )
    {
      os << " " << obj.feature_weights[ i ][ j ];
    }
    os << std::endl;
  }

  return os;
}

// ----------------------------------------------------------------------------
template < typename FeatureType, typename OutputType >
std::ostream&
operator<<( std::ostream& os,
            hashed_image_classifier< FeatureType, OutputType > const& obj )
{
  if( obj.model_ )
  {
    os << *( obj.model_ );
  }
  else
  {
    os << "[Empty Model]" << std::endl;
  }
  return os;
}

// Instantiate template classes
template class vidtk::hashed_image_classifier_model< double >;
template class vidtk::hashed_image_classifier_model< float >;

template class vidtk::hashed_image_classifier< vxl_byte, double >;
template class vidtk::hashed_image_classifier< vxl_byte, float >;

template class vidtk::hashed_image_classifier< vxl_uint_16, double >;
template class vidtk::hashed_image_classifier< vxl_uint_16, float >;

} // end namespace vidtk
