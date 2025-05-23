// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1607 parser.

#include "klv_1607.h"

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_1607_child_set_format
::klv_1607_child_set_format( klv_tag_traits_lookup const& traits )
  : klv_local_set_format{ traits }
{}

// ----------------------------------------------------------------------------
std::string
klv_1607_child_set_format
::description_() const
{
  return "ST1607 Child LS";
}

// ----------------------------------------------------------------------------
void
klv_1607_child_set_format
::check_set( VITAL_UNUSED klv_local_set const& klv ) const
{
  // Do nothing - i.e. don't check tag counts since this isn't meant to be a
  // valid set on its own
}

// ----------------------------------------------------------------------------
void
klv_1607_apply_child(
  klv_local_set& parent, klv_local_set const& child,
  std::function< klv_1607_child_policy( klv_lds_key ) > const& policy_fn )
{
  // Two loops so that all entries are deleted from parent set before adding
  // any from the child set, in the unlikely case of multiple values per tag
  // in the child set
  for( auto const& entry : child )
  {
    auto const policy =
      policy_fn ? policy_fn( entry.first ) : KLV_1607_CHILD_POLICY_KEEP_CHILD;
    if( policy & KLV_1607_CHILD_POLICY_KEEP_PARENT )
    {
      continue;
    }

    if( parent.count( entry.first ) > 1 )
    {
      LOG_WARN( kv::get_logger( "klv" ), "apply_child: "
                "modifying tag which has multiple values in parent set" );
    }
    parent.erase( entry.first );
  }

  for( auto const& entry : child )
  {
    auto const policy =
      policy_fn ? policy_fn( entry.first ) : KLV_1607_CHILD_POLICY_KEEP_CHILD;
    if( policy & KLV_1607_CHILD_POLICY_KEEP_CHILD )
    {
      parent.add( entry.first, entry.second );
    }
  }
}

// ----------------------------------------------------------------------------
klv_local_set
klv_1607_derive_child( klv_local_set const& lhs, klv_local_set const& rhs )
{
  // Start out with the target
  klv_local_set result = rhs;

  // Add null entries for tags which go missing from lhs to rhs
  for( auto const& entry : lhs )
  {
    if( !result.count( entry.first ) )
    {
      result.add( entry.first, {} );
    }
  }

  // Remove tags that stay the same from lhs to rhs
  std::set< klv_lds_key > tags;
  for( auto const& entry : rhs )
  {
    tags.emplace( entry.first );
  }
  for( auto const tag : tags )
  {
    auto const lhs_range = lhs.all_at( tag );
    auto const rhs_range = rhs.all_at( tag );
    if( lhs_range.size() != rhs_range.size() )
    {
      continue;
    }

    // A few locally necessary utilities
    using range_t = typename klv_local_set::const_range;
    using iterator_t = typename klv_local_set::const_iterator;
    auto const entry_cmp =
      []( iterator_t lhs_it, iterator_t rhs_it ){
        return *lhs_it < *rhs_it;
      };
    auto const entry_eq =
      []( iterator_t lhs_it, iterator_t rhs_it ){
        return *lhs_it == *rhs_it;
      };
    auto const get_sorted_entries =
      [ &entry_cmp ]( range_t range ){
        std::vector< iterator_t > entries;
        for( auto it = range.begin(); it != range.end(); ++it )
        {
          entries.emplace_back( it );
        }
        std::sort( entries.begin(), entries.end(), entry_cmp );
        return entries;
      };

    // We have to sort the entries which have the same tag in order to compare
    // across sets
    auto const lhs_entries = get_sorted_entries( lhs_range );
    auto const rhs_entries = get_sorted_entries( rhs_range );
    if( std::equal( lhs_entries.begin(), lhs_entries.end(),
                    rhs_entries.begin(), entry_eq ) )
    {
      result.erase( tag );
    }
  }

  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
