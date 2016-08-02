/*ckwg +5
 * Copyright 2012-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_FILE_FORMAT_CALLOUT_H
#define INCL_FILE_FORMAT_CALLOUT_H

#include <vital/vital_config.h>
#include <track_oracle/track_e2at_callout/track_e2at_callout_export.h>

#include <track_oracle/file_format_base.h>
#include <track_oracle/track_e2at_callout/track_e2at_callout.h>

namespace kwiver {
namespace track_oracle {

class TRACK_E2AT_CALLOUT_EXPORT file_format_e2at_callout: public file_format_base
{
public:
  file_format_e2at_callout(): file_format_base( TF_E2AT_CALLOUT, "E2AT callouts (CSV)" )
  {
    this->globs.push_back( "*.csv" );
  }
  virtual ~file_format_e2at_callout() {}

  virtual int supported_operations() const { return FF_READ; }

  // return a dynamically-allocated instance of the schema
  virtual track_base_impl* schema_instance() const { return new track_e2at_callout_type(); }

  // Inspect the file and return true if it is of this format
  virtual bool inspect_file( const std::string& fn ) const;

  using file_format_base::read;
  // read tracks from the file
  virtual bool read( const std::string& fn,
                     track_handle_list_type& tracks ) const;

  // read tracks from the file
  virtual bool read( std::istream& is,
                     track_handle_list_type& tracks ) const;

};

} // ...track_oracle
} // ...kwiver

#endif
