// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declaration of KLV timeline class.

#ifndef KWIVER_ARROWS_KLV_KLV_TIMELINE_H_
#define KWIVER_ARROWS_KLV_KLV_TIMELINE_H_

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <vital/util/interval_map.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Tracks the value of KLV fields over time.
///
/// This class is the final intended product of KLV parsing, efficiently
/// holding data from all standards over any period of time. Enough information
/// is stored to validly (though not necessarily identically) re-encode the KLV
/// stream.
class KWIVER_ALGO_KLV_EXPORT klv_timeline
{
public:
  struct key_t
  {
    klv_top_level_tag standard;
    klv_lds_key tag;
    uint64_t index;

    bool operator<( key_t const& other ) const;
  };

  using interval_t = kwiver::vital::interval< uint64_t >;
  using interval_map_t = kwiver::vital::interval_map< uint64_t, klv_value >;
  using container_t = std::map< key_t, interval_map_t >;
  using iterator = typename container_t::iterator;
  using const_iterator = typename container_t::const_iterator;
  using range = kwiver::vital::range::iterator_range< iterator >;
  using const_range = kwiver::vital::range::iterator_range< const_iterator >;

  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end() const;

  const_iterator cbegin() const;
  const_iterator cend() const;

  /// Return the value at the given location, or an empty \c klv_value if no
  /// value exists.
  ///
  /// \throws logic_error If more than one value is present.
  klv_value at( klv_top_level_tag standard, klv_lds_key tag,
                uint64_t time ) const;

  /// Return the value at the given location, or an empty \c klv_value if no
  /// value exists.
  klv_value at( klv_top_level_tag standard, klv_lds_key tag,
                uint64_t index, uint64_t time ) const;

  /// Return all values at the given location.
  std::vector< klv_value > all_at( klv_top_level_tag standard, klv_lds_key tag,
                                   uint64_t time ) const;

  /// Return the range of all entries using \p standard.
  range find_all( klv_top_level_tag standard );

  /// \copydoc range find_all( klv_top_level_tag standard )
  const_range find_all( klv_top_level_tag standard ) const;

  /// Return the range of all entries using \p standard and \p tag.
  range find_all( klv_top_level_tag standard, klv_lds_key tag );

  /// \copydoc range find_all( klv_top_level_tag, klv_lds_key );
  const_range find_all( klv_top_level_tag standard, klv_lds_key tag ) const;

  /// Return an iterator to the given location, or the end iterator if none
  /// exists.
  ///
  /// \throws logic_error If more than one entry is present at the given
  /// location.
  iterator find( klv_top_level_tag standard, klv_lds_key tag );

  /// \copydoc iterator find( klv_top_level_tag, klv_lds_key )
  const_iterator find( klv_top_level_tag standard, klv_lds_key tag ) const;

  /// Return an iterator to the given location, or the end iterator if none
  /// exists.
  iterator find( klv_top_level_tag standard, klv_lds_key tag, uint64_t index );

  /// \copydoc iterator find( klv_top_level_tag, klv_lds_key )
  const_iterator find( klv_top_level_tag standard, klv_lds_key tag,
                       uint64_t index ) const;

  /// Insert a new timeline, even if one already exists.
  ///
  /// \note The index of the inserted timeline is not guaranteed to be any
  /// particular number.
  ///
  /// \returns An iterator to the new timeline.
  iterator insert( klv_top_level_tag standard, klv_lds_key tag );

  /// Return an iterator to the given location, creating a new timeline if none
  /// exists.
  ///
  /// \throws logic_error If more than one entry is present at the given
  /// location.
  iterator insert_or_find( klv_top_level_tag standard, klv_lds_key tag );

  /// Return an iterator to the given location, creating a new timeline if none
  /// exists.
  iterator insert_or_find( klv_top_level_tag standard, klv_lds_key tag,
                           uint64_t index );

private:
  container_t m_map;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
