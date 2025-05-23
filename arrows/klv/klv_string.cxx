// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV string data formats.

#include <arrows/klv/klv_string.h>

#include <vital/util/text_codec_ascii.h>
#include <vital/util/text_codec_error_policies.h>
#include <vital/util/text_codec_transcode.h>
#include <vital/util/text_codec_utf_16.h>
#include <vital/util/text_codec_utf_8.h>

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
vital::text_codec::encode_error_policy const&
encode_error_policy()
{
  return vital::text_codec_encode_error_policy_abort::instance();
}

// ----------------------------------------------------------------------------
vital::text_codec::decode_error_policy const&
decode_error_policy()
{
  return vital::text_codec_decode_error_policy_abort::instance();
}

// ----------------------------------------------------------------------------
template< class Codec >
Codec const&
get_text_codec()
{
  static Codec const codec =
    [](){
      Codec tmp;
      tmp.set_encode_error_policy( encode_error_policy() );
      tmp.set_decode_error_policy( decode_error_policy() );
      return tmp;
    }();
  return codec;
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
klv_string_format
::klv_string_format(
  vital::text_codec const& codec,
  klv_length_constraints const& char_constraints,
  klv_length_constraints const& byte_constraints )
  : klv_data_format_< data_type >{ byte_constraints },
    m_codec{ &codec },
    m_char_constraints{ char_constraints }
{}

// ----------------------------------------------------------------------------
klv_string_format
::~klv_string_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_string_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  // Read bytes
  auto const value = klv_read_string( data, length );

  // Calculate number of characters in those bytes
  auto const length_tuple = m_codec->decoded_size( value );
  if( std::get< 0 >( length_tuple ) == vital::text_codec::ABORT )
  {
    throw vital::metadata_exception(
      "string is not valid " + m_codec->name() );
  }

  // Check character constraints
  auto const char_count = std::get< 1 >( length_tuple );
  if( !m_char_constraints.do_allow( char_count ) )
  {
    LOG_WARN( vital::get_logger( "klv" ),
      "format `" << description() << "` "
      << "received wrong number of characters ( " << char_count << " ) "
      << "when reading" );
  }

  return value;
}

// ----------------------------------------------------------------------------
void
klv_string_format
::write_typed( std::string const& value,
               klv_write_iter_t& data, size_t length ) const
{
  // Calculate number of characters in input
  auto const length_tuple = m_codec->decoded_size( value );
  if( std::get< 0 >( length_tuple ) == vital::text_codec::ABORT )
  {
    throw vital::metadata_exception(
      "string is not valid " + m_codec->name() );
  }

  // Check character constraints
  auto const char_count = std::get< 1 >( length_tuple );
  if( !m_char_constraints.do_allow( char_count ) )
  {
    LOG_WARN( vital::get_logger( "klv" ),
      "format `" << description() << "` "
      << "received wrong number of characters ( " << char_count << " ) "
      << "when writing" );
  }

  klv_write_string( value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_string_format
::length_of_typed( std::string const& value ) const
{
  return klv_string_length( value );
}

// ----------------------------------------------------------------------------
std::string
klv_string_format
::description_() const
{
  auto const result = "String (Encoding: " + m_codec->name() + ")";
  if( m_char_constraints.is_free() )
  {
    return result;
  }
  return result + " (Chars: " + m_char_constraints.description() + ")";
}

// ----------------------------------------------------------------------------
std::ostream&
klv_string_format
::print_typed( std::ostream& os, std::string const& value ) const
{
  static vital::text_codec_utf_8 print_codec;
  auto const transcoded =
    vital::text_codec_transcode( *m_codec, print_codec, value );
  if( std::get< 0 >( transcoded ) == vital::text_codec::ABORT )
  {
    return os << "<invalid>";
  }
  return os << std::get< 1 >( transcoded );
}

// ----------------------------------------------------------------------------
vital::text_codec const&
klv_string_format
::codec() const
{
  return *m_codec;
}

// ----------------------------------------------------------------------------
klv_ascii_format
::klv_ascii_format( klv_length_constraints const& length_constraints )
  : klv_string_format{
    get_text_codec< vital::text_codec_ascii >(),
    length_constraints, length_constraints }
{}

// ----------------------------------------------------------------------------
klv_ascii_format
::~klv_ascii_format()
{}

// ----------------------------------------------------------------------------
klv_utf_8_format
::klv_utf_8_format(
  klv_length_constraints const& char_constraints,
  klv_length_constraints const& byte_constraints )
  : klv_string_format{
    get_text_codec< vital::text_codec_utf_8 >(),
    char_constraints, byte_constraints }
{}

// ----------------------------------------------------------------------------
klv_utf_8_format
::~klv_utf_8_format()
{}

// ----------------------------------------------------------------------------
klv_utf_16_format
::klv_utf_16_format(
  klv_length_constraints const& char_constraints,
  klv_length_constraints const& byte_constraints )
  : klv_string_format{
    get_text_codec< vital::text_codec_utf_16_be >(),
    char_constraints, byte_constraints }
{}

// ----------------------------------------------------------------------------
klv_utf_16_format
::~klv_utf_16_format()
{}

} // namespace klv

} // namespace arrows

} // namespace kwiver
