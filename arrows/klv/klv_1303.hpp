// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1303 parser's templated functions.

#ifndef KWIVER_ARROWS_KLV_KLV_1303_HPP_
#define KWIVER_ARROWS_KLV_KLV_1303_HPP_

#include "klv_1303.h"

#include <arrows/klv/klv_util.h>

#include <vital/range/iota.h>

#include <list>
#include <numeric>
#include <sstream>
#include <type_traits>

namespace kv = kwiver::vital;
namespace kvr = kwiver::vital::range;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
// Calculate the jumps in memory required to move along each dimension of a
// multi-dimensional array.
std::vector< size_t >
get_deltas( std::vector< size_t > const& sizes )
{
  std::vector< size_t > deltas( sizes.size(), 1 );
  for( size_t i = 0; i < deltas.size(); ++i )
  {
    for( size_t j = i + 1; j < deltas.size(); ++j )
    {
      deltas[ i ] *= sizes[ j ];
    }
  }

  return deltas;
}

// ----------------------------------------------------------------------------
// One run-length entry, representing an n-dimensional rectangle of the same
// contiguous value. It starts out as just a single element, then grows as more
// continuous elements are added on. It may have a parent during construction:
// for example, a two-dimensional entry may have a one-dimensional entry as its
// child until it is determined whether that one-dimensional entry is large
// enough to be added on as the next row of its parent.
template < class T >
struct rle_entry
{
  using parent_iterator = typename std::list< rle_entry >::iterator;

  // Create as a single element
  rle_entry(
    T value, std::vector< size_t > const& coords, parent_iterator parent )
    : value{ value },
      coordinates{ coords },
      run_lengths( coords.size(), 1 ),
      parent{ parent },
      rank{ 0 }
  {}

  // Update the rank (what dimension this entry should be looking to expand on)
  // based on which element is being processed.
  void
  update_on_coords( std::vector< size_t > const& coords )
  {
    for( size_t i = 0; i < coords.size() - rank - 1; ++i )
    {
      if( coords[ i ] >= coordinates[ i ] + run_lengths[ i ] )
      {
        rank = coords.size() - i - 1;
        break;
      }
      if( coords[ i ] < coordinates[ i ] + run_lengths[ i ] - 1 )
      {
        break;
      }
    }
  }

  // Return whether this entry cannot expand any further
  bool
  done() const
  {
    return rank == coordinates.size();
  }

  // Determine whether the element being processed could be the beginning of a
  // new child entry
  bool
  check_child_begin( std::vector< size_t > const& coords )
  {
    update_on_coords( coords );
    if( done() || rank == 0 )
    {
      return false;
    }

    auto const rank_index = run_lengths.size() - rank - 1;
    for( size_t i = 0; i < coords.size(); ++i )
    {
      auto const expected_value =
        coordinates[ i ] + ( ( i == rank_index ) ? run_lengths[ i ] : 0 );
      if( coords[ i ] != expected_value )
      {
        return false;
      }
    }
    return true;
  }

  // Determine whether the element being processed could be the final element
  // of a child entry
  bool
  check_child_end( std::vector< size_t > const& coords )
  {
    update_on_coords( coords );
    if( done() )
    {
      return false;
    }

    auto const rank_index = run_lengths.size() - rank - 1;
    for( size_t i = 0; i < coords.size(); ++i )
    {
      auto const expected_value =
        coordinates[ i ] + run_lengths[ i ] - ( ( i == rank_index ) ? 0 : 1 );
      if( coords[ i ] != expected_value )
      {
        return false;
      }
    }
    return true;
  }

  T value;
  std::vector< size_t > coordinates;
  std::vector< size_t > run_lengths;
  parent_iterator parent;
  size_t rank;
};

// ----------------------------------------------------------------------------
// Find the most common element in the MDAP
template < class T >
T
rle_default_element( klv_1303_mdap< T > const& value )
{
  std::map< T, size_t > counts;
  for( auto const& element : value.elements )
  {
    ++counts.emplace( element, 0 ).first->second;
  }

  return
    std::max_element(
    counts.begin(), counts.end(),
    []( std::pair< T, size_t > const& lhs,
        std::pair< T, size_t > const& rhs ) -> bool {
      return lhs.second < rhs.second;
    } )->first;
}

// ----------------------------------------------------------------------------
// Express the values in the MDAP as a series of RLE entries.
template < class T >
std::vector< rle_entry< T > >
rle_encode( klv_1303_mdap< T > const& value )
{
  // Finalized entries
  std::vector< rle_entry< T > > entries;

  // Entries still in construction
  std::list< rle_entry< T > > tmp_entries;

  // Entry currently being built
  auto entry = tmp_entries.end();

  // Distance to move across each dimension
  auto const deltas = get_deltas( value.sizes );

  // Coordinates of current element
  std::vector< size_t > coordinates( deltas.size(), 0 );

  // To simplify the encoding algorithm, some sub-functions have been broken
  // off here:

  // Process that the current entry could not be completed
  auto interrupt_entry =
    [ & ](){
      // Null check
      if( entry == tmp_entries.end() )
      {
        return;
      }

      // Sever parent connection
      entry->parent = tmp_entries.end();

      // Set current entry to null
      entry = tmp_entries.end();
    };

  // Process that a new row was completed
  auto complete_entry =
    [ & ](){
      // Null check
      if( entry == tmp_entries.end() )
      {
        return;
      }

      // Absorb child entries into parents if they are complete
      for( auto ptr = entry; ptr->parent != tmp_entries.end();)
      {
        auto parent_ptr = ptr->parent;
        if( parent_ptr->check_child_end( coordinates ) )
        {
          // Child becomes more 'row' of parent
          auto const index =
            parent_ptr->run_lengths.size() - parent_ptr->rank - 1;
          ++parent_ptr->run_lengths[ index ];
          tmp_entries.erase( ptr );

          // Check if parent is now complete, to be absorbed by its parent
          ptr = parent_ptr;
        }
        else
        {
          break;
        }
      }

      // Set current entry to null
      entry = tmp_entries.end();
    };

  // Create an entry starting at the current coordinates
  auto create_entry =
    [ & ]( size_t i ){
      auto parent = tmp_entries.end();
      auto const element = value.elements[ i ];

      // Search for a compatible parent
      for( auto it = tmp_entries.begin(); it != tmp_entries.end();)
      {
        if( it->check_child_begin( coordinates ) )
        {
          if( it->value == element )
          {
            // Found compatible parent
            parent = it;
            break;
          }

          // Incompatible; skip
          ++it;
        }
        else if( it->rank == coordinates.size() )
        {
          // Finalize unreachable parent
          entries.emplace_back( std::move( *it ) );
          it = tmp_entries.erase( it );
        }
        else
        {
          // Incompatible; skip
          ++it;
        }
      }

      // Actually create the entry
      tmp_entries.emplace_back( element, coordinates, parent );
      entry = std::prev( tmp_entries.end() );
    };

  // Start of the overall algorithm
  auto const default_element = rle_default_element( value );
  for( size_t i = 0; i < value.elements.size(); ++i )
  {
    if( value.elements[ i ] == default_element )
    {
      // Don't encode "background" element
      interrupt_entry();
      continue;
    }

    // Calculate coordinates from index
    auto tmp_i = i;
    for( size_t j = 0; j < coordinates.size(); ++j )
    {
      coordinates[ j ] = tmp_i / deltas[ j ];
      tmp_i %= deltas[ j ];
    }

    if( entry == tmp_entries.end() )
    {
      // No existing entry; make a new one with just this element
      create_entry( i );
      continue;
    }

    if( value.elements[ i ] != entry->value )
    {
      // Row of same element has been interrupted by a new value
      interrupt_entry();
      create_entry( i );
      continue;
    }

    // This element is a continuation of the current entry
    ++entry->run_lengths.back();

    if( ( entry->parent != tmp_entries.end() &&
          entry->parent->check_child_end( coordinates ) ) ||
        ( entry->parent == tmp_entries.end() &&
          coordinates.back() == value.sizes.back() - 1 ) )
    {
      // Reached end of parent width or row
      complete_entry();
      continue;
    }
  }

  // Finalize any remaining entries
  complete_entry();
  for( auto& tmp_entry : tmp_entries )
  {
    entries.emplace_back( std::move( tmp_entry ) );
  }

  return entries;
}

// ----------------------------------------------------------------------------
void
throw_apa_type_mismatch( klv_1303_apa apa, std::type_info const& type )
{
  std::stringstream ss;
  ss << "MDAP: APA '" << apa << "' is incompatible with data type '"
     << vital::demangle( type.name() ) << "'";
  throw vital::metadata_exception( ss.str() );
}

// ----------------------------------------------------------------------------
void
throw_unknown_apa( klv_1303_apa apa )
{
  std::stringstream ss;
  ss << "MDAP: unknown APA value '" << static_cast< uint64_t >( apa ) << "'";
  throw vital::metadata_exception( ss.str() );
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
DEFINE_TEMPLATE_CMP(
  klv_1303_mdap< T >,
  &klv_1303_mdap< T >::sizes,
  &klv_1303_mdap< T >::elements )

// ----------------------------------------------------------------------------
template < class T >
std::ostream&
operator<<( std::ostream& os, klv_1303_mdap< T > const& value )
{
  return os << "{ "
            << "sizes: " << value.sizes << ", "
            << "elements: " << value.elements
            << " }";
}

// ----------------------------------------------------------------------------
template < class Format >
std::string
klv_1303_mdap_format< Format >
::description_() const
{
  return "ST1303 MDARRAY Pack";
}

// ----------------------------------------------------------------------------
template < class Format >
klv_1303_mdap< typename Format::data_type >
klv_1303_mdap_format< Format >
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  mdap_t result;

  // Number of dimensions
  result.sizes.resize(
      klv_read_ber_oid< size_t >( data, tracker.remaining() ) );
  if( !result.sizes.size() )
  {
    VITAL_THROW( kv::metadata_exception,
                 "MDAP: number of dimensions must be positive." );
  }

  // Dimension sizes
  for( auto& size : result.sizes )
  {
    size = klv_read_ber_oid< size_t >( data, tracker.remaining() );
    if( !size )
    {
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: each dimension's size must be positive" );
    }
  }

  auto const length_of_array =
    std::accumulate(
      result.sizes.begin(), result.sizes.end(),
      size_t{ 1 }, std::multiplies< size_t >{} );

  // Element size
  result.element_size =
    klv_read_ber_oid< size_t >( data, tracker.remaining() );

  // Array processing algorithm
  result.apa = klv_1303_apa_format{}.read_( data, tracker.verify( 1 ) );

  // Array processing algorithm parameters
  std::unique_ptr< klv_data_format > format;
  uint64_t uint_bias;
  element_t rle_default_element = {};
  switch( result.apa )
  {
    case KLV_1303_APA_IMAP:
      if constexpr( std::is_same_v< element_t, double > )
      {
        result.apa_params_length =
          tracker.remaining() - length_of_array * result.element_size;

        auto const param_length = result.apa_params_length / 2;
        auto const minimum =
          klv_read_float( data, tracker.verify( param_length ) );
        auto const maximum =
          klv_read_float( data, tracker.verify( param_length ) );
        result.imap_params = kv::interval< double >{ minimum, maximum };

        using format_t = klv_lengthless_format< klv_imap_format >;
        format.reset( new format_t{ *result.imap_params,
                                    result.element_size } );
      }
      else
      {
        throw_apa_type_mismatch( result.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_NATURAL:
      result.apa_params_length = 0;
      format.reset( new Format{ m_format } );
      format->set_length_constraints( result.element_size );
      break;
    case KLV_1303_APA_UINT:
      if constexpr( std::is_same_v< element_t, uint64_t > )
      {
        result.element_size = 1;
        uint_bias =
          klv_read_ber_oid< uint64_t >( data, tracker.remaining() );
        result.apa_params_length = klv_ber_oid_length( uint_bias );
        format.reset( new klv_ber_oid_format{} );
      }
      else
      {
        throw_apa_type_mismatch( result.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_BOOLEAN:
      if constexpr( std::is_same_v< element_t, bool > )
      {
        result.apa_params_length = 0;
      }
      else
      {
        throw_apa_type_mismatch( result.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_RLE:
      result.apa_params_length = result.element_size;
      format.reset( new Format{ m_format } );
      format->set_length_constraints( result.element_size );
      rle_default_element =
        format->read( data, tracker.verify( result.apa_params_length ) )
        .template get< element_t >();
      break;
    default:
      throw_unknown_apa( result.apa );
      break;
  }

  // Array data elements
  result.elements.reserve( length_of_array );
  switch( result.apa )
  {
    case KLV_1303_APA_IMAP:
    case KLV_1303_APA_NATURAL:
      // Read in each value of constant length
      for( size_t i = 0; i < length_of_array; ++i )
      {
        auto value =
          format->read( data, tracker.verify( result.element_size ) );
        result.elements.emplace_back(
            std::move( value.template get< element_t >() ) );
      }
      break;
    case KLV_1303_APA_UINT:
      if constexpr( std::is_same_v< element_t, uint64_t > )
      {
        // Read in each value of self-encoded length, add bias
        for( size_t i = 0; i < length_of_array; ++i )
        {
          auto const value = format->read( data, tracker.remaining() );
          result.elements.emplace_back(
              value.template get< element_t >() + uint_bias );
        }
      }
      break;
    case KLV_1303_APA_BOOLEAN:
      if constexpr( std::is_same_v< element_t, bool > )
      {
        // Read each bit as a boolean value
        uint8_t byte = 0;
        for( size_t i = 0; i < length_of_array; ++i )
        {
          if( i % 8 == 0 )
          {
            tracker.verify( 1 );
            byte = *data;
            ++data;
          }
          else
          {
            byte <<= 1;
          }
          result.elements.emplace_back( static_cast< bool >( byte & 0x80 ) );
        }
      }
      break;
    case KLV_1303_APA_RLE:
    {
      result.elements.resize( length_of_array, rle_default_element );

      // Precalculate jump sizes along each dimension
      auto const deltas = get_deltas( result.sizes );

      // Read actual RLE entries
      while( tracker.remaining() )
      {
        // Read value we're going to set the block to
        auto const value =
          m_format.read_( data, tracker.verify( result.element_size ) );

        // Read starting coordinates of rectangular block
        std::vector< size_t > coordinates( result.sizes.size(), 0 );
        for( size_t j = 0; j < result.sizes.size(); ++j )
        {
          coordinates[ j ] =
            klv_read_ber_oid< size_t >( data, tracker.remaining() );
        }

        // Read dimensions of rectangular block
        std::vector< size_t > run_lengths( result.sizes.size(), 0 );
        for( size_t j = 0; j < result.sizes.size(); ++j )
        {
          run_lengths[ j ] =
            klv_read_ber_oid< size_t >( data, tracker.remaining() );
        }

        // Determine starting index from coordinates
        size_t j = 0;
        for( size_t k = 0; k < deltas.size(); ++k )
        {
          j += coordinates[ k ] * deltas[ k ];
        }

        // Iterate through each element of the block
        std::vector< size_t > subindices( run_lengths.size(), 0 );
        while( subindices.front() < run_lengths.front() )
        {
          // Set element value
          result.elements[ j ] = value;

          // Increment coordinates within block
          for( size_t k = run_lengths.size() - 1; true; --k )
          {
            // Move index to next element along this dimension
            ++subindices[ k ];
            j += deltas[ k ];

            // Check if we're at the end of this dimension
            if( subindices[ k ] >= run_lengths[ k ] && k != 0 )
            {
              // Reset this dimension and proceed to the next-highest one
              subindices[ k ] = 0;
              j -= deltas[ k ] * run_lengths[ k ];
              continue;
            }

            // No overflow; we're finished
            break;
          }
        }
      }
      break;
    }
    default:
      throw_unknown_apa( result.apa );
      break;
  }

  return result;
}

// ----------------------------------------------------------------------------
template < class Format >
void
klv_1303_mdap_format< Format >
::write_typed( mdap_t const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Number of dimensions
  auto const dimension_count = value.sizes.size();
  if( !dimension_count )
  {
    VITAL_THROW( kv::metadata_exception,
                 "MDAP: number of dimensions must be positive." );
  }
  klv_write_ber_oid( dimension_count, data, tracker.remaining() );

  // Dimension sizes
  for( auto const size : value.sizes )
  {
    if( !size )
    {
      VITAL_THROW( kv::metadata_exception,
                   "MDAP: each dimension's size must be positive" );
    }
    klv_write_ber_oid( size, data, tracker.remaining() );
  }

  // Element size
  klv_write_ber_oid( value.element_size, data, tracker.remaining() );

  // Array processing algorithm
  klv_1303_apa_format{}.write_( value.apa, data, tracker.remaining() );

  // Array processing algorithm parameters
  std::unique_ptr< klv_data_format > format;
  uint64_t uint_bias = 0;
  switch( value.apa )
  {
    case KLV_1303_APA_IMAP:
      if constexpr( std::is_same_v< element_t, double > )
      {
        auto const param_length = value.apa_params_length / 2;
        auto const minimum = value.imap_params->lower();
        auto const maximum = value.imap_params->upper();
        klv_write_float( minimum, data, tracker.verify( param_length ) );
        klv_write_float( maximum, data, tracker.verify( param_length ) );
        using format_t = klv_lengthless_format< klv_imap_format >;
        format.reset( new format_t{ *value.imap_params, value.element_size } );
      }
      else
      {
        throw_apa_type_mismatch( value.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_NATURAL:
      format.reset( new Format{ m_format } );
      format->set_length_constraints( value.element_size );
      break;
    case KLV_1303_APA_UINT:
      if constexpr( std::is_same_v< element_t, uint64_t > )
      {
        uint_bias =
          *std::min_element( value.elements.begin(), value.elements.end() );
        klv_write_ber_oid( uint_bias, data, tracker.remaining() );

        format.reset( new klv_ber_oid_format{} );
      }
      else
      {
        throw_apa_type_mismatch( value.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_BOOLEAN:
      if constexpr( !std::is_same_v< element_t, bool > )
      {
        throw_apa_type_mismatch( value.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_RLE:
      format.reset( new Format{ m_format } );
      format->set_length_constraints( value.element_size );
      format->write( rle_default_element( value ), data, tracker.remaining() );
      break;
    default:
      throw_unknown_apa( value.apa );
      break;
  }

  // Array data elements
  switch( value.apa )
  {
    case KLV_1303_APA_IMAP:
    case KLV_1303_APA_NATURAL:
      // Simple write
      for( auto const element : value.elements )
      {
        if constexpr ( std::is_same_v< element_t, bool > )
        {
          // Special case for std::vector< bool >
          format->write(
            static_cast< element_t >( element ),
            data, tracker.verify( value.element_size ) );
        }
        else
        {
          format->write( element, data, tracker.verify( value.element_size ) );
        }
      }
      break;
    case KLV_1303_APA_UINT:
      // Simple write, but subtract bias from each element
      if constexpr( std::is_same_v< element_t, uint64_t > )
      {
        for( auto const element : value.elements )
        {
          format->write( element - uint_bias, data, tracker.remaining() );
        }
      }
      break;
    case KLV_1303_APA_BOOLEAN:
      // Write bit-efficiently
      if constexpr( std::is_same_v< element_t, bool > )
      {
        for( size_t i = 0; i < value.elements.size(); ++i )
        {
          if( i % 8 == 0 )
          {
            if( i )
            {
              ++data;
            }
            *data = 0;
          }
          if( value.elements[ i ] )
          {
            *data |= ( 0x80 >> ( i % 8 ) );
          }
        }
        ++data;
      }
      break;
    case KLV_1303_APA_RLE:
    {
      // Write as RLE entries
      auto const entries = rle_encode< element_t >( value );
      for( auto const& entry : entries )
      {
        format->write( entry.value, data, tracker.remaining() );
        for( auto const coord : entry.coordinates )
        {
          klv_write_ber_oid( coord, data, tracker.remaining() );
        }
        for( auto const run_length : entry.run_lengths )
        {
          klv_write_ber_oid( run_length, data, tracker.remaining() );
        }
      }
      break;
    }
    default:
      throw_unknown_apa( value.apa );
      break;
  }
}

// ----------------------------------------------------------------------------
template < class Format >
size_t
klv_1303_mdap_format< Format >
::length_of_typed( mdap_t const& value ) const
{
  auto const length_of_dimension_count =
    klv_ber_oid_length( value.sizes.size() );
  auto const length_of_sizes =
    std::accumulate( value.sizes.begin(), value.sizes.end(), size_t{ 0 },
                     []( size_t count, size_t size ){
                       return count + klv_ber_oid_length( size );
                     } );
  auto const length_of_element_size =
    klv_ber_oid_length( value.element_size );
  auto const length_of_apa =
    klv_1303_apa_format{}.length_of_( value.apa );
  auto length_of_apa_params = value.apa_params_length;

  // Determine length of array based on APA
  size_t length_of_array = 0;
  switch( value.apa )
  {
    case KLV_1303_APA_IMAP:
      if constexpr( !std::is_same_v< element_t, double > )
      {
        throw_apa_type_mismatch( value.apa, typeid( element_t ) );
      }
    // Fallthrough is intentional here
    case KLV_1303_APA_NATURAL:
      length_of_array = value.elements.size() * value.element_size;
      break;
    case KLV_1303_APA_BOOLEAN:
      if constexpr( std::is_same_v< element_t, bool > )
      {
        length_of_array = ( value.elements.size() + 7 ) / 8;
      }
      else
      {
        throw_apa_type_mismatch( value.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_UINT:
      if constexpr( std::is_same_v< element_t, uint64_t > )
      {
        auto const bias =
          *std::min_element( value.elements.begin(), value.elements.end() );
        length_of_apa_params = klv_ber_oid_length( bias );
        for( auto const element : value.elements )
        {
          length_of_array += klv_ber_oid_length( element - bias );
        }
      }
      else
      {
        throw_apa_type_mismatch( value.apa, typeid( element_t ) );
      }
      break;
    case KLV_1303_APA_RLE:
    {
      auto const entries = rle_encode< element_t >( value );
      for( auto const& entry : entries )
      {
        length_of_array += value.element_size;
        for( size_t i = 0; i < value.sizes.size(); ++i )
        {
          length_of_array +=
            klv_ber_oid_length( entry.coordinates[ i ] ) +
            klv_ber_oid_length( entry.run_lengths[ i ] );
        }
      }
      break;
    }
    default:
      throw_unknown_apa( value.apa );
      break;
  }

  return length_of_dimension_count +
         length_of_sizes +
         length_of_element_size +
         length_of_apa +
         length_of_apa_params +
         length_of_array;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
