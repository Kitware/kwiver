// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV set template class.

#ifndef KWIVER_VITAL_KLV_KLV_SET_H_
#define KWIVER_VITAL_KLV_KLV_SET_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_tag_traits.h>
#include <arrows/klv/klv_value.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/range/iterator_range.h>

#include <map>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Base class for KLV universal and local sets.
///
/// This class is mostly a wrapper around a std::multimap, but with a slightly
/// modified interface. Usually, there will be only one entry for a key, so we
/// include the at() and find() methods here - otherwise, extracting a single
/// item from a multimap and verifying its singularity is awkward. Another
/// issue addressed here is that multimaps only sort by key, not value.
/// Therefore, multimaps { A : 1, A : 2 } and { A : 2, A : 1 } evaluate as not
/// equal. This class treats those sets as equal.
template < class Key >
class KWIVER_ALGO_KLV_EXPORT klv_set
{
public:
  using container = std::multimap< Key, klv_value >;
  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;
  using value_type = typename container::value_type;
  using range = kwiver::vital::range::iterator_range< iterator >;
  using const_range = kwiver::vital::range::iterator_range< const_iterator >;

  klv_set();

  klv_set( container const& items );

  klv_set( std::initializer_list< value_type > const& items );

  iterator
  begin();

  const_iterator
  begin() const;

  const_iterator
  cbegin() const;

  iterator
  end();

  const_iterator
  end() const;

  const_iterator
  cend() const;

  bool
  empty() const;

  size_t
  size() const;

  size_t
  count( Key const& key ) const;

  bool
  has( Key const& key ) const;

  void
  add( Key const& key, klv_value const& datum );

  void
  erase( const_iterator it );

  void
  erase( Key const& key );

  void
  clear();

  /// Return single entry corresponding to \p key, or end iterator on failure.
  iterator
  find( Key const& key );

  /// \copydoc iterator find( Key const& key )
  const_iterator
  find( Key const& key ) const;

  /// Return single value corresponding to \p key.
  ///
  /// \throws out_of_range If no \p key entry is present.
  /// \throws logic_error If more than one \p key entry is present.
  klv_value&
  at( Key const& key );

  /// \copydoc klv_value& at( Key const& key )
  klv_value const&
  at( Key const& key ) const;

  /// Return the range of entries corresponding to \p key.
  ///
  /// \note Order of entries returned is not defined.
  range
  all_at( Key const& key );

  /// \copydoc range all_at( Key const& key )
  const_range
  all_at( Key const& key ) const;

  /// Returns iterators to all entries, sorted by key, then by value.
  std::vector< const_iterator >
  fully_sorted() const;

  template < class K >
  friend bool
  operator==( klv_set< K > const& lhs, klv_set< K > const& rhs );

  template < class K >
  friend bool
  operator<( klv_set< K > const& lhs, klv_set< K > const& rhs );

  template < class K >
  friend std::ostream&
  operator<<( std::ostream& os, klv_set< K > const& rhs );

private:
  // Sort by key, then value.
  static bool
  value_compare( const_iterator lhs, const_iterator rhs );

  std::multimap< Key, klv_value > m_items;
};

// ----------------------------------------------------------------------------
/// Interprets data as a local or universal set.
template < class Key >
class KWIVER_ALGO_KLV_EXPORT klv_set_format
  : public klv_data_format_< klv_set< Key > >
{
public:
  explicit
  klv_set_format( klv_tag_traits_lookup const& traits );

  virtual
  ~klv_set_format();

protected:
  klv_set< Key >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_set< Key > const& klv,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_set< Key > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os, klv_set< Key > const& value ) const;

  // Print warnings if tags appear too few or too many times in the given set.
  void
  check_tag_counts( klv_set< Key > const& klv ) const;

  virtual void
  check_set( klv_set< Key > const& klv ) const;

  klv_tag_traits_lookup const& m_traits;
};

// ----------------------------------------------------------------------------
/// KLV local set. Key-value pairs of a format defined by a standard.
using klv_local_set = klv_set< klv_lds_key >;

// ----------------------------------------------------------------------------
/// Interprets data as a KLV local set.
using klv_local_set_format = klv_set_format< klv_lds_key >;

// ----------------------------------------------------------------------------
/// KLV universal set. Key-value pairs of a format defined by a standard.
using klv_universal_set = klv_set< klv_uds_key >;

// ----------------------------------------------------------------------------
/// Interprets data as a KLV universal set.
using klv_universal_set_format = klv_set_format< klv_uds_key >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
