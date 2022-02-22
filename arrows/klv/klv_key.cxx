// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the implementation for the KLV key classes.

#include "klv_key.h"
#include "klv_read_write.txx"

#include <iomanip>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
uint8_t const
klv_uds_key
::prefix[] = { 0x06, 0x0e, 0x2b, 0x34 };

// ----------------------------------------------------------------------------
klv_uds_key
::klv_uds_key() : m_key{ 0 }
{}

// ----------------------------------------------------------------------------
klv_uds_key
::klv_uds_key( uint64_t word1, uint64_t word2 )
{
  auto it = m_key;
  klv_write_int( word1, it, sizeof( uint64_t ) );
  klv_write_int( word2, it, sizeof( uint64_t ) );
}

// ----------------------------------------------------------------------------
uint8_t
klv_uds_key
::operator[]( size_t index ) const
{
  return ( index < length ) ? m_key[ index ] : 0u;
}

// ----------------------------------------------------------------------------
klv_uds_key::const_iterator
klv_uds_key
::cbegin() const
{
  return m_key;
}

// ----------------------------------------------------------------------------
klv_uds_key::const_iterator
klv_uds_key
::cend() const
{
  return m_key + length;
}

// ----------------------------------------------------------------------------
bool
klv_uds_key
::is_valid() const
{
  if( !is_prefix_valid() )
  {
    return false;
  }

  auto const is_msb_set = []( uint8_t byte ) -> bool {
                            return byte & 0x80;
                          };

  // Bytes 4-7 cannot have most significiant bit set
  if( std::any_of( m_key + 4, m_key + 8, is_msb_set ) )
  {
    return false;
  }

  switch( category() )
  {
    case CATEGORY_SINGLE:
      return single_type() != SINGLE_INVALID;
    case CATEGORY_GROUP:
      return group_type() != GROUP_INVALID;
    case CATEGORY_WRAPPER:
      return wrapper_type() != WRAPPER_INVALID;
    case CATEGORY_LABEL:
    case CATEGORY_PRIVATE:
      return true;
    case CATEGORY_INVALID:
    default:
      return false;
  }
}

// ----------------------------------------------------------------------------
bool
klv_uds_key
::is_prefix_valid() const
{
  return std::equal( m_key, m_key + 4, prefix );
}

// ----------------------------------------------------------------------------
klv_uds_key::category_t
klv_uds_key
::category() const
{
  auto const byte = m_key[ 4 ];
  return ( byte < CATEGORY_ENUM_END )
         ? static_cast< category_t >( byte )
         : CATEGORY_INVALID;
}

// ----------------------------------------------------------------------------
klv_uds_key::single_t
klv_uds_key
::single_type() const
{
  auto const byte = m_key[ 5 ];
  return ( byte < SINGLE_ENUM_END && category() == CATEGORY_SINGLE )
         ? static_cast< single_t >( byte )
         : SINGLE_INVALID;
}

// ----------------------------------------------------------------------------
klv_uds_key::group_t
klv_uds_key
::group_type() const
{
  // Group type encoded in the lower 3 bits
  auto const byte = m_key[ 5 ] & 0x07;
  return ( byte < GROUP_ENUM_END && category() == CATEGORY_GROUP )
         ? static_cast< group_t >( byte )
         : GROUP_INVALID;
}

// ----------------------------------------------------------------------------
klv_uds_key::wrapper_t
klv_uds_key
::wrapper_type() const
{
  auto const byte = m_key[ 5 ];
  return ( byte < WRAPPER_ENUM_END && category() == CATEGORY_WRAPPER )
         ? static_cast< wrapper_t >( byte )
         : WRAPPER_INVALID;
}

// ----------------------------------------------------------------------------
size_t
klv_uds_key
::group_item_length_size() const
{
  // The two-bit number from bits 6 and 7 maps to 0, 1, 2, 4
  uint8_t result = ( m_key[ 5 ] & 0x60 ) >> 5;
  result = ( result == 3 ) ? 4 : result;

  switch( group_type() )
  {
    case GROUP_GLOBAL_SET:
    case GROUP_LOCAL_SET:
    case GROUP_VARIABLE_PACK:
      return result;
    default:
      return 0;
  }
}

// ----------------------------------------------------------------------------
size_t
klv_uds_key
::group_item_tag_size() const
{
  if( this->group_type() != GROUP_LOCAL_SET )
  {
    return 0;
  }

  // The two-bit number from bits 4 and 5 maps to the following values
  size_t const map[] = { 1, 0, 2, 4 };
  return map[ ( m_key[ 5 ] & 0x18 ) >> 3 ];
}

// ----------------------------------------------------------------------------
bool
operator==( klv_uds_key const& lhs, klv_uds_key const& rhs )
{
  // SMPTE specifies that byte 7 does not play a role in a key's uniqueness
  return std::equal( lhs.cbegin(), lhs.cbegin() + 7, rhs.cbegin() ) &&
         std::equal( lhs.cbegin() + 8, lhs.cend(), rhs.cbegin() + 8 );
}

// ----------------------------------------------------------------------------
bool
operator!=( klv_uds_key const& lhs, klv_uds_key const& rhs )
{
  return !( lhs == rhs );
}

// ----------------------------------------------------------------------------
bool
operator<( klv_uds_key const& lhs, klv_uds_key const& rhs )
{
  // SMPTE specifies that byte 7 does not play a role in a key's uniqueness
  return std::lexicographical_compare( lhs.cbegin(), lhs.cbegin() + 7,
                                       rhs.cbegin(), rhs.cbegin() + 7 ) ||
         ( std::equal( lhs.cbegin(), lhs.cbegin() + 7, rhs.cbegin() ) &&
           std::lexicographical_compare( lhs.cbegin() + 8, lhs.cend(),
                                         rhs.cbegin() + 8, rhs.cend() ) );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_uds_key const& key )
{
  auto const flags = os.flags();
  for( size_t i = 0; i < key.length; ++i )
  {
    os << std::hex << std::setfill( '0' ) << std::setw( 2 );
    os << static_cast< unsigned int >( key[ i ] );
    if( i % 4 == 3 && i != key.length - 1 )
    {
      os << ' ';
    }
  }
  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
size_t
klv_uds_key_length( klv_uds_key value )
{
  return value.length;
}

// ----------------------------------------------------------------------------
size_t
klv_lds_key_length( klv_lds_key value )
{
  return klv_ber_oid_length( value );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
