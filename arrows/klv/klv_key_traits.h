// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utility file to deal with local and universal sets in a unified manner.

#include <arrows/klv/klv_tag_traits.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
// Helper class for klv_set_format allowing compile-time lookup of functions
// pertaining to a KLV key type.
template < class Key > class key_traits;

// ----------------------------------------------------------------------------
template <>
class key_traits< klv_lds_key >
{
public:
  static klv_lds_key
  read_key( klv_read_iter_t& data, size_t max_length )
  {
    return klv_read_lds_key( data, max_length );
  }

  static void
  write_key( klv_lds_key const& key,
             klv_write_iter_t& data, size_t max_length )
  {
    klv_write_lds_key( key, data, max_length );
  }

  static size_t
  length_of_key( klv_lds_key const& key )
  {
    return klv_lds_key_length( key );
  }

  static klv_tag_traits const&
  tag_traits_from_key( klv_tag_traits_lookup const& lookup,
                       klv_lds_key const& key )
  {
    return lookup.by_tag( key );
  }

  static klv_lds_key
  key_from_tag_traits( klv_tag_traits const& trait )
  {
    return trait.tag();
  }
};

// ----------------------------------------------------------------------------
template <>
class key_traits< klv_uds_key >
{
public:
  static klv_uds_key
  read_key( klv_read_iter_t& data, size_t max_length )
  {
    return klv_read_uds_key( data, max_length );
  }

  static void
  write_key( klv_uds_key const& key,
             klv_write_iter_t& data, size_t max_length )
  {
    klv_write_uds_key( key, data, max_length );
  }

  static size_t
  length_of_key( klv_uds_key const& key )
  {
    return klv_uds_key_length( key );
  }

  static klv_tag_traits const&
  tag_traits_from_key( klv_tag_traits_lookup const& lookup,
                       klv_uds_key const& key )
  {
    return lookup.by_uds_key( key );
  }

  static klv_uds_key
  key_from_tag_traits( klv_tag_traits const& trait )
  {
    return trait.uds_key();
  }
};

} // namespace klv

} // namespace arrows

} // namespace kwiver
