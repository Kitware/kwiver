/*ckwg +5
 * Copyright 2012-2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_ORACLE_API_TYPES_H
#define INCL_TRACK_ORACLE_API_TYPES_H

///
/// basic types used by the track oracle API
///

#include <vital/vital_config.h>
#include <track_oracle/track_oracle_export.h>

#include <iostream>
#include <vector>
#include <map>

namespace kwiver {
namespace track_oracle {

//
// conceptually, track oracle presents all the data as a large sparse
// matrix of elements.  Each column is a single type; a row is an
// instance of a data structure defined by the columns "picked up" by
// the row.
//

// the row == oracle_entry_handle_type
typedef size_t oracle_entry_handle_type;

// the column == field_handle_type
typedef size_t field_handle_type;

// flags for invalid rows and columns
const oracle_entry_handle_type INVALID_ROW_HANDLE = static_cast<oracle_entry_handle_type>( -1 );
const field_handle_type INVALID_FIELD_HANDLE = static_cast<field_handle_type>( -1 );

//
// The only structure track oracle imposes on the data is the abstract
// structure of a moving object track:
// -- track-level data (ID, other metadata such as labels)
// -- frame-level data (bounding boxes, timestamps, etc)
//
// track_handle_type and frame_handle_type are used to convey this
// distinction.  Functionally, they are identical.
//
// Of course, there's no reason track oracle couldn't store non-moving-object
// tracks; there probably should be a POD / non-track type.
//

struct TRACK_ORACLE_EXPORT track_handle_type
{
  oracle_entry_handle_type row;
  track_handle_type(): row( INVALID_ROW_HANDLE ) {}
  bool is_valid() const {return this->row != INVALID_ROW_HANDLE;}
  explicit track_handle_type( oracle_entry_handle_type r ): row(r) {}
};
bool TRACK_ORACLE_EXPORT operator==( const track_handle_type& lhs, const track_handle_type& rhs );
bool TRACK_ORACLE_EXPORT operator!=( const track_handle_type& lhs, const track_handle_type& rhs );
bool TRACK_ORACLE_EXPORT operator<( const track_handle_type& lhs, const track_handle_type& rhs );
std::ostream& TRACK_ORACLE_EXPORT operator<<( std::ostream& os, const track_handle_type& t );
std::istream& TRACK_ORACLE_EXPORT operator>>( std::istream& os, track_handle_type& t );

struct TRACK_ORACLE_EXPORT frame_handle_type
{
  oracle_entry_handle_type row;
  frame_handle_type(): row( INVALID_ROW_HANDLE ) {}
  bool is_valid() const {return this->row != INVALID_ROW_HANDLE;}
  explicit frame_handle_type( oracle_entry_handle_type r): row(r) {}
};
bool TRACK_ORACLE_EXPORT operator==( const frame_handle_type& lhs, const frame_handle_type& rhs );
bool TRACK_ORACLE_EXPORT operator!=( const frame_handle_type& lhs, const frame_handle_type& rhs );
bool TRACK_ORACLE_EXPORT operator<( const  frame_handle_type& lhs, const frame_handle_type& rhs );
std::ostream& TRACK_ORACLE_EXPORT operator<<( std::ostream& os, const frame_handle_type& f );
std::istream& TRACK_ORACLE_EXPORT operator>>( std::istream& os, frame_handle_type& f );

// lists of handles

typedef std::vector< track_handle_type> track_handle_list_type;
typedef std::vector< frame_handle_type> frame_handle_list_type;
typedef std::vector< oracle_entry_handle_type > handle_list_type;

///
/// domains are used to scope sets of tracks, to avoid requiring
/// a unique key (such as "track_id") across multiple sets of
/// data which may, in fact, not have unique keys.
///

typedef unsigned int domain_handle_type;
const domain_handle_type DOMAIN_ALL = 0;

///
/// When reading a CSV file, track_oracle's get_csv_handler_map()
/// will return an csv_handler_map_type based on the headers, which
/// does two things:
///
/// 1) the keys tell you which data elements are in the CSV
///
/// 2) the values tell you which indices in the CSV are associated
/// with the header.
///
/// Data types with multiple header entries (i.e. boxes) are checked
/// for header-level completeness; it's up to the type's reader
/// to check for completeness at the instance level.
///
/// Headers not corresponding to any recognized type (or to partially
/// complete types) are enumerated in the INVALID_FIELD slot.
///
/// The indices are returned in the order the headers are listed by
/// the type's csv_headers() method.
///

typedef std::vector< size_t > csv_header_index_type;
typedef std::map< field_handle_type, csv_header_index_type > csv_handler_map_type;
typedef std::map< field_handle_type, csv_header_index_type >::const_iterator csv_handler_map_cit;

} // ...track_oracle
} // ...kwiver

#endif
