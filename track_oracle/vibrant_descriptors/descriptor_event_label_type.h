/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_DESCRIPTOR_EVENT_LABEL_TYPE_H
#define INCL_DESCRIPTOR_EVENT_LABEL_TYPE_H

///
/// Actually, this doesn't show up in the xml as a descriptor node (oversight!)
/// But the code paths are the same.
///

#include <vital/vital_config.h>
#include <track_oracle/vibrant_descriptors/vibrant_descriptors_export.h>

#include <iostream>
#include <string>
#include <vector>

namespace kwiver {
namespace track_oracle {

struct VIBRANT_DESCRIPTORS_EXPORT single_event_label_type
{
  std::string activity_name;
  double spatial_overlap;
  double temporal_overlap;
  single_event_label_type():
    activity_name(""),
    spatial_overlap(-1.0),
    temporal_overlap(-1.0)
  {}
  bool operator==( const single_event_label_type& rhs ) const
  {
    return
      ( this->activity_name == rhs.activity_name ) &&
      ( this->spatial_overlap == rhs.spatial_overlap ) &&
      ( this->temporal_overlap == rhs.temporal_overlap );
  }
};

struct VIBRANT_DESCRIPTORS_EXPORT descriptor_event_label_type
{
  std::string domain;
  std::vector< single_event_label_type > labels;
  descriptor_event_label_type():
    domain("")
  {}
  bool operator==( const descriptor_event_label_type& rhs ) const
  {
    //
    // Are two instances equal if their label sets are permutations
    // of each other?  We'll say no, because the main client of this
    // will be element_store's map, which really is looking at identity
    // and not semantic equivalence.
    //
    if ( this->domain != rhs.domain ) return false;
    if ( this->labels.size() != rhs.labels.size() ) return false;
    for (size_t i=0; i<this->labels.size(); ++i)
    {
      if ( ! ( this->labels[i] == rhs.labels[i] )) return false;
    }
    return true;
  }
};

std::ostream& VIBRANT_DESCRIPTORS_EXPORT operator<<( std::ostream& os, const descriptor_event_label_type& d );
std::istream& VIBRANT_DESCRIPTORS_EXPORT operator>>( std::istream& is, descriptor_event_label_type& d );

} // ...track_oracle
} // ...kwiver

#endif
