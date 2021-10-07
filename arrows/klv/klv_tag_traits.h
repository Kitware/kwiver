// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the KLV tag traits interface.

#ifndef KWIVER_ARROWS_KLV_KLV_TAG_TRAITS_H_
#define KWIVER_ARROWS_KLV_KLV_TAG_TRAITS_H_


#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_key.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/types/metadata.h>

#include <string>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Inclusive numerical range describing how many times a tag is allowed to
/// appear in the same metadata set.
class KWIVER_ALGO_KLV_EXPORT klv_tag_count_range
{
public:
  klv_tag_count_range( size_t exact );

  klv_tag_count_range( size_t lower, size_t upper );

  size_t
  lower() const;

  size_t
  upper() const;

  bool
  is_count_allowed( size_t count ) const;

  std::string
  description() const;

  std::string
  error_message( size_t count ) const;

private:
  size_t m_lower;
  size_t m_upper;
};

// ----------------------------------------------------------------------------
/// Contains the constant attributes of a KLV metadata tag.
class KWIVER_ALGO_KLV_EXPORT klv_tag_traits
{
public:
  klv_tag_traits( klv_uds_key uds_key,
                  klv_lds_key tag,
                  std::string const& enum_name,
                  klv_data_format_sptr format,
                  std::string const& name,
                  std::string const& description,
                  klv_tag_count_range const& tag_count_range );

  /// Return the LDS tag.
  klv_lds_key
  tag() const;

  /// Return the UDS key.
  klv_uds_key
  uds_key() const;

  /// Return the tag's name.
  std::string
  name() const;

  /// Return a string version of the LDS tag, e.g. "KLV_0601_CHECKSUM".
  std::string
  enum_name() const;

  /// Return the normative data type of the tag's value.
  std::type_info const&
  type() const;

  /// Return a string representation of the tag's value's data type.
  std::string
  type_name() const;

  /// Return a description of what this tag holds.
  std::string
  description() const;

  /// Return the data format used to represent this tag's value.
  klv_data_format&
  format() const;

  /// Return a range describing how many times this tag can appear in the same
  /// metadata set.
  klv_tag_count_range
  tag_count_range() const;

private:
  std::string m_name;
  std::string m_enum_name;
  std::string m_description;
  klv_lds_key m_lds_key;
  klv_uds_key m_uds_key;
  klv_data_format_sptr m_format;
  klv_tag_count_range m_tag_count_range;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
