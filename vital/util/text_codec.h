// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of the text codec interface.

#ifndef KWIVER_VITAL_UTIL_TEXT_CODEC_H_
#define KWIVER_VITAL_UTIL_TEXT_CODEC_H_

#include <vital/util/vital_util_export.h>

#include <string>
#include <tuple>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Method of translating Unicode code points to and from a \c char sequence.
///
/// A \c char sequence is used instead of e.g. \c uint8_t to preserve trivial
/// compatibility with \c std::string. Codecs that work in words larger than a
/// byte, e.g. UTF-16, must render their words into byte form, making big- and
/// little-endian versions of such codecs distinct from one another. Unicode
/// code points are represented by \c char32_t.
class VITAL_UTIL_EXPORT text_codec
{
public:
  /// Indicates the ending state of an encoding or decoding operation.
  enum result_code
  {
    /// All input was successfully en/decoded.
    DONE,

    /// The output buffer does not have enough room to en/decode the next code
    /// point.
    OUT_OF_SPACE,

    /// Invalid data has been encountered and the error policy has decided to
    /// stop encoding.
    ABORT,
  };

  /// What to do when a code point cannot be encoded by the codec.
  class encode_error_policy
  {
  public:
    using result_code = text_codec::result_code;

    /// Respond to unsupported code point \p c.
    ///
    /// \param codec The codec which does not support \p c.
    /// \param c The unsupported code point.
    /// \param begin Beginning of output range.
    /// \param end End of output range.
    ///
    /// \return (result code, pointer to end of data written).
    virtual std::tuple< result_code, char* > handle(
      text_codec const& codec, char32_t c, char* begin, char* end ) const = 0;
  };

  /// What to do when a \c char sequence does not correspond to any supported
  /// code point.
  class decode_error_policy
  {
  public:
    using result_code = text_codec::result_code;

    /// Respond to an invalid \c char sequence.
    ///
    /// \param codec The codec encountering the invalid \c char sequence.
    /// \param begin Beginning of output range.
    /// \param end End of output range.
    ///
    /// \return (result code, pointer to end of data written).
    ///
    /// \warning Decode policies may not write more than one code point to the
    /// output range.
    virtual std::tuple< result_code, char32_t* > handle(
      text_codec const& codec, char32_t* begin, char32_t* end ) const = 0;
  };

  text_codec();
  virtual ~text_codec();

  /// Return the ASCII name of this codec.
  virtual std::string name() const = 0;

  /// Return \c true if the codec can encode code point \p c without error.
  ///
  /// The base class version of this function returns \c true for all Unicode
  /// code points (minus surrogates).
  virtual bool can_encode( char32_t c ) const;

  /// Return \c true if the codec can encode the string without error.
  bool can_encode( char32_t const* begin, char32_t const* end ) const;

  /// Return \c true if the codec can encode \p s without error.
  bool can_encode( std::u32string const& s ) const;

  /// Translate a sequence of code points to a sequence of \c char.
  ///
  /// As much data as possible is processed before returning, but only in whole
  /// units of one code point. That is, no partial representations of code
  /// points are ever written.
  ///
  /// \param decoded_begin Beginning of code point range to read.
  /// \param decoded_end End of code point range to read.
  /// \param encoded_begin Beginning of \c char range to write to.
  /// \param encoded_end End of \c char range to write to.
  ///
  /// \return
  ///   (result, pointer to end of data read, pointer to end of data written).
  virtual std::tuple< result_code, char32_t const*, char* > encode(
    char32_t const* decoded_begin, char32_t const* decoded_end,
    char* encoded_begin, char* encoded_end ) const = 0;

  /// Translate \p s to a \c std::string.
  std::tuple< result_code, std::string > encode(
    std::u32string const& s ) const;

  /// Return the number of \c char required to encode \p c.
  virtual std::tuple< result_code, size_t > encoded_size( char32_t c ) const;

  /// Return the number of \c char required to encode the code points between
  /// \p begin and \p end.
  virtual std::tuple< result_code, size_t > encoded_size(
    char32_t const* begin, char32_t const* end ) const;

  /// Return the number of \c char required to encode \c s.
  std::tuple< result_code, size_t > encoded_size(
    std::u32string const& s ) const;

  /// Translate a sequence of \c char to a sequence of code points.
  ///
  /// As much data as possible is processed before returning. The algorithm
  /// cannot require more than a distance of 1 between \c decoded_begin and
  /// \c decoded_end, even in the case of an error.
  ///
  /// \param encoded_begin Beginning of \c char range to read.
  /// \param encoded_end End of \c char range to read.
  /// \param decoded_begin Beginning of code point range to write to.
  /// \param decoded_end End of code point range to write to.
  /// \param has_true_end Whether \p encoded_end is truly the end of the input
  /// text data. This is important for multibyte encodings, since if the data
  /// is being buffered and sent through the codec piecemeal, there may be a
  /// truncated code point at the end of this piece which should not be read
  /// until the next piece completes it. However, if this is truly the end of
  /// the text data, that last partial code point should be passed to the error
  /// policy, since it is invalid.
  ///
  /// \return
  ///   (result, pointer to end of data read, pointer to end of data written).
  virtual std::tuple< result_code, char const*, char32_t* > decode(
    char const* encoded_begin, char const* encoded_end,
    char32_t* decoded_begin, char32_t* decoded_end,
    bool has_true_end ) const = 0;

  /// Translate \p s to a sequence of code points.
  std::tuple< result_code, std::u32string > decode(
    std::string const& s ) const;

  /// Return the number of code points encoded between \p begin and \p end.
  virtual std::tuple< result_code, size_t > decoded_size(
    char const* begin, char const* end, bool has_true_end ) const;

  /// Return the number of code points encoded in \p s.
  std::tuple< result_code, size_t > decoded_size( std::string const& s ) const;

  /// Set how this codec should respond to unsupported code points.
  void set_encode_error_policy( encode_error_policy const& policy );

  /// Set how this codec should respond to invalid \c char sequences.
  void set_decode_error_policy( decode_error_policy const& policy );

  static encode_error_policy const& default_encode_error_policy();
  static decode_error_policy const& default_decode_error_policy();

protected:
  encode_error_policy const* m_encode_error_policy;
  decode_error_policy const* m_decode_error_policy;
};

} // namespace vital

} // namespace kwiver

#endif
