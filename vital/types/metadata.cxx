// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief This file contains the implementation for vital metadata.
 */

#include "metadata.h"
#include "metadata_traits.h"

#include <vital/logger/logger.h>
#include <vital/types/metadata_traits.h>
#include <vital/util/demangle.h>

#include <typeindex>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------------------
metadata_item
::metadata_item( vital_metadata_tag p_tag,
                 kwiver::vital::any const& p_data )
    : m_data{ p_data },
      m_tag{ p_tag }
{
  if( tag_traits_by_tag( m_tag ).type() != m_data.type() )
  {
    throw bad_any_cast( m_data.type_name(),
                        tag_traits_by_tag( m_tag ).type_name() );
  }
}

// ----------------------------------------------------------------------------
metadata_item
::metadata_item( vital_metadata_tag p_tag,
                 kwiver::vital::any&& p_data )
    : m_data{ std::move( p_data ) },
      m_tag{ p_tag }
{
  if( tag_traits_by_tag( m_tag ).type() != m_data.type() )
  {
    throw bad_any_cast( m_data.type_name(),
                        tag_traits_by_tag( m_tag ).type_name() );
  }
}

// ----------------------------------------------------------------------------
bool
metadata_item
::is_valid() const
{
  return m_tag != VITAL_META_UNKNOWN;
}

// ----------------------------------------------------------------------------
std::string
metadata_item
::name() const
{
  return tag_traits_by_tag( m_tag ).name();
}

// ----------------------------------------------------------------------------
std::type_info const&
metadata_item
::type() const
{
  return tag_traits_by_tag( m_tag ).type();
}

// ----------------------------------------------------------------------------
kwiver::vital::any
metadata_item
::data() const
{
  return this->m_data;
}

// ----------------------------------------------------------------------------
double
metadata_item
::as_double() const
{
  return kwiver::vital::any_cast< double > ( this->m_data );
}

// ----------------------------------------------------------------------------
bool
metadata_item
::has_double() const
{
  return m_data.type() == typeid( double );
}

// ----------------------------------------------------------------------------
uint64_t
metadata_item
::as_uint64() const
{
  return kwiver::vital::any_cast< uint64_t > ( this->m_data );
}

// ----------------------------------------------------------------------------
bool
metadata_item
::has_uint64() const
{
  return m_data.type() == typeid( uint64_t );
}

// ----------------------------------------------------------------------------
std::string
metadata_item
::as_string() const
{
  if( this->has_string() )
  {
    return any_cast< string_t >( m_data );
  }

  std::stringstream ss;
  print_value( ss );
  return ss.str();
}

// ----------------------------------------------------------------------------
bool
metadata_item
::has_string() const
{
  return m_data.type() == typeid( std::string );
}

// ----------------------------------------------------------------------------
std::ostream&
metadata_item
::print_value( std::ostream& os ) const
{
  if( m_data.is_type< bool >() )
  {
    os << any_cast< bool >( m_data );
  }
  else if( m_data.is_type< uint64_t >() )
  {
    os << any_cast< uint64_t >( m_data );
  }
  else if ( m_data.is_type< int >() )
  {
    os << any_cast< int >( m_data );
  }
  else if( m_data.is_type< double >() )
  {
    os << any_cast< double >( m_data );
  }
  else if( m_data.is_type< string_t >() )
  {
    os << any_cast< string_t >( m_data );
  }
  else if( m_data.is_type< geo_point >() )
  {
    os << any_cast< geo_point >( m_data );
  }
  else if ( m_data.is_type< geo_polygon >() )
  {
    os << any_cast< geo_polygon >( m_data );
  }
  else
  {
    throw std::logic_error( "unsupported type" );
  }
  return os;
}

// ---------------------------------------------------------------------
metadata_item*
metadata_item
::clone() const
{
  return new metadata_item{ *this };
}


// ==================================================================
metadata
::metadata()
{ }

// ---------------------------------------------------------------------
metadata
::metadata( metadata const& other )
{
  for( auto const& md_item : other.m_metadata_map )
  {
    // Add a copy of the other metadata map's items
    add_copy( md_item.second );
  }
}

// ---------------------------------------------------------------------
metadata&
metadata::operator=( metadata const& other )
{
  auto copy = other;
  *this = std::move( copy );
  return *this;
}

// ---------------------------------------------------------------------
void
metadata
::add( std::unique_ptr< metadata_item >&& item )
{
  if ( !item )
  {
    throw std::invalid_argument{ "null pointer" };
  }

  auto const tag = item->tag();
#ifdef VITAL_STD_MAP_UNIQUE_PTR_ALLOWED
  this->m_metadata_map[ tag ] = std::move( item );
#else
  this->m_metadata_map[ tag ] = item_ptr{ item.release() };
#endif
  if( !add_internal( *item ) )
  {
    std::stringstream ss;
    ss  << demangle( typeid( *this ).name() ) << ": tag "
        << tag_traits_by_tag( tag ).enum_name()
        << " not supported by internal data structure";
    LOG_DEBUG( get_logger( "metadata" ), ss.str() );
  }
}

// ---------------------------------------------------------------------
void
metadata
::add_copy( std::shared_ptr<metadata_item const> const& item )
{
  if ( !item )
  {
    throw std::invalid_argument{ "null pointer" };
  }

  // Since the design intent for this map is that the metadata
  // collection owns the elements, we will clone the item passed in.
  // The original parameter will be freed eventually.
  auto const tag = item->tag();
  this->m_metadata_map[ tag ] = item_ptr{ item->clone() };
  if( !add_internal( *item ) )
  {
    std::stringstream ss;
    ss  << demangle( typeid( *this ).name() ) << ": tag "
        << tag_traits_by_tag( tag ).enum_name()
        << " not supported by internal data structure";
    LOG_DEBUG( get_logger( "metadata" ), ss.str() );
  }
}

// -------------------------------------------------------------------
void
metadata
::add_any( vital_metadata_tag tag, any const& data )
{
  if( tag_traits_by_tag( tag ).type() != data.type() )
  {
    throw bad_any_cast{ data.type_name(), tag_traits_by_tag( tag ).type_name() };
  }
  this->add( std::unique_ptr< metadata_item >( new metadata_item{ tag, data } ) );
}

// -------------------------------------------------------------------
bool
metadata
::has( vital_metadata_tag tag ) const
{
  return m_metadata_map.find( tag ) != m_metadata_map.end();
}

// ------------------------------------------------------------------
metadata_item const&
metadata
::find( vital_metadata_tag tag ) const
{
  static metadata_item unknown_item{ VITAL_META_UNKNOWN, 0 };

  const_iterator_t it = m_metadata_map.find( tag );
  if ( it == m_metadata_map.end() )
  {
    return unknown_item;
  }

  return *(it->second);
}

// ------------------------------------------------------------------
bool
metadata
::erase( vital_metadata_tag tag )
{
  erase_internal( tag );
  return m_metadata_map.erase( tag ) > 0;
}

// ------------------------------------------------------------------
metadata::const_iterator_t
metadata
::begin() const
{
  return m_metadata_map.begin();
}

metadata::const_iterator_t
metadata
::cbegin() const
{
  return m_metadata_map.cbegin();
}

metadata::const_iterator_t
metadata
::end() const
{
  return m_metadata_map.end();
}

metadata::const_iterator_t
metadata
::cend() const
{
  return m_metadata_map.cend();
}

// ---------------------------------------------------------------------
size_t
metadata
::size() const
{
  return m_metadata_map.size();
}

// ---------------------------------------------------------------------
bool
metadata
::empty() const
{
  return m_metadata_map.empty();
}

// ------------------------------------------------------------------
void
metadata
::set_timestamp( kwiver::vital::timestamp const& ts )
{
  if( ts.has_valid_frame() )
  {
    this->add_any< VITAL_META_VIDEO_FRAME_NUMBER >(
      static_cast< uint64_t >( ts.get_frame() ) );
  }
  else
  {
    this->erase( VITAL_META_VIDEO_FRAME_NUMBER );
  }
  if( ts.has_valid_time() )
  {
    this->add_any< VITAL_META_VIDEO_MICROSECONDS >(
      static_cast< uint64_t >( ts.get_time_usec() ) );
  }
  else
  {
    this->erase( VITAL_META_VIDEO_MICROSECONDS );
  }
}

// ---------------------------------------------------------------------
kwiver::vital::timestamp
metadata
::timestamp() const
{
  kwiver::vital::timestamp timestamp_;
  if( this->has( VITAL_META_VIDEO_FRAME_NUMBER ) )
  {
    timestamp_.set_frame(
      this->find( VITAL_META_VIDEO_FRAME_NUMBER ).as_uint64() );
  }
  if( this->has( VITAL_META_VIDEO_MICROSECONDS ) )
  {
    timestamp_.set_time_usec(
      this->find( VITAL_META_VIDEO_MICROSECONDS ).as_uint64() );
  }
  return timestamp_;
}

// ------------------------------------------------------------------
bool
metadata
::add_internal( metadata_item const& item ) { return true; }

// ------------------------------------------------------------------
void
metadata
::erase_internal( vital_metadata_tag tag ) {}

// ------------------------------------------------------------------
std::string
metadata
::format_string( std::string const& val )
{
  const char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  const size_t len( val.size() );
  bool unprintable_found(false);
  std::string ascii;
  std::string hex;

  for (size_t i = 0; i < len; i++)
  {
    char const l_byte = val[i];
    if ( ! isprint( l_byte ) )
    {
      ascii.append( 1, '.' );
      unprintable_found = true;
    }
    else
    {
      ascii.append( 1, l_byte );
    }

    // format as hex
    if (i > 0)
    {
      hex += " ";
    }

    hex += hex_chars[ ( l_byte & 0xF0 ) >> 4 ];
    hex += hex_chars[ ( l_byte & 0x0F ) >> 0 ];

  } // end for

  if (unprintable_found)
  {
    ascii += " (" + hex + ")";
  }

  return ascii;
}

// ------------------------------------------------------------------
std::ostream& print_metadata( std::ostream& str, metadata const& metadata )
{
  auto eix = metadata.end();
  for ( auto ix = metadata.begin(); ix != eix; ix++)
  {
    // process metada items
   std::string name = ix->second->name();
   kwiver::vital::any data = ix->second->data();

   str << "Metadata item: "
       << name
       << " <" << demangle( ix->second->type().name() ) << ">: "
       << metadata::format_string (ix->second->as_string())
       << std::endl;
  } // end for

  return str;
}

// ----------------------------------------------------------------------------
bool test_equal_content( const kwiver::vital::metadata& one,
                         const kwiver::vital::metadata& other )
{
  // They must be the same size to be the same content
  if ( one.size() != other.size() ) { return false; }

  for ( const auto& mi : one )
  {
    // element is <tag, any>
    const auto tag = mi.first;
    const auto metap = mi.second;

    auto& omi = other.find( tag );
    if ( ! omi ) { return false; }

    // It is simpler to just do a string comparison than to try to do
    // a type specific comparison.
    if ( metap->as_string() != omi.as_string() ) { return false; }

  } // end for

  return true;
}

} } // end namespace
