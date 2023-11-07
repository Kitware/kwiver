// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utilities for reading and writing csv files.

#include <arrows/core/csv_io.h>

#include <vital/logger/logger.h>
#include <vital/util/demangle.h>

#include <charconv>
#include <iomanip>
#include <type_traits>
#include <limits>

#include <cctype>
#include <cfloat>

namespace kwiver {

namespace arrows {

namespace core {

namespace csv {

// ----------------------------------------------------------------------------
extern begf_t const begf = {};
extern endf_t const endf = {};
extern skipf_t const skipf = {};
extern endl_t const endl = {};
extern comment_t const comment = {};

} // namespace csv

// ----------------------------------------------------------------------------
csv_io_base
::csv_io_base( char delim, char quote, char quote_esc, char comment )
  : m_delim{ delim },
    m_quote{ quote },
    m_quote_esc{ quote_esc },
    m_comment{ comment }
{}

// ----------------------------------------------------------------------------
char
csv_io_base
::delim() const
{
  return m_delim;
}

// ----------------------------------------------------------------------------
char
csv_io_base
::quote() const
{
  return m_quote;
}

// ----------------------------------------------------------------------------
char
csv_io_base
::quote_esc() const
{
  return m_quote_esc;
}

// ----------------------------------------------------------------------------
char
csv_io_base
::comment() const
{
  return m_comment;
}

// ----------------------------------------------------------------------------
csv_writer
::csv_writer(
  std::ostream& os, char delim, char quote, char quote_esc, char comment )
  : csv_io_base{ delim, quote, quote_esc, comment },
    m_os{ os },
    m_ss{},
    m_in_field{ false },
    m_first_field{ true }
{
  m_os << std::boolalpha;
  m_ss << std::boolalpha;
}

// ----------------------------------------------------------------------------
csv_writer
::~csv_writer()
{
  if( !m_first_field )
  {
    m_os << '\n';
  }
}

// ----------------------------------------------------------------------------
template< class T >
csv_writer&
csv_writer
::write( T const& value )
{
  if constexpr(
    std::is_arithmetic_v< T > ||
    std::is_same_v< T, std::string > ||
    std::is_same_v< T, char const* > )
  {
    if constexpr( std::is_same_v< T, bool > )
    {
      m_ss << value;
    }
    else if constexpr( std::is_integral_v< T > && std::is_signed_v< T > )
    {
      m_ss << static_cast< int64_t >( value );
    }
    else if constexpr( std::is_integral_v< T > && std::is_unsigned_v< T > )
    {
      m_ss << static_cast< uint64_t >( value );
    }
    else if constexpr( std::is_same_v< T, float > )
    {
      m_ss << std::setprecision( FLT_DIG + 1 ) << value;
    }
    else if constexpr( std::is_same_v< T, double > )
    {
      m_ss << std::setprecision( DBL_DIG + 1 ) << value;
    }
    else
    {
      m_ss << value;
    }
    commit();
  }
  else if constexpr( std::is_same_v< T, csv::begf_t > )
  {
    if( m_in_field )
    {
      throw std::invalid_argument(
        "CSV writer received 'begin field' with open field" );
    }
    m_in_field = true;
  }
  else if constexpr( std::is_same_v< T, csv::endf_t > )
  {
    if( !m_in_field )
    {
      throw std::invalid_argument(
        "CSV writer received 'end field' without open field" );
    }
    m_in_field = false;
    commit();
  }
  else if constexpr( std::is_same_v< T, csv::skipf_t > )
  {
    if( m_in_field )
    {
      throw std::invalid_argument(
        "CSV writer received 'skip field' with open field" );
    }
    commit();
  }
  else if constexpr( std::is_same_v< T, csv::endl_t > )
  {
    if( m_in_field )
    {
      throw std::invalid_argument(
        "CSV writer received 'end line' with open field" );
    }
    m_os << '\n';
    m_first_field = true;
  }
  else if constexpr( std::is_same_v< T, csv::comment_t > )
  {
    if( m_in_field )
    {
      throw std::invalid_argument(
        "CSV writer received 'begin comment' with open field" );
    }
    if( !m_first_field )
    {
      throw std::invalid_argument(
        "CSV writer received 'begin comment' in middle of row" );
    }
    m_os << m_comment;
  }
  else
  {
    std::stringstream ss;
    ss << "CSV writer does not support type: "
       << vital::demangle( typeid( T ).name() );
    throw std::invalid_argument( ss.str() );
  }

  return *this;
}

// ----------------------------------------------------------------------------
void
csv_writer
::commit()
{
  if( m_in_field )
  {
    return;
  }

  m_first_field = m_first_field ? false : ( m_os << m_delim, false );

  auto needs_quotes =
    m_ss.str().size() &&
    ( std::isspace( m_ss.str().front() ) ||
      std::isspace( m_ss.str().back() ) );
  if( !needs_quotes )
  {
    for( auto const c : m_ss.str() )
    {
      for( auto const special_c :
          { m_delim, m_quote_esc, m_quote, m_comment, '\n' } )
      {
        if( c == special_c )
        {
          needs_quotes = true;
          break;
        }
      }
    }
  }

  if( needs_quotes )
  {
    m_os << std::quoted( m_ss.str(), m_quote, m_quote_esc );
  }
  else
  {
    m_os << m_ss.str();
  }

  m_ss.str( {} );
  m_ss.clear();
}

// ----------------------------------------------------------------------------
#define INSTANTIATE_WRITE( T, TREF ) \
csv_writer&                          \
operator<<( csv_writer& os, T TREF value ) { return os.write( value ); }

INSTANTIATE_WRITE( char const*, )
INSTANTIATE_WRITE( std::string, const& )
INSTANTIATE_WRITE( bool, )
INSTANTIATE_WRITE( char, )
INSTANTIATE_WRITE( uint8_t, )
INSTANTIATE_WRITE( uint16_t, )
INSTANTIATE_WRITE( uint32_t, )
INSTANTIATE_WRITE( uint64_t, )
INSTANTIATE_WRITE( int8_t, )
INSTANTIATE_WRITE( int16_t, )
INSTANTIATE_WRITE( int32_t, )
INSTANTIATE_WRITE( int64_t, )
INSTANTIATE_WRITE( float, )
INSTANTIATE_WRITE( double, )
INSTANTIATE_WRITE( csv::begf_t, const& )
INSTANTIATE_WRITE( csv::endf_t, const& )
INSTANTIATE_WRITE( csv::skipf_t, const& )
INSTANTIATE_WRITE( csv::endl_t, const& )
INSTANTIATE_WRITE( csv::comment_t, const& )

#undef INSTANTIATE_WRITE

// ----------------------------------------------------------------------------
csv_reader::parse_error
::parse_error( std::string const& string, std::type_info const& to_type )
  : std::runtime_error{
      "CSV reader failed to parse the string '" + string + "' "
      "as type: " + vital::demangle( to_type.name() ) },
    m_string{ string },
    m_to_type{ to_type }
{}

// ----------------------------------------------------------------------------
std::string const&
csv_reader::parse_error
::string() const
{
  return m_string;
}

// ----------------------------------------------------------------------------
std::type_info const&
csv_reader::parse_error
::to_type() const
{
  return m_to_type;
}

// ----------------------------------------------------------------------------
csv_reader
::csv_reader(
  std::istream& is, char delim, char quote, char quote_esc, char comment )
  : csv_io_base{ delim, quote, quote_esc, comment },
    m_is{ is },
    m_first_field{ true },
    m_is_eol{ false }
{
  m_is >> std::boolalpha;
  skip_blank_lines();
}

// ----------------------------------------------------------------------------
csv_reader
::~csv_reader()
{}

namespace {

// ----------------------------------------------------------------------------
// Template to avoid having to do near-identical code for std::stoi, std::stol,
// etc.
template< bool is_signed >
struct str_to_int64;

// ----------------------------------------------------------------------------
template<>
struct str_to_int64< false >
{
  using type = uint64_t;

  template< class... Args >
  type operator()( Args&&... args ) const {
    return std::stoull( std::forward< Args >( args )... ); };
};


// ----------------------------------------------------------------------------
template<>
struct str_to_int64< true >
{
  using type = int64_t;

  template< class... Args >
  type operator()( Args&&... args ) const {
    return std::stoll( std::forward< Args >( args )... ); };
};

// ----------------------------------------------------------------------------
template< class T >
struct is_optional : std::false_type
{};

// ----------------------------------------------------------------------------
template< class T >
struct is_optional< std::optional< T > > : std::true_type
{};

// ----------------------------------------------------------------------------
template< class T >
constexpr bool is_optional_v = is_optional< T >::value;

// ----------------------------------------------------------------------------
template< class T >
struct decay_optional
{
  using type = T;
};

// ----------------------------------------------------------------------------
template< class T >
struct decay_optional< std::optional< T > >
{
  using type = T;
};

// ----------------------------------------------------------------------------
template< class T >
using decay_optional_t = typename decay_optional< T >::type;

// ----------------------------------------------------------------------------
template< class T >
T bad_parse( std::string const& s )
{
  if constexpr( is_optional_v< T > )
  {
    return std::nullopt;
  }
  else
  {
    throw csv_reader::parse_error( s, typeid( T ) );
  }
}

}

// ----------------------------------------------------------------------------
template< class T >
T
csv_reader
::read()
{
  if( is_at_eof() )
  {
    throw std::invalid_argument( "CSV reader read() called at EOF" );
  }

  // Can't read anything else at end of line
  if constexpr( !std::is_same_v< T, csv::endl_t > )
  {
    if( is_at_eol() )
    {
      throw std::invalid_argument( "CSV reader read() called at EOL" );
    }
  }

  // Skip any comment lines, unless we mean to read a comment
  if constexpr( !std::is_same_v< T, csv::comment_t > )
  {
    while( is_at_comment() )
    {
      skip_line();
    }
  }

  // Logic for reading an actual field
  // Result is "intended" string with quotes, escapes processed out
  if constexpr(
    std::is_arithmetic_v< T > ||
    std::is_same_v< T, std::string > ||
    std::is_same_v< T, csv::skipf_t > ||
    is_optional_v< T > )
  {
    if( !m_first_field && m_is.peek() == m_delim )
    {
      // Skip opening delimiter
      m_is.ignore();
    }
    m_first_field = false;

    auto are_in_quotes = m_is.peek() == m_quote;
    auto const were_in_quotes = are_in_quotes;
    if( are_in_quotes )
    {
      // Skip opening quote character
      m_is.ignore();
    }

    std::stringstream ss;
    auto escape_this_char = false;
    for( auto c = m_is.peek(); c != EOF; c = m_is.peek() )
    {
      if( are_in_quotes ) // Quoted field
      {
        if( escape_this_char ) // This char is to be escaped
        {
          if( c != m_quote && c != m_quote_esc )
          {
            // Escape char should be escaped if meant literally
            LOG_WARN(
              vital::get_logger( "csv" ),
              "CSV quote field contains unescaped escape character" );
          }
          escape_this_char = false;
        }
        else if( c == m_quote ) // Unescaped quote char
        {
          // Look at the next character
          m_is.ignore();
          auto const next_c = m_is.peek();

          if( next_c == m_delim || next_c == '\n' || next_c == EOF )
          {
            // Closing quote
            are_in_quotes = false;
            break;
          }

          if( m_quote == m_quote_esc )
          {
            // Quote used to escape a future quote
            escape_this_char = true;
            continue;
          }

          // Rogue quote
          LOG_WARN(
            vital::get_logger( "csv" ),
            "CSV quoted field contains unescaped quote character" );
          continue;
        }
        else if( c == m_quote_esc ) // Unescaped escape char
        {
          escape_this_char = true;
        }
      }
      else if( c == m_quote ) // Quote char in unquoted field
      {
        LOG_WARN(
          vital::get_logger( "csv" ),
          "CSV unquoted field contains quote character" );
      }
      else if( c == m_quote_esc ) // Escape char in unquoted field
      {
        LOG_WARN(
          vital::get_logger( "csv" ),
          "CSV unquoted field contains escape character" );
      }
      else if ( c == m_delim || c == '\n' ) // End of unquoted field
      {
        break;
      }

      // Add non-escaped characters to result
      if( !escape_this_char )
      {
        ss.put( c );
      }

      // On to the next character
      m_is.ignore();
    }

    // Check for no closing quote
    if( are_in_quotes )
    {
      LOG_WARN(
        vital::get_logger( "csv" ),
        "CSV quoted field hit EOF before closing quote" );
    }

    // Check for end of line
    if( auto const c = m_is.peek(); c == EOF || c == '\n' )
    {
      m_is_eol = true;

      // Discard the final newlines closing out the file
      size_t newline_count = 0;
      while( m_is.peek() == '\n' )
      {
        m_is.ignore();
        ++newline_count;
      }

      if( m_is.peek() != EOF )
      {
        for( size_t i = 0; i < newline_count; ++i )
        {
          m_is.putback( '\n' );
        }
      }
    }

    // If we're skipping this field, we're done
    if constexpr( std::is_same_v< T, csv::skipf_t > )
    {
      return {};
    }

    // Get unquoted field text
    std::string s = ss.str();

    // Check for empty field
    if constexpr( is_optional_v< T > )
    {
      if( s.empty() && !were_in_quotes )
      {
        return std::nullopt;
      }
    }
    using decayed_t = decay_optional_t< T >;

    // If all we wanted was a string, just return that
    if constexpr( std::is_same_v< decayed_t, std::string > )
    {
      return s;
    }

    // Convert string to boolean
    if constexpr( std::is_same_v< decayed_t, bool > )
    {
      if( s == "0" || s == "false" )
      {
        return false;
      }
      if( s == "1" || s == "true" )
      {
        return true;
      }

      return bad_parse< T >( s );
    }

    // Numbers should not have leading whitespace
    if constexpr( std::is_arithmetic_v< decayed_t > )
    {
      if( s.empty() || std::isspace( s[ 0 ] ) )
      {
        return bad_parse< T >( s );
      }
    }

    // Convert string to integer
    if constexpr( std::is_integral_v< decayed_t > )
    {
      // Parse into a 64-bit integer
      using parser_t = str_to_int64< std::is_signed_v< decayed_t > >;
      parser_t parser;
      typename parser_t::type value = 0;
      size_t offset = 0;
      try
      {
        value = parser( s, &offset );
      }
      catch( std::exception const& )
      {
        return bad_parse< T >( s );
      }

      // Check if reducing to the desired integer size would overflow
      if( value < std::numeric_limits< decayed_t >::lowest() ||
          value > std::numeric_limits< decayed_t >::max() ||
          !offset || offset != s.size() )
      {
        return bad_parse< T >( s );
      }

      // Downcast to correct type
      return static_cast< T >( value );
    }

    // Parse to floating point
    // Didn't use std::is_floating_point_v because we don't handle long double
    if constexpr(
      std::is_same_v< decayed_t, float > ||
      std::is_same_v< decayed_t, double > )
    {
      decayed_t ( *parser )( char const*, char** ) = nullptr;
      if constexpr( std::is_same_v< decayed_t, float > )
      {
        parser = &std::strtof;
      }
      if constexpr( std::is_same_v< decayed_t, double > )
      {
        parser = &std::strtod;
      }

      auto const begin = &*s.begin();
      auto end = &*s.end();
      auto const value = parser( begin, &end );
      if( end == begin || end != &*s.end() )
      {
        return bad_parse< T >( s );
      }
      return value;
    }
  }

  // Special logic for reading newline
  if constexpr( std::is_same_v< T, csv::endl_t > )
  {
    if( m_is.peek() != '\n' )
    {
      throw std::invalid_argument(
        "CSV reader received 'end line', but cursor was not at end of line" );
    }
    skip_line();
    return {};
  }

  // Special logic for reading comments
  if constexpr( std::is_same_v< T, csv::comment_t > )
  {
    if( m_is.peek() != m_comment )
    {
      throw std::invalid_argument(
        "CSV reader received 'begin comment', but no comment was present" );
    }
    m_is.ignore();
    return {};
  }

  // Catchall
  std::stringstream err_ss;
  err_ss << "CSV reader does not support type: "
         << vital::demangle( typeid( T ).name() );
  throw std::invalid_argument( err_ss.str() );
}

// ----------------------------------------------------------------------------
csv_reader&
csv_reader
::next_line()
{
  read< csv::endl_t >();
  return *this;
}

// ----------------------------------------------------------------------------
csv_reader&
csv_reader
::skip_line()
{
  if( m_is.peek() == EOF )
  {
    throw std::invalid_argument(
      "CSV reader received 'skip line', but cursor is at end of file" );
  }

  for( auto c = m_is.get(); c != '\n' && c != EOF; c = m_is.get() )
  {}

  m_is_eol = false;
  m_first_field = true;
  skip_blank_lines();
  return *this;
}

// ----------------------------------------------------------------------------
csv_reader&
csv_reader
::skip_field()
{
  read< csv::skipf_t >();
  return *this;
}

// ----------------------------------------------------------------------------
void
csv_reader
::skip_blank_lines()
{
  while( m_is.peek() == '\n' )
  {
    m_is.ignore();
  }

  if( is_at_eof() )
  {
    m_is_eol = true;
  }
}


// ----------------------------------------------------------------------------
bool
csv_reader
::is_at_eof()
{
  return m_is.eof();
}

// ----------------------------------------------------------------------------
bool
csv_reader
::is_at_eol()
{
  return m_is_eol;
}

// ----------------------------------------------------------------------------
bool
csv_reader
::is_at_comment()
{
  return m_first_field && m_is.peek() == m_comment;
}

// ----------------------------------------------------------------------------
bool
csv_reader
::is_at_field()
{
  return !is_at_eof() && !is_at_eol() && !is_at_comment();
}

// ----------------------------------------------------------------------------
#define INSTANTIATE_READ( T ) \
  template KWIVER_ALGO_CORE_EXPORT T csv_reader::read< T >();

INSTANTIATE_READ( std::string )
INSTANTIATE_READ( std::optional< std::string > )
INSTANTIATE_READ( bool )
INSTANTIATE_READ( std::optional< bool > )
INSTANTIATE_READ( char )
INSTANTIATE_READ( std::optional< char > )
INSTANTIATE_READ( uint8_t )
INSTANTIATE_READ( std::optional< uint8_t > )
INSTANTIATE_READ( uint16_t )
INSTANTIATE_READ( std::optional< uint16_t > )
INSTANTIATE_READ( uint32_t )
INSTANTIATE_READ( std::optional< uint32_t > )
INSTANTIATE_READ( uint64_t )
INSTANTIATE_READ( std::optional< uint64_t > )
INSTANTIATE_READ( int8_t )
INSTANTIATE_READ( std::optional< int8_t > )
INSTANTIATE_READ( int16_t )
INSTANTIATE_READ( std::optional< int16_t > )
INSTANTIATE_READ( int32_t )
INSTANTIATE_READ( std::optional< int32_t > )
INSTANTIATE_READ( int64_t )
INSTANTIATE_READ( std::optional< int64_t > )
INSTANTIATE_READ( float )
INSTANTIATE_READ( std::optional< float > )
INSTANTIATE_READ( double )
INSTANTIATE_READ( std::optional< double > )
INSTANTIATE_READ( csv::skipf_t )
INSTANTIATE_READ( csv::endl_t )
INSTANTIATE_READ( csv::comment_t )

#undef INSTANTIATE_READ

} // namespace core

} // namespace arrows

} // namespace kwiver
