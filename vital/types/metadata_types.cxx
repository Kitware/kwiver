// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the definitions for the types of metadata values.

#include "metadata_types.h"

#include <vital/exceptions.h>

#include <cctype>
#include <ctime>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, std_0102_lds const& )
{
  // TODO
  return os << "std_0102_local_set";
}

// ----------------------------------------------------------------------------
uint64_t
std_0104_datetime_to_unix_timestamp( std::string const& value )
{
  try
  {
    // Check datetime length - should be in YYYYMMDDThhmmss format
    constexpr size_t length = 15;
    if( value.size() != length )
    {
      VITAL_THROW( metadata_exception, "invalid length" );
    }

    // Check datetime format
    for ( size_t i = 0; i < length; ++i )
    {
      auto const c = value[ i ];
      if( ( i == 8 && c != 'T' ) || ( i != 8 && !std::isdigit( c ) ) )
      {
          VITAL_THROW( metadata_exception, "invalid format" );
      }
    }

    // Parse datetime fields
    std::tm datetime = {};
    datetime.tm_year =  std::stoi( value.substr( 0, 4 ) ) - 1900;
    datetime.tm_mon =   std::stoi( value.substr( 4, 2 ) ) - 1;
    datetime.tm_mday =  std::stoi( value.substr( 6, 2 ) );
    datetime.tm_hour =  std::stoi( value.substr( 9, 2 ) );
    datetime.tm_min =   std::stoi( value.substr( 11, 2 ) );
    datetime.tm_sec =   std::stoi( value.substr( 13, 2 ) );
    datetime.tm_wday =  -1;
    datetime.tm_yday =  -1;
    datetime.tm_isdst = -1;

    auto const record = datetime;

    // Call system function to convert UTC -> Unix
    std::time_t const timestamp =
#ifdef _WIN32
      _mkgmtime64( &datetime );
#else
      timegm( &datetime );
#endif

    // Struct will be corrected if given date is invalid (e.g. hour = 24 ->
    // hour = 0 and day += 1). Catch when this happens and throw, since our
    // input is expected to be valid
    if( datetime.tm_year != record.tm_year ||
        datetime.tm_mon  != record.tm_mon ||
        datetime.tm_mday != record.tm_mday ||
        datetime.tm_hour != record.tm_hour ||
        datetime.tm_min  != record.tm_min ||
        datetime.tm_sec  != record.tm_sec )
    {
      VITAL_THROW( metadata_exception, "invalid datetime" );
    }

    // (time_t)(-1) is returned if date is out of representable range.
    // If time_t is signed, negative values are possible (before epoch), but
    // not representable by our uint64_t return type.
    // When C++17 becomes available, use `if constexpr` here to check for signed
    // time_t
    if( timestamp == static_cast< std::time_t >( -1 ) || timestamp < 0 )
    {
      VITAL_THROW( metadata_exception, "out of range" );
    }

    // Convert to microseconds
    return timestamp * 1000000;
  }
  catch( std::exception const& e )
  {
    VITAL_THROW( metadata_exception,
                 std::string() + "invalid 0104 timestamp: " + e.what() );
  }
}

} } // end namespace
