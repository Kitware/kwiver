// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of standard text codec error policies.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_ERROR_POLICIES_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_ERROR_POLICIES_H_

#include <vital/util/singleton.h>
#include <vital/util/text_codec.h>
#include <vital/util/vital_util_export.h>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// When a character cannot be encoded, skip it.
class VITAL_UTIL_EXPORT text_codec_encode_error_policy_skip
  : public text_codec::encode_error_policy,
    public singleton< text_codec_encode_error_policy_skip >
{
public:
  std::tuple< result_code, char* > handle(
    text_codec const& codec, char32_t c,
    char* begin, char* end ) const override;
};

// ----------------------------------------------------------------------------
/// When a character cannot be encoded, stop encoding.
class VITAL_UTIL_EXPORT text_codec_encode_error_policy_abort
  : public text_codec::encode_error_policy,
    public singleton< text_codec_encode_error_policy_abort >
{
public:
  std::tuple< result_code, char* > handle(
    text_codec const& codec, char32_t c,
    char* begin, char* end ) const override;
};

// ----------------------------------------------------------------------------
/// When a character cannot be encoded, insert a 'substitute' character.
class VITAL_UTIL_EXPORT text_codec_encode_error_policy_substitute
  : public text_codec::encode_error_policy,
    public singleton< text_codec_encode_error_policy_substitute >
{
public:
  std::tuple< result_code, char* > handle(
    text_codec const& codec, char32_t c,
    char* begin, char* end ) const override;
};

// ----------------------------------------------------------------------------
/// When a character cannot be encoded, write it as an escaped string '\uXXXX'
/// or '\UXXXXXXXX'.
class VITAL_UTIL_EXPORT text_codec_encode_error_policy_unicode_escape
  : public text_codec::encode_error_policy,
    public singleton< text_codec_encode_error_policy_unicode_escape >
{
public:
  std::tuple< result_code, char* > handle(
    text_codec const& codec, char32_t c,
    char* begin, char* end ) const override;
};

// ----------------------------------------------------------------------------
/// When a character cannot be decoded, skip it.
class VITAL_UTIL_EXPORT text_codec_decode_error_policy_skip
  : public text_codec::decode_error_policy,
    public singleton< text_codec_decode_error_policy_skip >
{
public:
  std::tuple< result_code, char32_t* > handle(
    text_codec const& codec, char32_t* begin, char32_t* end ) const override;
};

// ----------------------------------------------------------------------------
/// When a character cannot be decoded, stop decoding.
class VITAL_UTIL_EXPORT text_codec_decode_error_policy_abort
  : public text_codec::decode_error_policy,
    public singleton< text_codec_decode_error_policy_abort >
{
public:
  std::tuple< result_code, char32_t* > handle(
    text_codec const& codec, char32_t* begin, char32_t* end ) const override;
};

// ----------------------------------------------------------------------------
/// When a character cannot be decoded, read it as a 'substitute' character.
class VITAL_UTIL_EXPORT text_codec_decode_error_policy_substitute
  : public text_codec::decode_error_policy,
    public singleton< text_codec_decode_error_policy_substitute >
{
public:
  std::tuple< result_code, char32_t* > handle(
    text_codec const& codec, char32_t* begin, char32_t* end ) const override;
};

} // namespace vital

} // namespace kwiver

#endif
