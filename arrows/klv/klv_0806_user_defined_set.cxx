// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0806 User Defined Set parser.

#include "klv_0806_user_defined_set.h"

#include "klv_0806.h"

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

klv_tag_traits_lookup const&
klv_0806_user_defined_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0806_USER_DEFINED_SET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E01020311000000 },
      ENUM_AND_NAME( KLV_0806_USER_DEFINED_SET_DATA_TYPE_ID ),
      std::make_shared< klv_0806_user_defined_data_type_id_format >(),
      "Numeric ID and Data Type",
      "Data type and numeric ID for the user defined data.",
      1 },
    { { 0x060E2B3401010101, 0x0E01020312000000 },
      ENUM_AND_NAME( KLV_0806_USER_DEFINED_SET_DATA ),
      std::make_shared< klv_0806_user_defined_data_format >(),
      "User Data",
      "User-defined data. Data type defined in tag 1.",
      1 } };

  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_set_tag tag )
{
  return os << klv_0806_user_defined_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_data_type_id value )
{
  return os << "{ type: " << value.type << ", value: " << value.id << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0806_user_defined_data_type_id,
  &klv_0806_user_defined_data_type_id::type,
  &klv_0806_user_defined_data_type_id::id
)

// ----------------------------------------------------------------------------
klv_0806_user_defined_data_type_id_format
::klv_0806_user_defined_data_type_id_format()
  : klv_data_format_< klv_0806_user_defined_data_type_id >{ 1 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0806_user_defined_data_type_id_format
::description() const
{
  return "user defined data type / id of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_0806_user_defined_data_type_id
klv_0806_user_defined_data_type_id_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0806_user_defined_data_type_id result;
  auto const value = klv_read_int< uint8_t >( data, length );
  result.type =
    static_cast< klv_0806_user_defined_data_type >( ( value & 0xC0 ) >> 6 );
  result.id = value & 0x3F;
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0806_user_defined_data_type_id_format
::write_typed( klv_0806_user_defined_data_type_id const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto int_value = static_cast< uint8_t >( value.id & 0x3F ) |
                   static_cast< uint8_t >( value.type << 6 );
  klv_write_int( int_value, data, length );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_data const& value )
{
  return os << klv_blob{ value.bytes };
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0806_user_defined_data,
  &klv_0806_user_defined_data::bytes
)

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_data_type value )
{
  static std::string strings[ KLV_0806_USER_DEFINED_SET_DATA_TYPE_ENUM_END +
                              1 ] = {
    "String",
    "Signed Integer",
    "Unsigned Integer",
    "Experimental",
    "Unknown Data Type" };

  os << strings[ std::min( value,
                           KLV_0806_USER_DEFINED_SET_DATA_TYPE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
klv_0806_user_defined_data_format
::klv_0806_user_defined_data_format()
{
}

// ----------------------------------------------------------------------------
std::string
klv_0806_user_defined_data_format
::description() const
{
  return "user defined data of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_0806_user_defined_data
klv_0806_user_defined_data_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const begin = data;
  data += length;
  return { { begin, data } };
}

// ----------------------------------------------------------------------------
void
klv_0806_user_defined_data_format
::write_typed( klv_0806_user_defined_data const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_blob( value.bytes, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_0806_user_defined_data_format
::length_of_typed( klv_0806_user_defined_data const& value ) const
{
  return value.bytes.size();
}

// ----------------------------------------------------------------------------
klv_0806_user_defined_set_format
::klv_0806_user_defined_set_format()
  : klv_local_set_format{ klv_0806_user_defined_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0806_user_defined_set_format
::description() const
{
  return "user defined local set of " + m_length_constraints.description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
