// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1607 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1607_H_
#define KWIVER_ARROWS_KLV_KLV_1607_H_

#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
///
enum klv_1607_child_policy
{
  KLV_1607_CHILD_POLICY_KEEP_NEITHER = 0,
  KLV_1607_CHILD_POLICY_KEEP_CHILD   = ( 1 << 0 ),
  KLV_1607_CHILD_POLICY_KEEP_PARENT  = ( 1 << 1 ),
  KLV_1607_CHILD_POLICY_KEEP_BOTH    = KLV_1607_CHILD_POLICY_KEEP_CHILD |
                                       KLV_1607_CHILD_POLICY_KEEP_PARENT,
};

using klv_1607_child_policy_fn =
  std::function< klv_1607_child_policy( klv_lds_key ) >;

// ----------------------------------------------------------------------------
/// Interprets data as a KLV ST1607 amend or segment local set.
class KWIVER_ALGO_KLV_EXPORT klv_1607_child_set_format
  : public klv_local_set_format
{
public:
  klv_1607_child_set_format( klv_tag_traits_lookup const& traits );

  std::string
  description_() const override;

private:
  void
  check_set( klv_local_set const& klv ) const override;
};

// ----------------------------------------------------------------------------
/// Return a copy of \p parent, overridden with any entries in \p child.
///
/// \param parent Local set to serve as the base.
/// \param child Local set which overrides values in \p parent.
///
/// \return Modified \p parent.
KWIVER_ALGO_KLV_EXPORT
void
klv_1607_apply_child(
  klv_local_set& parent, klv_local_set const& child,
  klv_1607_child_policy_fn const& policy_fn = nullptr );

// ----------------------------------------------------------------------------
/// Produce a 'diff' between two local sets in the form of a child local set.
///
/// The result is the set of entries in \p rhs which are not in \p lhs, along
/// with an empty entry for each entry in \p lhs which is not in \p rhs.
///
/// \param lhs Local set serving as the base / parent.
/// \param rhs Local set serving as the target of the application of the child.
///
/// \return The resultant child set.
KWIVER_ALGO_KLV_EXPORT
klv_local_set
klv_1607_derive_child( klv_local_set const& lhs, klv_local_set const& rhs );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
