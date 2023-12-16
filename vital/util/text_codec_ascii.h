// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the ASCII text codec.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_ASCII_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_ASCII_H_

#include <vital/util/text_codec.h>
#include <vital/util/vital_util_export.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// ASCII codec.
///
/// This codec only allows strict 7-bit ASCII, not to be confused with 8-bit
/// ASCII supersets Windows-1252 or ANSEL.
class VITAL_UTIL_EXPORT text_codec_ascii : public text_codec
{
public:
  std::string name() const override;
  bool can_encode( char32_t c ) const override;

  std::tuple< result_code, char32_t const*, char* > encode(
    char32_t const* decoded_begin, char32_t const* decoded_end,
    char* encoded_begin, char* encoded_end ) const override;

  std::tuple< result_code, char const*, char32_t* > decode(
    char const* encoded_begin, char const* encoded_end,
    char32_t* decoded_begin, char32_t* decoded_end,
    bool has_true_end ) const override;
};

} // namespace vital

} // namespace kwiver

#endif
