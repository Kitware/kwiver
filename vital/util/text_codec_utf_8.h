// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the UTF-8 text codec.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_UTF_8_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_UTF_8_H_

#include <vital/util/text_codec.h>
#include <vital/util/vital_util_export.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// UTF-8 codec.
///
/// This codec does not accept nonstandard UTF-8, including overlong encodings.
class VITAL_UTIL_EXPORT text_codec_utf_8 : public text_codec
{
public:
  std::string name() const override;

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
