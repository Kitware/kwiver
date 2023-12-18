// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of text transcoding capabilities.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_TRANSCODE_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_TRANSCODE_H_

#include <vital/range/iterator_range.h>
#include <vital/util/text_codec.h>
#include <vital/util/vital_util_export.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Provides efficient translation from one text encoding to another.
///
/// This must be a class rather than a function because the input is decoded in
/// chunks before being encoded again. Since the output buffer is not
/// guaranteed to be able to hold the entire chunk, the remainder must be
/// stored for future calls to \c transcode().
class VITAL_UTIL_EXPORT text_transcoder
{
public:
  /// \param src_codec Codec with which the input data will be encoded.
  /// \param dst_codec Codec with which the output data should be encoded.
  text_transcoder( text_codec const& src_codec, text_codec const& dst_codec );

  text_transcoder( text_transcoder const& other );
  text_transcoder( text_transcoder&& other );

  text_transcoder&
  operator=( text_transcoder const& other );
  text_transcoder&
  operator=( text_transcoder&& other );

  /// Write as much output data as possible, reading from input as needed.
  ///
  /// \param src_begin Beginning of input data.
  /// \param src_end End of input data.
  /// \param dst_begin Beginning of output data.
  /// \param dst_end End of output data.
  /// \param has_true_end See \c text_codec::decode().
  ///
  /// \return (result code, pointer to end of data read,
  ///          pointer to end of data written).
  ///
  /// \warning
  ///   This transcoder buffers data internally, so the returned read pointer
  ///   may be arbitrarily far ahead of the write pointer.
  std::tuple< text_codec::result_code, char const*, char* >
  transcode(
    char const* src_begin, char const* src_end,
    char* dst_begin, char* dst_end, bool has_true_end );

  /// Wipe internal buffers, preparing for the start of new input.
  text_transcoder& clear();

private:
  text_codec const* m_src_codec;
  text_codec const* m_dst_codec;
  char32_t m_buffer[ BUFSIZ ];
  char32_t* m_buffer_begin;
  char32_t* m_buffer_end;
};

// ----------------------------------------------------------------------------
/// Translate \p s from \p src_codec to \p dst_codec.
///
/// \param src_codec Codec with which the input data is encoded.
/// \param dst_codec Codec with which the output data should be encoded.
/// \param s Input string, encoded with \p src_codec.
///
/// \return (result code, transcoded string)
VITAL_UTIL_EXPORT
std::tuple< text_codec::result_code, std::string >
text_codec_transcode(
  text_codec const& src_codec, text_codec const& dst_codec,
  std::string const& s );

// ----------------------------------------------------------------------------
/// Return the number of \c char produced when transcoding the string from
/// \p src_codec to \p dst_codec.
VITAL_UTIL_EXPORT
std::tuple< text_codec::result_code, size_t >
text_codec_transcoded_size(
  text_codec const& src_codec, text_codec const& dst_codec,
  char const* begin, char const* end, bool has_true_end );

// ----------------------------------------------------------------------------
/// Return the number of \c char produced when transcoding \p s from
/// \p src_codec to \p dst_codec.
VITAL_UTIL_EXPORT
std::tuple< text_codec::result_code, size_t >
text_codec_transcoded_size(
  text_codec const& src_codec, text_codec const& dst_codec,
  std::string const& s );

} // namespace vital

} // namespace kwiver

#endif
