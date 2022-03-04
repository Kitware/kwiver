// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the implementation of the \c klv_value class.

#include "klv_value.h"

#include "klv_0102.h"
#include "klv_0104.h"
#include "klv_0601.h"
#include "klv_0806_aoi_set.h"
#include "klv_0806_poi_set.h"
#include "klv_0806_user_defined_set.h"
#include "klv_0806.h"
#include "klv_0903_algorithm_set.h"
#include "klv_0903_location_pack.h"
#include "klv_0903_ontology_set.h"
#include "klv_0903_vchip_set.h"
#include "klv_0903_vmask_set.h"
#include "klv_0903_vobject_set.h"
#include "klv_0903_vtarget_pack.h"
#include "klv_0903_vtracker_set.h"
#include "klv_0903_vtrackitem_pack.h"
#include "klv_0903.h"
#include "klv_1010.h"
#include "klv_1108_metric_set.h"
#include "klv_1108.h"
#include "klv_1204.h"
#include "klv_blob.h"
#include "klv_packet.h"
#include "klv_series.hpp"
#include "klv_set.h"

#include <vital/util/demangle.h>

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
template < class T,
           typename std::enable_if< !std::is_floating_point< T >::value,
                                    bool >::type = true >
bool
equal_or_nan( T const& lhs, T const& rhs )
{
  return lhs == rhs;
}

// ----------------------------------------------------------------------------
template < class T,
           typename std::enable_if< std::is_floating_point< T >::value,
                                    bool >::type = true >
bool
equal_or_nan( T const& lhs, T const& rhs )
{
  return lhs == rhs || ( std::isnan( lhs ) && std::isnan( rhs ) );
}

} // namespace

// ----------------------------------------------------------------------------
klv_bad_value_cast
::klv_bad_value_cast( std::type_info const& requested_type,
                      std::type_info const& actual_type )
{
  std::stringstream ss;
  ss    << "klv_value: type "
        << kwiver::vital::demangle( requested_type.name() )
        << " was requested, but the object holds type "
        << kwiver::vital::demangle( actual_type.name() );
  m_message = ss.str();
}

// ----------------------------------------------------------------------------
char const*
klv_bad_value_cast
::what() const noexcept
{
  return m_message.c_str();
}

// ----------------------------------------------------------------------------
class klv_value::internal_base
{
public:
  virtual ~internal_base() = default;

  virtual std::type_info const& type() const noexcept = 0;
  virtual bool less_than( internal_base const& rhs ) const = 0;
  virtual bool equal_to( internal_base const& rhs ) const = 0;
  virtual std::ostream& print( std::ostream& os ) const = 0;
  virtual internal_base* clone() const = 0;
  virtual kwiver::vital::any to_any() const = 0;
};

// ----------------------------------------------------------------------------
template < class T >
class klv_value::internal_ : public internal_base
{
public:
  explicit
  internal_( T const& value ) : m_item( value ) {}

  explicit
  internal_( T&& value ) : m_item( value ) {}

  std::type_info const&
  type() const noexcept override final
  {
    return typeid( T );
  }

  bool
  less_than( internal_base const& rhs ) const override final
  {
    auto const& lhs = *this;
    // First, compare types
    if( lhs.type().before( rhs.type() ) )
    {
      return true;
    }
    else if( lhs.type() == rhs.type() )
    {
      auto const& rhs_item =
        dynamic_cast< internal_< T > const& >( rhs ).m_item;
      // Second, compare values
      return lhs.m_item < rhs_item;
    }
    return false;
  }

  bool
  equal_to( internal_base const& rhs ) const override final
  {
    auto const& lhs = *this;

    // First, compare types
    if( lhs.type() != rhs.type() )
    {
      return false;
    }

    auto const& rhs_item =
      dynamic_cast< internal_< T > const& >( rhs ).m_item;
    // Second, compare values
    return equal_or_nan( lhs.m_item, rhs_item );
  }

  std::ostream&
  print( std::ostream& os ) const override final
  {
    return os << m_item;
  }

  internal_base*
  clone() const override final
  {
    return new internal_< T >{ m_item };
  }

  kwiver::vital::any
  to_any() const override final
  {
    return m_item;
  }

  T m_item;
};

// ----------------------------------------------------------------------------
klv_value
::klv_value()
{}

// ----------------------------------------------------------------------------
template < class T, typename > klv_value
::klv_value( T&& value )
{
  m_item.reset( new internal_< typename std::decay< T >::type >{
                  std::forward< T >( value ) } );
}

// ----------------------------------------------------------------------------
klv_value
::klv_value( klv_value const& other )
{
  m_item.reset( other.m_item ? other.m_item->clone() : nullptr );
}

// ----------------------------------------------------------------------------
klv_value
::klv_value( klv_value&& other )
{
  swap( other );
}

// ----------------------------------------------------------------------------
klv_value
::~klv_value() {}

// ----------------------------------------------------------------------------
klv_value&
klv_value
::operator=( klv_value const& other )
{
  m_item.reset( other.m_item ? other.m_item->clone() : nullptr );
  return *this;
}

// ----------------------------------------------------------------------------
klv_value&
klv_value
::operator=( klv_value&& other )
{
  return swap( other );
}

// ----------------------------------------------------------------------------
template < class T >
klv_value&
klv_value
::operator=( T&& rhs )
{
  klv_value{ rhs }.swap( *this );
  return *this;
}

// ----------------------------------------------------------------------------
kwiver::vital::any
klv_value
::to_any() const
{
  return m_item ? m_item->to_any() : kwiver::vital::any{};
}

// ----------------------------------------------------------------------------
klv_value&
klv_value
::swap( klv_value& rhs ) noexcept
{
  m_item.swap( rhs.m_item );
  return *this;
}

// ----------------------------------------------------------------------------
bool
klv_value
::empty() const noexcept
{
  return !m_item;
}

// ----------------------------------------------------------------------------
bool
klv_value
::valid() const noexcept
{
  return m_item && type() != typeid( klv_blob );
}

// ----------------------------------------------------------------------------
void
klv_value
::clear() noexcept
{
  m_item.reset();
}

// ----------------------------------------------------------------------------
std::type_info const&
klv_value
::type() const noexcept
{
  return m_item ? m_item->type() : typeid( void );
}

// ----------------------------------------------------------------------------
std::string
klv_value
::type_name() const noexcept
{
  return kwiver::vital::demangle( type().name() );
}

// ----------------------------------------------------------------------------
std::string
klv_value
::to_string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

// ----------------------------------------------------------------------------
template < class T >
T&
klv_value
::get()
{
  auto const ptr = get_ptr< T >();
  if( !ptr )
  {
    throw klv_bad_value_cast( typeid( T ), type() );
  }
  return *ptr;
}

// ----------------------------------------------------------------------------
template < class T >
T const&
klv_value
::get() const
{
  auto const ptr = get_ptr< T >();
  if( !ptr )
  {
    throw klv_bad_value_cast( typeid( T ), type() );
  }
  return *ptr;
}

// ----------------------------------------------------------------------------
template < class T >
T*
klv_value
::get_ptr() noexcept
{
  if( !m_item )
  {
    return nullptr;
  }

  auto const ptr = dynamic_cast< internal_< T >* >( m_item.get() );
  return ptr ? &ptr->m_item : nullptr;
}

// ----------------------------------------------------------------------------
template < class T >
T const*
klv_value
::get_ptr() const noexcept
{
  if( !m_item )
  {
    return nullptr;
  }

  auto const ptr = dynamic_cast< internal_< T > const* >( m_item.get() );
  return ptr ? &ptr->m_item : nullptr;
}

// ----------------------------------------------------------------------------
bool
operator<( klv_value const& lhs, klv_value const& rhs )
{
  if( rhs.empty() )
  {
    return false;
  }
  return lhs.empty() ? true : lhs.m_item->less_than( *rhs.m_item );
}

// ----------------------------------------------------------------------------
bool
operator==( klv_value const& lhs, klv_value const& rhs )
{
  if( lhs.empty() != rhs.empty() )
  {
    return false;
  }
  return ( lhs.empty() && rhs.empty() )
         ? true
         : lhs.m_item->equal_to( *rhs.m_item );
}

// ----------------------------------------------------------------------------
bool
operator!=( klv_value const& lhs, klv_value const& rhs )
{
  return !( lhs == rhs );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_value const& rhs )
{
  return rhs.empty() ? os << "(empty)" : rhs.m_item->print( os );
}

// ----------------------------------------------------------------------------
#define KLV_INSTANTIATE( T )                         \
  template KWIVER_ALGO_KLV_EXPORT                    \
  klv_value::klv_value( T&& );                       \
  template KWIVER_ALGO_KLV_EXPORT                    \
  klv_value::klv_value( T& );                        \
  template KWIVER_ALGO_KLV_EXPORT                    \
  klv_value::klv_value( T const& );                  \
  template KWIVER_ALGO_KLV_EXPORT                    \
  klv_value & klv_value::operator= < T >( T && );    \
  template KWIVER_ALGO_KLV_EXPORT                    \
  T & klv_value::get< T >();                         \
  template KWIVER_ALGO_KLV_EXPORT                    \
  T const& klv_value::get< T >() const;              \
  template KWIVER_ALGO_KLV_EXPORT                    \
  T * klv_value::get_ptr< T >() noexcept;            \
  template KWIVER_ALGO_KLV_EXPORT                    \
  T const* klv_value::get_ptr< T >() const noexcept; \
  template class KWIVER_ALGO_KLV_EXPORT              \
  klv_value::internal_< T >;

KLV_INSTANTIATE( double );
KLV_INSTANTIATE( int64_t );
KLV_INSTANTIATE( klv_0102_country_coding_method );
KLV_INSTANTIATE( klv_0102_security_classification );
KLV_INSTANTIATE( klv_0601_airbase_locations );
KLV_INSTANTIATE( klv_0601_control_command );
KLV_INSTANTIATE( klv_0601_country_codes );
KLV_INSTANTIATE( klv_0601_frame_rate );
KLV_INSTANTIATE( klv_0601_icing_detected );
KLV_INSTANTIATE( klv_0601_location_dlp );
KLV_INSTANTIATE( klv_0601_operational_mode );
KLV_INSTANTIATE( klv_0601_platform_status );
KLV_INSTANTIATE( klv_0601_sensor_control_mode );
KLV_INSTANTIATE( klv_0601_sensor_fov_name );
KLV_INSTANTIATE( klv_0601_view_domain );
KLV_INSTANTIATE( klv_0601_weapons_general_status );
KLV_INSTANTIATE( klv_0601_weapons_store );
KLV_INSTANTIATE( klv_0806_aoi_type );
KLV_INSTANTIATE( klv_0806_user_defined_data );
KLV_INSTANTIATE( klv_0806_user_defined_data_type );
KLV_INSTANTIATE( klv_0806_user_defined_data_type_id );
KLV_INSTANTIATE( klv_0903_detection_status );
KLV_INSTANTIATE( klv_0903_fpa_index );
KLV_INSTANTIATE( klv_0903_location_pack );
KLV_INSTANTIATE( klv_0903_pixel_run );
KLV_INSTANTIATE( klv_0903_velocity_pack );
KLV_INSTANTIATE( klv_0903_vtarget_pack );
KLV_INSTANTIATE( klv_0903_vtrackitem_pack );
KLV_INSTANTIATE( klv_1010_sdcc_flp );
KLV_INSTANTIATE( klv_1108_assessment_point );
KLV_INSTANTIATE( klv_1108_compression_profile );
KLV_INSTANTIATE( klv_1108_compression_type );
KLV_INSTANTIATE( klv_1108_metric_implementer );
KLV_INSTANTIATE( klv_1108_metric_period_pack );
KLV_INSTANTIATE( klv_1108_window_corners_pack );
KLV_INSTANTIATE( klv_1204_miis_id );
KLV_INSTANTIATE( klv_blob );
KLV_INSTANTIATE( klv_lengthy< double > );
KLV_INSTANTIATE( klv_local_set );
KLV_INSTANTIATE( klv_universal_set );
KLV_INSTANTIATE( klv_uuid );
KLV_INSTANTIATE( std::string );
KLV_INSTANTIATE( std::vector< klv_0601_payload_record > );
KLV_INSTANTIATE( std::vector< klv_0601_wavelength_record > );
KLV_INSTANTIATE( std::vector< klv_0601_weapons_store > );
KLV_INSTANTIATE( std::vector< klv_0903_location_pack > );
KLV_INSTANTIATE( std::vector< klv_0903_pixel_run > );
KLV_INSTANTIATE( std::vector< klv_0903_vtarget_pack > );
KLV_INSTANTIATE( std::vector< klv_local_set > );
KLV_INSTANTIATE( std::vector< klv_packet > );
KLV_INSTANTIATE( std::vector< uint16_t > );
KLV_INSTANTIATE( std::vector< uint64_t > );
KLV_INSTANTIATE( uint64_t );

} // namespace klv

} // namespace arrows

} // namespace kwiver
