// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to the KLV 0806 User Defined Set parser.

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_packet.h"
#include "klv_set.h"

#include <vital/util/variant/variant.hpp>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0806_user_defined_set_tag : klv_lds_key
{
  KLV_0806_USER_DEFINED_SET_UNKNOWN      = 0,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_ID = 1,
  KLV_0806_USER_DEFINED_SET_DATA         = 2,
  KLV_0806_USER_DEFINED_SET_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_set_tag tag );

// ----------------------------------------------------------------------------
/// Indicates how to interpret the user-defined data bytes.
enum klv_0806_user_defined_data_type
{
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_STRING       = 0,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_INT          = 1,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_UINT         = 2,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_EXPERIMENTAL = 3,
  KLV_0806_USER_DEFINED_SET_DATA_TYPE_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_data_type value );

// ----------------------------------------------------------------------------
/// Contains the data type and entry id for a user-defined data entry.
struct KWIVER_ALGO_KLV_EXPORT klv_0806_user_defined_data_type_id
{
  klv_0806_user_defined_data_type type;
  uint8_t id;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_data_type_id value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_0806_user_defined_data_type_id lhs,
           klv_0806_user_defined_data_type_id rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_0806_user_defined_data_type_id lhs,
            klv_0806_user_defined_data_type_id rhs );

// ----------------------------------------------------------------------------
/// Interprets data as a ST0806 user-defined data type/id.
class KWIVER_ALGO_KLV_EXPORT klv_0806_user_defined_data_type_id_format
  : public klv_data_format_< klv_0806_user_defined_data_type_id >
{
public:
  klv_0806_user_defined_data_type_id_format();

  std::string
  description() const override;

private:
  klv_0806_user_defined_data_type_id
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0806_user_defined_data_type_id const& value,
               klv_write_iter_t& data, size_t length ) const override;
};

// ----------------------------------------------------------------------------
/// Contains the bytes for a user-defined data entry.
struct KWIVER_ALGO_KLV_EXPORT klv_0806_user_defined_data
{
  std::vector< uint8_t > bytes;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_user_defined_data const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_0806_user_defined_data const& lhs,
           klv_0806_user_defined_data const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_0806_user_defined_data const& lhs,
            klv_0806_user_defined_data const& rhs );

// ----------------------------------------------------------------------------
/// Interprets data as a ST0806 user-defined data entry.
class KWIVER_ALGO_KLV_EXPORT klv_0806_user_defined_data_format
  : public klv_data_format_< klv_0806_user_defined_data >
{
public:
  klv_0806_user_defined_data_format();

  std::string
  description() const override;

private:
  klv_0806_user_defined_data
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0806_user_defined_data const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0806_user_defined_data const& value,
                   size_t length_hint ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0801 user-defined local set.
class KWIVER_ALGO_KLV_EXPORT klv_0806_user_defined_set_format
  : public klv_local_set_format
{
public:
  klv_0806_user_defined_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0806 User Defined Set tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0806_user_defined_set_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver
