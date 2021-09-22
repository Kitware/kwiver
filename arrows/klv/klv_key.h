// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the interface for the KLV key classes.

#ifndef KWIVER_ARROWS_KLV_KLV_KEY_H_
#define KWIVER_ARROWS_KLV_KLV_KEY_H_

#include <arrows/klv/kwiver_algo_klv_export.h>
#include <arrows/klv/klv_read_write.h>

#include <vital/exceptions/metadata.h>
#include <vital/logger/logger.h>

#include <algorithm>
#include <ostream>

#include <cstddef>
#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------
/// Universal Data Set 16-byte key.
class KWIVER_ALGO_KLV_EXPORT klv_uds_key
{
public:
  using const_iterator = uint8_t const*;

  /// Categories of KLV types (represented by byte 5)
  enum category_t
  {
    CATEGORY_INVALID = 0x00,
    CATEGORY_SINGLE  = 0x01,
    CATEGORY_GROUP   = 0x02,
    CATEGORY_WRAPPER = 0x03,
    CATEGORY_LABEL   = 0x04,
    CATEGORY_PRIVATE = 0x05,
    CATEGORY_ENUM_END,
  };

  /// Sub-categories of KLV single items (represented by byte 6)
  enum single_t
  {
    SINGLE_INVALID  = 0x00,
    SINGLE_METADATA = 0x01,
    SINGLE_ESSENCE  = 0x02,
    SINGLE_CONTROL  = 0x03,
    SINGLE_TYPE     = 0x04,
    SINGLE_ENUM_END,
  };

  /// Sub-categories of KLV group items (represented by byte 6)
  enum group_t
  {
    GROUP_INVALID       = 0x00,
    GROUP_UNIVERSAL_SET = 0x01,
    GROUP_GLOBAL_SET    = 0x02,
    GROUP_LOCAL_SET     = 0x03,
    GROUP_VARIABLE_PACK = 0x04,
    GROUP_FIXED_PACK    = 0x05,
    GROUP_ENUM_END,
  };

  /// Sub-categories of KLV wrapper items (represented by byte 6)
  enum wrapper_t
  {
    WRAPPER_INVALID = 0x00,
    WRAPPER_SIMPLE  = 0x01,
    WRAPPER_COMPLEX = 0x02,
    WRAPPER_ENUM_END,
  };

  klv_uds_key();

  template < class Iterator > explicit
  klv_uds_key( Iterator bytes )
  {
    using iterated_t = typename std::decay< decltype( *bytes ) >::type;
    static_assert( std::is_same< uint8_t, iterated_t >::value,
                   "must be iterator to uint8_t" );
    std::copy_n( bytes, length, m_key );
  }

  // For easy construction from literals: klv_uds_key{ 0x..., 0x... };
  klv_uds_key( uint64_t word1, uint64_t word2 );

  /// Access a byte of the key
  uint8_t
  operator[]( size_t index ) const;

  const_iterator
  cbegin() const;

  const_iterator
  cend() const;

  /// Check if this is a valid 16-byte SMPTE-administered Universal Label
  bool
  is_valid() const;

  /// Check if this key has the required 4 byte prefix
  bool
  is_prefix_valid() const;

  /// Return the category represented by this key
  category_t
  category() const;

  /// Return the type of single item (aka dictionary).
  ///
  /// Only valid for keys with CATEGORY_SINGLE.
  single_t
  single_type() const;

  /// Return the type of grouping used.
  ///
  /// Only valid for keys with CATEGORY_GROUP.
  group_t
  group_type() const;

  /// Return the type of wrapper used.
  ///
  /// Only valid for keys with CATEGORY_WRAPPER.
  wrapper_t
  wrapper_type() const;

  /// Return the number of bytes used to represent the length of each group
  /// item.
  ///
  /// Only valid for GROUP_GLOBAL_SET, GROUP_LOCAL_SET, GROUP_VARIABLE_PACK. A
  /// return value of 0 indicates BER encoding, which doesn't have a fixed
  /// length.
  size_t
  group_item_length_size() const;

  /// Return the number of bytes used to represent the local tags.
  ///
  /// Only valid for GROUP_LOCAL_SET. A return value of 0 indicates BER
  /// encoding,
  /// which doesn't have a fixed length.
  size_t
  group_item_tag_size() const;

  /// Formats output as a hex string
  friend
  KWIVER_ALGO_KLV_EXPORT
  std::ostream& operator<<( std::ostream& os, klv_uds_key const& key );

  // All UDS keys start with this 4 byte prefix
  static uint8_t const prefix[ 4 ];

  static size_t const length = 16;

private:
  uint8_t m_key[ length ];
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool operator==( klv_uds_key const& lhs, klv_uds_key const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool operator!=( klv_uds_key const& lhs, klv_uds_key const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool operator<( klv_uds_key const& lhs, klv_uds_key const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_uds_key const& key );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_uds_key::category_t category );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_uds_key::single_t category );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_uds_key::group_t category );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_uds_key::wrapper_t category );

// ----------------------------------------------------------------------------
/// Read a 16-byte Universal Data Set key from a sequence of bytes.
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to read.
///
/// \returns UDS key read in from \p data.
///
/// \throws metadata_buffer_overflow When \p max_length is less than 16.
template < class Iterator >
klv_uds_key
klv_read_uds_key( Iterator& data, size_t max_length )
{
  if( max_length < klv_uds_key::length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "uds key overflows data buffer" );
  }

  auto const value = klv_uds_key( data );
  data += klv_uds_key::length;
  return value;
}

// ----------------------------------------------------------------------------
/// Write a 16-byte Universal Data Set key to a sequence of bytes.
///
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to write.
///
/// \throws metadata_buffer_overflow When \p max_length is less than 16.
template < class Iterator >
void
klv_write_uds_key( klv_uds_key value, Iterator& data, size_t max_length )
{
  if( max_length < klv_uds_key::length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "uds key overflows data buffer" );
  }
  data = std::copy( value.cbegin(), value.cend(), data );
}

// ----------------------------------------------------------------------------
/// Return the number of bytes required to store the given Universal Data Set
/// key.
///
/// \param value Key whose byte length is being queried.
///
/// \returns Bytes required to write \p value.
KWIVER_ALGO_KLV_EXPORT
size_t
klv_uds_key_length( klv_uds_key value );

// ----------------------------------------------------------------------------
/// Local Data Set key with support for up to two bytes.
///
/// Technically there is no upper limit on the size of an LDS key, but as the
/// largest MISB local set currently has fewer than 150 tags, two bytes should
/// be more than sufficient.
using klv_lds_key = uint16_t;

// ----------------------------------------------------------------------------
/// Read a Local Data Set key from a sequence of bytes.
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to read.
///
/// \returns LDS key read in from \p data.
///
/// \throws metadata_buffer_overflow When decoding would require reading more
/// than \p max_length bytes.
template < class Iterator >
klv_lds_key
klv_read_lds_key( Iterator& data, size_t max_length )
{
  return klv_read_ber_oid< klv_lds_key >( data, max_length );
}

// ----------------------------------------------------------------------------
/// Write a Local Data Set key to a sequence of bytes.
///
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to write.
///
/// \throws metadata_buffer_overflow When encoding would require writing more
/// than \p max_length bytes.
template < class Iterator >
void
klv_write_lds_key( klv_lds_key value, Iterator& data, size_t max_length )
{
  klv_write_ber_oid( value, data, max_length );
}

// ----------------------------------------------------------------------------
/// Return the number of bytes required to store the given Local Data Set key.
///
/// \param value Key whose byte length is being queried.
///
/// \returns Bytes required to write \p value.
KWIVER_ALGO_KLV_EXPORT
size_t
klv_lds_key_length( klv_lds_key value );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
