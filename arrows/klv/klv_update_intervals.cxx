// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of KLV update interval settings.

#include "klv_update_intervals.h"

#include <arrows/klv/klv_all.h>

#include <vital/logger/logger.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_update_intervals::key_t
::key_t( klv_top_level_tag standard ) : standard{ standard }, tag{}
{}

// ----------------------------------------------------------------------------
klv_update_intervals::key_t
::key_t(
  klv_top_level_tag standard, std::optional< klv_lds_key > tag )
  : standard{ standard }, tag{ tag }
{}

// ----------------------------------------------------------------------------
klv_update_intervals
::klv_update_intervals() : m_default{ KLV_UPDATE_INTERVAL_DEFAULT }, m_map{}
{}

// ----------------------------------------------------------------------------
klv_update_intervals
::klv_update_intervals(
  std::initializer_list< container_t::value_type > const& items )
  : m_default{ KLV_UPDATE_INTERVAL_DEFAULT }, m_map{ items }
{}

// ----------------------------------------------------------------------------
uint64_t
klv_update_intervals
::at( key_t const& key ) const
{
  auto const it = m_map.find( key );
  if( it == m_map.end() )
  {
    auto const jt = m_map.find( { key.standard, std::nullopt } );
    return ( jt == m_map.end() ) ? m_default : jt->second;
  }
  return it->second;
}

// ----------------------------------------------------------------------------
void
klv_update_intervals
::set( key_t const& key, value_t value )
{
  if( value > KLV_UPDATE_INTERVAL_MAX )
  {
    LOG_WARN(
      kv::get_logger( "klv" ),
      "Update interval of " << value
      << " being truncated to maximum value of " << KLV_UPDATE_INTERVAL_MAX );
    value = KLV_UPDATE_INTERVAL_MAX;
  }
  m_map.emplace( key, value ).first->second = value;
}

// ----------------------------------------------------------------------------
void
klv_update_intervals
::set( value_t value )
{
  m_default = value;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_update_intervals::key_t,
  &klv_update_intervals::key_t::standard,
  &klv_update_intervals::key_t::tag )

// ----------------------------------------------------------------------------
klv_update_intervals const&
klv_recommended_update_intervals()
{
  static klv_update_intervals const result;
  // TODO
  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
