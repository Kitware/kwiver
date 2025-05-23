// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of KLV IMAP encoding / decoding logic.

#ifndef KWIVER_ARROWS_KLV_KLV_IMAP_H_
#define KWIVER_ARROWS_KLV_KLV_IMAP_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_lengthy.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/util/interval.h>

#include <ostream>

#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_imap
{
public:
  /// Designates IMAP varieties.
  ///
  /// The order here determines sorting order.
  enum kind_t : uint8_t {
    KIND_USER_DEFINED,
    KIND_NAN_QUIET,
    KIND_NAN_SIGNALING,
    KIND_BELOW_MIN,
    KIND_NORMAL,
    KIND_ABOVE_MAX,
    KIND_ENUM_END,
  };

  /// Create an IMAP with a value of 0.
  klv_imap();

  /// Create an IMAP from a floating point number.
  ///
  /// \param value Value to store.
  ///
  /// This will accept NaN, but assumes it is quiet with a zero NaN id.
  explicit klv_imap( double value );

  /// Create an IMAP with a custom NaN value.
  ///
  /// \param is_signaling True if the value should be a signaling NaN.
  /// \param sign True if the value should be negative.
  /// \param nan_id Up to 59-bit value to encode in the NaN.
  static klv_imap nan(
    bool is_signaling, bool sign, uint64_t nan_id );

  /// Create an IMAP indicating a value below the allowed value range.
  static klv_imap below_minimum();

  /// Create an IMAP indicating a value above the allowed value range.
  static klv_imap above_maximum();

  /// Create an IMAP with a user-defined payload.
  ///
  /// \param payload Payload to encode. May be up to 59 bits long.
  static klv_imap user_defined( uint64_t payload );

  /// Return what kind of IMAP this is.
  kind_t kind() const;

  /// Return the closest possible floating point representation of this value.
  double as_double() const;

  /// Return any payload bits encoded with the value.
  uint64_t other_bits() const;

  /// Return the number of payload bits encoded with the value.
  uint8_t other_bits_count() const;

  bool operator<( klv_imap const& other ) const;
  bool operator>( klv_imap const& other ) const;
  bool operator<=( klv_imap const& other ) const;
  bool operator>=( klv_imap const& other ) const;
  bool operator==( klv_imap const& other ) const;
  bool operator!=( klv_imap const& other ) const;

private:
  klv_imap( double value, kind_t kind, uint64_t other_bits );

  double m_value;
  uint64_t m_other_bits;
  kind_t m_kind;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_imap const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream& operator<<( std::ostream& os, klv_imap::kind_t const& value );

// ----------------------------------------------------------------------------
/// Interprets data as a floating point value encoded in IMAP format.
class KWIVER_ALGO_KLV_EXPORT klv_imap_format
  : public klv_data_format_< klv_lengthy< klv_imap > >
{
public:
  klv_imap_format(
    vital::interval< double > const& interval,
    klv_length_constraints const& length_constraints = {} );

  std::string
  description_() const override;

  vital::interval< double >
  interval() const;

protected:
  klv_lengthy< klv_imap >
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_lengthy< klv_imap > const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_lengthy< klv_imap > const& value ) const override;

  std::ostream&
  print_typed( std::ostream& os,
               klv_lengthy< klv_imap > const& value ) const override;

  vital::interval< double > m_interval;
};

using klv_lengthless_imap_format = klv_lengthless_format< klv_imap_format >;

// ----------------------------------------------------------------------------
/// Read an IMAP-encoded floating-point value from a sequence of bytes.
///
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param interval Mapped range.
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param length Number of bytes to read.
///
/// \returns Number or special value decoded from \p data buffer.
///
/// \throws metadata_type_overflow When \p length is greater than the size of a
/// \c uint64_t, or the span of \p interval is too large for a \c double to
/// hold, or the result value would fall outside \p interval.
KWIVER_ALGO_KLV_EXPORT
klv_imap
klv_read_imap(
  vital::interval< double > const& interval,
  klv_read_iter_t& data, size_t length );

// ----------------------------------------------------------------------------
/// Write a floating-point value into the IMAP format.
///
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param value Number or special value to encode into IMAP.
/// \param interval Mapped range.
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param length Number of bytes to write. This determines the precision of
/// the encoded value.
///
/// \throws logic_error When \p length is zero.
/// \throws metadata_type_overflow When the span of \p interval is too large
/// for a \c double to hold.
KWIVER_ALGO_KLV_EXPORT
void
klv_write_imap( klv_imap value, vital::interval< double > const& interval,
                klv_write_iter_t& data, size_t length );

// ----------------------------------------------------------------------------
/// Return the number of bytes required for the given IMAP specification.
///
/// Precision here is the distance between successive discrete mapped values.
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param interval Mapped range.
/// \param precision Desired precision of IMAP value.
///
/// \returns Byte length of an IMAP value meeting the provided parameters.
///
/// \throws logic_error When \p precision is greater than the span of \p
/// interval, or is nonfinite.
/// \throws metadata_type_overflow When \p length is greater than the size of a
/// \c uint64_t or the span of \p interval is too large for a \c double to hold.
KWIVER_ALGO_KLV_EXPORT
size_t
klv_imap_length( vital::interval< double > const& interval, double precision );

// ----------------------------------------------------------------------------
/// Return the precision offered by the given IMAP specification.
///
/// Precision here is the distance between successive discrete mapped values.
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param interval Mapped range.
/// \param length Desired byte length of IMAP value.
///
/// \returns Precision of an IMAP value meeting the provided parameters.
///
/// \throws logic_error When \p length is zero.
/// \throws metadata_type_overflow When the span of \p interval is too large
/// for a \c double to hold.
KWIVER_ALGO_KLV_EXPORT
double
klv_imap_precision( vital::interval< double > const& interval, size_t length );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
