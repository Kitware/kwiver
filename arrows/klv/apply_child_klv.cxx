// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of apply_child_klv filter.

#include <arrows/klv/apply_child_klv.h>

#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1607.h>
#include <arrows/klv/klv_metadata.h>

#include <vital/logger/logger.h>
#include <vital/range/iterator_range.h>

#include <iterator>
#include <list>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_1607_child_policy
klv_0601_child_policy( klv_lds_key tag )
{
  if( klv_0601_traits_lookup().by_tag( tag ).tag_count_range().upper() > 1 )
  {
    return KLV_1607_CHILD_POLICY_KEEP_BOTH;
  }
  return KLV_1607_CHILD_POLICY_KEEP_CHILD;
}

// ----------------------------------------------------------------------------
void
apply_amend( klv_value& value ) {
  // Extract the local set
  auto const set_ptr = value.get_ptr< klv_local_set >();
  if( !set_ptr )
  {
    return;
  }
  auto& set = *set_ptr;

  // Find amend set(s)
  std::vector< klv_local_set > valid_amend_sets;
  auto const amend_range = set.all_at( KLV_0601_AMEND_LOCAL_SET );
  if( amend_range.size() > 1 )
  {
    LOG_WARN(
      vital::get_logger( "klv.apply_child_klv" ),
      "Multiple sibling amend sets found. In accordance with the MISP "
      "Handbook, only one can be applied. This algorithm will choose one "
      "arbitrarily, which is likely not the desired behavior. Use nested "
      "amend sets to describe multiple generations of amendments which can be "
      "applied in succession." );
  }

  // TODO: Add this code as a config option or delete it:
  // // Use this code to apply all sibling amend sets in arbitrary order,
  // // contra the MISP Handbook.
  // for( auto it = amend_range.begin();
  //      it != set.end() && it->first == KLV_0601_AMEND_LOCAL_SET; )
  // {
  //   apply_amend( it->second );
  //   auto const amend_ptr = it->second.get_ptr< klv_local_set >();
  //   if( amend_ptr )
  //   {
  //     valid_amend_sets.emplace_back( std::move( *amend_ptr ) );
  //     it = set.erase( it );
  //   }
  //   else
  //   {
  //     ++it;
  //   }
  // }

  // Find first valid amend set in any order, then remove all of them
  if( !amend_range.empty() )
  {
    while( valid_amend_sets.empty() )
    {
      auto& amend_value = amend_range.begin()->second;
      apply_amend( amend_value );
      auto const amend_ptr = amend_value.get_ptr< klv_local_set >();
      if( amend_ptr )
      {
        valid_amend_sets.emplace_back( std::move( *amend_ptr ) );
      }
    }
    set.erase( KLV_0601_AMEND_LOCAL_SET );
  }

  // Apply the chosen amend set(s)
  for( auto const& amend_set : valid_amend_sets )
  {
    klv_1607_apply_child( set, amend_set, klv_0601_child_policy );
  }
}

// ----------------------------------------------------------------------------
// Returns the range of sets created by applying segment sets.
vital::range::iterator_range< typename std::list< klv_packet >::iterator >
apply_segment(
  std::list< klv_packet >& packets,
  typename std::list< klv_packet >::iterator packet_it )
{
  // Extract the local set
  auto const set_ptr = packet_it->value.get_ptr< klv_local_set >();
  if( !set_ptr )
  {
    return { std::next( packet_it ), std::next( packet_it ) };
  }
  auto& set = *set_ptr;

  // Find segment set(s) and remove them
  std::vector< klv_local_set > segment_sets;
  auto const segment_range = set.all_at( KLV_0601_SEGMENT_LOCAL_SET );
  for( auto it = segment_range.begin();
        it != set.end() && it->first == KLV_0601_SEGMENT_LOCAL_SET; )
  {
    auto const segment_ptr = it->second.get_ptr< klv_local_set >();
    if( segment_ptr )
    {
      segment_sets.emplace_back( std::move( *segment_ptr ) );
      it = set.erase( it );
    }
    else
    {
      ++it;
    }
  }
  if( segment_sets.empty() )
  {
    return { std::next( packet_it ), std::next( packet_it ) };
  }

  // Apply segment sets to create new packets and erase the original one
  auto const original_packet_it = packet_it;
  for( auto const& segment_set : segment_sets )
  {
    packet_it =
      packets.insert(
        std::next( packet_it ), klv_packet{ packet_it->key, set } );
    klv_1607_apply_child(
      packet_it->value.get< klv_local_set >(), segment_set );
  }
  auto const begin_it = std::next( original_packet_it );
  auto const end_it = std::next( packet_it );
  packets.erase( original_packet_it );

  return { begin_it, end_it };
}

// ----------------------------------------------------------------------------
apply_child_klv
::apply_child_klv()
{}

// ----------------------------------------------------------------------------
apply_child_klv
::~apply_child_klv()
{}

// ----------------------------------------------------------------------------
vital::config_block_sptr
apply_child_klv
::get_configuration() const
{
  return algorithm::get_configuration();
}

// ----------------------------------------------------------------------------
void
apply_child_klv
::set_configuration( vital::config_block_sptr config )
{
  auto existing_config = algorithm::get_configuration();
  existing_config->merge_config( config );
}

// ----------------------------------------------------------------------------
bool
apply_child_klv
::check_configuration( [[maybe_unused]] vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
vital::metadata_vector
apply_child_klv
::filter(
  vital::metadata_vector const& input_metadata,
  [[maybe_unused]] vital::image_container_scptr const& input_image )
{
  vital::metadata_vector results;
  for( auto const& src_md : input_metadata )
  {
    // Case: null
    if( !src_md )
    {
      results.emplace_back( nullptr );
      continue;
    }

    // Deep copy. Not 100% sure this is necessary - future config option?
    results.emplace_back( src_md->clone() );

    // Case: no KLV
    auto const klv_md = dynamic_cast< klv_metadata* >( results.back().get() );
    if( !klv_md )
    {
      continue;
    }

    // This is a list instead of a vector for easy segment set insertion
    std::list< klv_packet > result_klv{
      klv_md->klv().begin(), klv_md->klv().end() };

    // Apply amend and segment sets
    for( auto it = result_klv.begin(); it != result_klv.end(); )
    {
      // Only ST0601 has segment / amend sets that I am aware of
      if( it->key != klv_0601_key() || !it->value.valid() )
      {
        ++it;
        continue;
      }

      // Amend function recurses internally
      apply_amend( it->value );

      // Segment function recurses using this outer loop
      it = apply_segment( result_klv, it ).begin();
    }

    // Move packets back to vector for storage
    klv_md->klv().assign(
      std::make_move_iterator( result_klv.begin() ),
      std::make_move_iterator( result_klv.end() ) );
  }

  return results;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
