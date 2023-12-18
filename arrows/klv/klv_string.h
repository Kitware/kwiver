// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV string data formats.

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/util/text_codec.h>

#ifndef KWIVER_ARROWS_KLV_KLV_STRING_H_
#define KWIVER_ARROWS_KLV_KLV_STRING_H_

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Interprets data as a string.
class KWIVER_ALGO_KLV_EXPORT klv_string_format
  : public klv_data_format_< std::string >
{
public:
  klv_string_format(
    vital::text_codec const& codec,
    klv_length_constraints const& char_constraints,
    klv_length_constraints const& byte_constraints );
  virtual ~klv_string_format();

  std::string description_() const override;

  vital::text_codec const& codec() const;

protected:
  std::string read_typed(
    klv_read_iter_t& data, size_t length ) const override;

  void write_typed(
    std::string const& value, klv_write_iter_t& data, size_t length )
  const override;

  size_t length_of_typed( std::string const& value ) const override;

  std::ostream& print_typed(
    std::ostream& os, std::string const& value ) const override;

  vital::text_codec const* m_codec;
  klv_length_constraints m_char_constraints;
};

// ----------------------------------------------------------------------------
/// Interprets data as an ASCII string.
class KWIVER_ALGO_KLV_EXPORT klv_ascii_format : public klv_string_format
{
public:
  klv_ascii_format( klv_length_constraints const& length_constraints = {} );
  virtual ~klv_ascii_format();
};

// ----------------------------------------------------------------------------
/// Interprets data as a UTF-8 string.
class KWIVER_ALGO_KLV_EXPORT klv_utf_8_format : public klv_string_format
{
public:
  klv_utf_8_format(
    klv_length_constraints const& char_constraints = {},
    klv_length_constraints const& byte_constraints = {} );
  virtual ~klv_utf_8_format();
};

// ----------------------------------------------------------------------------
/// Interprets data as a UTF-16 string.
///
/// The big-endian variation of UTF-16 is used, consistent with the encoding of
/// integers elsewhere in MISB KLV.
class KWIVER_ALGO_KLV_EXPORT klv_utf_16_format : public klv_string_format
{
public:
  klv_utf_16_format(
    klv_length_constraints const& char_constraints = {},
    klv_length_constraints const& byte_constraints = {} );
  virtual ~klv_utf_16_format();
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
