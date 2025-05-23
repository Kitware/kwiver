// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the UTF-16 text codecs.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_UTF_16_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_UTF_16_H_

#include <vital/util/text_codec.h>
#include <vital/util/text_codec_endian.h>
#include <vital/util/vital_util_export.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Endian-agnostic UTF-16 codec. Not directly instantiable.
///
/// This codec does not accept nonstandard UTF-16.
class VITAL_UTIL_EXPORT text_codec_utf_16
  : public text_codec,
    virtual public text_codec_16
{
public:
  std::tuple< result_code, char32_t const*, char* > encode(
    char32_t const* decoded_begin, char32_t const* decoded_end,
    char* encoded_begin, char* encoded_end ) const override;

  std::tuple< result_code, char const*, char32_t* > decode(
    char const* encoded_begin, char const* encoded_end,
    char32_t* decoded_begin, char32_t* decoded_end,
    bool has_true_end ) const override;
};

// ----------------------------------------------------------------------------
/// Big-endian UTF-16 codec.
class VITAL_UTIL_EXPORT text_codec_utf_16_be
  : public text_codec_utf_16,
    public text_codec_16_be
{
public:
  std::string name() const override;
};

// ----------------------------------------------------------------------------
/// Little-endian UTF-16 codec.
class VITAL_UTIL_EXPORT text_codec_utf_16_le
  : public text_codec_utf_16,
    public text_codec_16_le
{
public:
  std::string name() const override;
};

} // namespace vital

} // namespace kwiver

#endif
