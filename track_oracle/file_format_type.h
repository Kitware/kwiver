/*ckwg +5
 * Copyright 2012-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_FILE_FORMAT_TYPE_H
#define INCL_FILE_FORMAT_TYPE_H

#include <vital/vital_config.h>
#include <track_oracle/track_oracle_format_base_export.h>

#include <string>

///
/// This header file lists the full set of file formats understood
/// by track oracle.
///
/// It's broken out into its own file so that it can be referenced
/// by both the file_format_manager (which needs to link against
/// all the file formats) and the file_format_schema (which doesn't.)
/// For example, the tracking library's convert-xml-to-event routines
/// need to read kwxml, but don't need all the rest of it.
///
/// Eventually, we'll start selectively compiling in file types based
/// on public/private projects; when we do, the actual values of fields
/// in this enumeration will be configuration dependent.  So it's important
/// that any serialization rely on the text string of the file formats
/// and then locally convert that to the (source-code-invariant, compile-time-
/// changing) enumeration value.
///
/// This header file also defines the static methods responsible for
/// enum <-> string conversion (a job previously held by the file
/// format manager, which is too far up the class hierarchy to allow
/// convenient linkage.)
///

namespace kwiver {
namespace track_oracle {

enum file_format_enum
{
  TF_BEGIN,
  TF_KW18 = TF_BEGIN,
  TF_XGTF,
  TF_KWXML,
  TF_APIX,
  TF_MITRE_BOX_XML,
  TF_COMMS_XML,
  TF_KST,
  TF_VATIC,
  TF_VPD_TRACK,
  TF_VPD_EVENT,
  TF_E2AT_CALLOUT,
  TF_4676,
  TF_CSV,
  TF_KWIVER,
  TF_INVALID_TYPE   // must always be last entry
};

struct TRACK_ORACLE_FORMAT_BASE_EXPORT file_format_type
{
  static std::string to_string( file_format_enum f );
  static file_format_enum from_string( const std::string& s );
};

} // ...track_oracle
} // ...kwiver

#endif
