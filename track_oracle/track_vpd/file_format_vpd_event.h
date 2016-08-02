/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_FILE_FORMAT_VPD_EVENT_H
#define INCL_FILE_FORMAT_VPD_EVENT_H

#include <vital/vital_config.h>
#include <track_oracle/track_vpd/track_vpd_export.h>

#include <track_oracle/file_format_base.h>
#include <track_oracle/track_vpd/track_vpd_event.h>

namespace kwiver {
namespace track_oracle {

///
/// In order to accomodate multi-track events, the VIRAT Public Data
/// format puts overall event information in an events.txt file and
/// links the event to objects in a separate mappings.txt file.  This
/// reader assumes we can find the mappings.txt file is in the same
/// directory as the events.txt file.  It also makes no effort to
/// verify that the objects referred have actually been loaded.


class TRACK_VPD_EXPORT file_format_vpd_event: public file_format_base
{
public:
  file_format_vpd_event(): file_format_base( TF_VPD_EVENT, "VIRAT Public Data 2.0 event" )
  {
    this->globs.push_back( "*.viratdata.events.txt" );
  }
  virtual ~file_format_vpd_event() {}

  virtual int supported_operations() const { return FF_READ_FILE; }

  // return a dynamically-allocated instance of the schema
  virtual track_base_impl* schema_instance() const { return new track_vpd_event_type(); }

  // Inspect the file and return true if it is of this format
  // (also checks that mapping file exists)
  virtual bool inspect_file( const std::string& fn ) const;

  // read tracks from the file-- no stream-only because
  // we need to get the mapping filename from the event filename
  virtual bool read( const std::string& event_fn,
                     track_handle_list_type& events ) const;
private:
  using file_format_base::read;
};

} // ...track_oracle
} // ...kwiver

#endif
