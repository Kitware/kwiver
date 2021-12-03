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

#include <initializer_list>
#include <string>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class klv_tag_traits_lookup;

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
                  klv_tag_count_range const& tag_count_range,
                  klv_tag_traits_lookup const* subtag_lookup = nullptr );

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

  // If this tag corresponds to a local set, return that set's tag lookup
  // object.
  klv_tag_traits_lookup const*
  subtag_lookup() const;

private:
  std::string m_name;
  std::string m_enum_name;
  std::string m_description;
  klv_lds_key m_lds_key;
  klv_uds_key m_uds_key;
  klv_data_format_sptr m_format;
  klv_tag_count_range m_tag_count_range;
  klv_tag_traits_lookup const* m_subtag_lookup;
};

// ----------------------------------------------------------------------------
/// Provides access to tag traits via several lookup alternatives.
class KWIVER_ALGO_KLV_EXPORT klv_tag_traits_lookup
{
public:
  using iterator = typename std::vector< klv_tag_traits >::const_iterator;

  /// Create lookup tables for the tag, uds_key, name, and enum name of
  /// \p traits.
  ///
  /// Elements with empty or invalid lookup keys will silently not be included
  /// in the corresponding lookup tables. For example, if an element has the
  /// empty string as its name, it will not be included in the \c name lookup
  /// table. The first element of \p traits will be returned whenever a lookup
  /// fails.
  ///
  /// \throws logic_error If \p traits is empty, or if it contains two elements
  /// with identical valid lookup keys.
  klv_tag_traits_lookup(
    std::initializer_list< klv_tag_traits > const& traits );

  /// \copydoc
  /// klv_tag_traits_lookup( std::initializer_list< klv_tag_traits > const& )
  klv_tag_traits_lookup( std::vector< klv_tag_traits > const& traits );

  iterator
  begin() const;

  iterator
  end() const;

  /// Return the traits object with \p tag as its tag.
  klv_tag_traits const&
  by_tag( klv_lds_key tag ) const;

  /// Return the traits object with \p key as its UDS key.
  klv_tag_traits const&
  by_uds_key( klv_uds_key const& key ) const;

  /// Return the traits object with \p name as its name.
  klv_tag_traits const&
  by_name( std::string const& name ) const;

  /// Return the traits object with \p enum_name as its enum name.
  klv_tag_traits const&
  by_enum_name( std::string const& enum_name ) const;

private:
  void initialize();

  std::vector< klv_tag_traits > m_traits;
  std::map< klv_lds_key, klv_tag_traits const* > m_tag_to_traits;
  std::map< klv_uds_key, klv_tag_traits const* > m_uds_key_to_traits;
  std::map< std::string, klv_tag_traits const* > m_name_to_traits;
  std::map< std::string, klv_tag_traits const* > m_enum_name_to_traits;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
