// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of settings structure for the creation of a klv stream.

#ifndef KWIVER_ARROWS_KLV_KLV_STREAM_SETTINGS_H_
#define KWIVER_ARROWS_KLV_KLV_STREAM_SETTINGS_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Synchronicity of a KLV stream.
enum klv_stream_type {
  KLV_STREAM_TYPE_SYNC,
  KLV_STREAM_TYPE_ASYNC
};

// ----------------------------------------------------------------------------
/// Parameters describing the general characteristics of a KLV stream.
///
/// Members have been left public so users may modify them at their disgression.
struct KWIVER_ALGO_KLV_EXPORT klv_stream_settings {
  klv_stream_settings();

  /// Whether this stream is synchronous or asynchronous.
  klv_stream_type type;

  /// Index of this stream in the input file. May not determine the index in
  /// an output file.
  int index;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool operator==( klv_stream_settings const& lhs,
                 klv_stream_settings const& rhs );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
