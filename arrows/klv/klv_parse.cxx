// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the implementation for the klv parser.

#include "klv_0104.h"
#include "klv_0601.h"
#include "klv_data.h"
#include "klv_key.h"
#include "klv_parse.h"
#include "klv_read_write_int.h"

#include <vital/exceptions/metadata.h>

#include <vital/logger/logger.h>

#include <iomanip>
#include <sstream>

#include <cctype>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ---------------------------------------------------------------------------
// If unprintable chars detected, replaces them with '.' and appends hex of
// entire string
std::string
format_string( std::string const& value )
{
  // Prints value, but with any unprintable chars replaced with '.'
  bool print_hex = false;
  std::stringstream ss;
  for( char c : value )
  {
    // See https://en.cppreference.com/w/cpp/string/byte/isprint for rationale
    // for static_cast
    if( std::isprint( static_cast< unsigned char >( c ) ) )
    {
      ss << c;
    }
    else
    {
      ss << '.';
      print_hex = true;
    }
  }

  // Outputs like " (01 5A 2C 00)"
  if( print_hex )
  {
    ss << " (";
    ss << std::hex << std::uppercase << std::setfill( '0' ) << std::setw( 2 );
    for( auto it = value.cbegin(); it != value.cend(); ++it )
    {
      if( it != value.cbegin() )
      {
        ss << ' ';
      }
      ss << static_cast< unsigned int >( *it );
    }
    ss << ')';
  }

  return ss.str();
}

} // end namespace

// ---------------------------------------------------------------------------
// Pop the first KLV UDS key-value pair found in the data buffer.
// TODO: Possibly change to work on templated iterator references instead of
//       forcing data type to deque - potential for greater efficiency
bool
klv_pop_next_packet( std::deque< uint8_t >& data, klv_data& klv_packet )
{
  auto const key_length = klv_uds_key::size();

  // Key (key_length bytes) with no Length or Value - must be in Label category
  auto const min_packet_length = key_length;

  while( data.size() >= min_packet_length )
  {
    // The buffer must start with key prefix
    if( std::equal( data.cbegin(), data.cbegin() + 4, klv_uds_key::prefix ) )
    {
      // Must copy to vector to guarantee contiguous memory
      // We are guaranteed enough bytes in the deque because of the
      // preceeding test in while()
      auto const key_bytes =
        std::vector< uint8_t >{ data.cbegin(), data.cbegin() + key_length };
      auto const key = klv_uds_key{ key_bytes.data() };

      if( key.is_valid() )
      {
        if( key.category() == klv_uds_key::CATEGORY_LABEL )
        {
          // Collect bytes that make up the key only
          // Keys with category "Label" have no length or value data
          auto const raw_data =
            klv_data::container_t{ data.begin(), data.begin() + key_length };
          klv_packet = { raw_data, 0, key_length, 0, 0 };

          data.erase( data.begin(), data.begin() + key_length );

          return true;
        }

        try
        {
          // Determine offset and length of value
          // value_begin not const because it is modified by read_ber_encoded()
          auto value_begin = data.cbegin() + key_length;
          auto const value_length =
            klv_read_ber< size_t >( value_begin, data.size() - key_length );
          auto const value_offset =
            static_cast< size_t >(
              std::distance( data.cbegin(), value_begin ) );
          auto const total_length = value_offset + value_length;

          // Is the full packet in the input buffer?
          if( data.size() >= total_length )
          {
            // Collect bytes that make up the key, length, and value
            auto const raw_data =
              klv_data::container_t{ data.begin(),
                                     data.begin() + total_length };
            klv_packet = { raw_data, 0, key_length, value_offset,
                           value_length };

            data.erase( data.begin(), data.begin() + total_length );

            return true;
          }
        }
        catch ( kv::metadata_exception const& e )
        {
          // Could not read length - data buffer too short
        }

        // Buffer too short - no point in continuing
        return false;
      }
    }

    // If prefix does not match or key not valid, delete byte from top of input
    // and try again
    auto logger = kwiver::vital::get_logger( "vital.klv_parse" );
    LOG_DEBUG( logger, "discarding klv byte - 0x" << std::hex
                         << static_cast< int >( data.front() ) );
    data.pop_front();
  }

  return false;
}

// ----------------------------------------------------------------
// Parse out Local Data Set (LDS) packet
std::vector< klv_lds_pair >
parse_klv_lds( klv_data const& data )
{
  std::vector< klv_lds_pair > lds_pairs;

  auto remaining_length = data.value_size();
  auto it = data.value_begin();

  // Key (1 byte), Length (1 byte), Value (0 bytes)
  constexpr size_t min_packet_length = 2;

  while( remaining_length >= min_packet_length )
  {
    try
    {
      // Parse key
      auto const key = klv_lds_key { *( it++ ) }; // 1 byte key; TODO: expand
      --remaining_length;

      // Parse length
      auto const length_begin = it;
      auto const value_length = klv_read_ber< size_t >( it, remaining_length );
      remaining_length -= std::distance( length_begin, it );
      if ( remaining_length < value_length )
      {
        VITAL_THROW( kv::metadata_exception,
                     "insufficient buffer length for complete LDS packet" );
      }

      // Parse value
      auto const value = std::vector< uint8_t >{ it, it + value_length };
      remaining_length -= value_length;
      it += value_length;

      lds_pairs.push_back( { key, value } );
    }
    catch ( kv::metadata_exception const& e )
    {
      auto logger = kwiver::vital::get_logger( "vital.klv_parse" );
      LOG_WARN( logger, "too few bytes while parsing LDS" );
      break;
    }
  }

  if( remaining_length != 0 )
  {
    auto logger = kwiver::vital::get_logger( "vital.klv_parse" );
    LOG_WARN( logger, remaining_length << " bytes left over when parsing LDS" );
  }

  return lds_pairs;
}

// ---------------------------------------------------------------------------
// Parse data set with universal keys
// TODO: We copy each data packet twice here - once into the deque, once into
//       the vector. Suggestion to use iterators instead
klv_uds_vector_t
parse_klv_uds( klv_data const& klv )
{
  std::vector< klv_uds_pair > uds_pairs;

  // Create queue of data portion of packet
  auto data = std::deque< uint8_t >( klv.value_begin(), klv.value_end() );

  for( klv_data packet; klv_pop_next_packet( data, packet ); )
  {
    auto const key = klv_uds_key{ packet }; // 16 byte key
    auto const value =
      std::vector< uint8_t >{ packet.value_begin(), packet.value_end() };
    uds_pairs.push_back( { key, value } );
  }

  if( data.size() != 0 )
  {
    auto logger = kwiver::vital::get_logger( "vital.klv_parse" );
    LOG_WARN( logger, data.size() << " bytes left over when parsing UDS" );
  }

  return uds_pairs;
}

// ---------------------------------------------------------------------------
std::ostream&
print_klv( std::ostream& str, klv_data const& klv )
{
  auto const uds_key = klv_uds_key{ klv }; // create key from raw data

  if( is_klv_0601_key( uds_key ) )
  {
    str << "0601 Universal Key of size " << klv.value_size() << std::endl;
    if( !klv_0601_checksum( klv ) )
    {
      str << "Checksum failed" << std::endl;
      str << "Raw hex of packet: " << klv << std::endl;
    }

    // Try to decode even if checksum failed.
    // This is useful when a valid packet has a bad checksum.
    // May fail badly if packet is really corrupt.
    auto const lds = parse_klv_lds( klv );

    str << "  found " << lds.size() << " tags" << std::endl;
    for( auto it = lds.begin(); it != lds.end(); ++it )
    {
      if( ( it->first <= KLV_0601_UNKNOWN ) ||
          ( it->first >= KLV_0601_ENUM_END ) )
      {
        str << "    #" << static_cast< int >( it->first ) << " is not supported"
            << std::endl;
        continue;
      }

      // Convert a single tag
      auto const tag = klv_0601_get_tag( it->first );

      // Extract relevant data from associated data bytes.
      auto const value =
        klv_0601_value( tag, &it->second[ 0 ], it->second.size() );

      str << "    #" << tag << " - "
          << klv_0601_tag_to_string( tag ) << ": "
          << klv_0601_value_string( tag, value ) << "  ["
          << klv_0601_value_hex_string( tag, value ) << "]" << std::endl;
    }
  }
  else if( klv_0104::is_key( uds_key ) )
  {
    str << "Predator (0104) Universal Key of size " << klv.value_size()
        << std::endl;

    auto const uds = parse_klv_uds( klv );

    str << "  found " << uds.size() << " tags" << std::endl;
    for( auto it = uds.begin(); it != uds.end(); ++it )
    {
      try
      {
        auto const tag = klv_0104::instance()->get_tag( it->first );
        if( tag == klv_0104::UNKNOWN )
        {
          str << "Unknown key: " << it->first
              << "Length: " << it->second.size() << " bytes\n";
          continue;
        }

        auto const value =
          klv_0104::instance()->get_value( tag, &it->second[ 0 ],
                                           it->second.size() );
        auto const str_val =
          format_string( klv_0104::instance()->get_string( tag, value ) );

        str << "    #" << tag << " - "
            << klv_0104::instance()->get_tag_name( tag ) << "("
            << it->second.size() << " bytes): "
            << str_val << " " << std::endl;
      }
      catch ( kwiver::vital::metadata_exception const& e )
      {
        str << "Error in 0104 klv: " << e.what() << "\n";
      }
    }
  }
  else
  {
    str << "Unsupported UDS Key: " << uds_key
        << " data size is " << klv.value_size() << std::endl;

    switch( uds_key.category() )
    {
      case klv_uds_key::CATEGORY_SINGLE:
        str << "  Contains a single data item." << std::endl;
        break;

      case klv_uds_key::CATEGORY_GROUP:
      {
        switch( uds_key.group_type() )
        {
          case klv_uds_key::GROUP_UNIVERSAL_SET:
            str << "  Contains a universal set." << std::endl;
            break;

          case klv_uds_key::GROUP_GLOBAL_SET:
            str << "  Contains a global set." << std::endl;
            break;

          case klv_uds_key::GROUP_LOCAL_SET:
          {
            str << "  Contains a local set." << std::endl;

            auto const lds = parse_klv_lds( klv );
            str << "    found " << lds.size() << " tags" << std::endl;
            str << "    local keys:";
            for( auto it = lds.begin(); it != lds.end(); ++it )
            {
              str << " " << it->first;
            }
            str << std::endl;
            break;
          }

          case klv_uds_key::GROUP_VARIABLE_PACK:
            str << "  Contains a variable length pack." << std::endl;
            break;

          case klv_uds_key::GROUP_FIXED_PACK:
            str << "  Contains a fixed length pack." << std::endl;
            break;

          default:
            str << "  Contains an invalid type of group." << std::endl;
            break;
        }
        break;
      }

      case klv_uds_key::CATEGORY_WRAPPER:
        str << "  Is a wrapper around another data format." << std::endl;
        break;

      case klv_uds_key::CATEGORY_LABEL:
        str << "  Is a label and contains no data." << std::endl;
        break;

      default:
        str << "  Format is unknown." << std::endl;
        break;
    }
  }

  return str;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
