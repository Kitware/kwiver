// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the basic KLV read/write functions.

// This file currently handles the serialization / deserialization of these
// basic formats of KLV data:
// - int : General signed or unsigned integer of any integral byte length up to
// 8. Important to get this one precisely right because it's the base of most
// KLV data. Written MSB first.
// - BER : Unsigned integer which encodes its own length. Up to 127 is
// identical to standard int, otherwise the first byte encodes the number of
// following bytes.
// - BER-OID : Unsigned integer which encodes its own length. First bit of each
// byte signals whether there is another following byte; lower seven bits
// concatenated together form the actual value.
// - IMAP : Floating-point number between defined upper and lower limits,
// represented as an integer, the full range of which is uniformly mapped
// between those limits. Has special defined values for infinities, NaNs, etc.
// If zero is within the limits, the conversion maps one integral value to zero
// exactly. The number of bytes is variable and determines the precision of the
// mapping.
//
// The functions are templated to be able to handle any iterator type (pointer,
// std::vector, std::deque, etc), different native integer sizes (uint8_t ...
// uint64_t), and different KLV integer sizes (1 ... 8). This hopefully allows
// these functions to be reused in every KLV standard.

#ifndef KWIVER_ARROWS_KLV_KLV_READ_WRITE_H_
#define KWIVER_ARROWS_KLV_KLV_READ_WRITE_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <cstdlib>

namespace kwiver {

namespace arrows {

namespace klv {

// ---------------------------------------------------------------------------
/// Read an integer from a sequence of bytes (big-endian).
///
/// This function handles signed and unsigned integers as well as values of \p
/// length which are not powers of 2. It assumes there are \p length bytes
/// available in the source buffer pointed to by \p data.
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param length Number of bytes to read.
///
/// \returns Integer read in from \p data.
///
/// \throws metadata_type_overflow When \p length is greater than the number of
///         bytes in the return type \c T.
template < class T, class Iterator >
KWIVER_ALGO_KLV_EXPORT
T
klv_read_int( Iterator& data, size_t length );

// ---------------------------------------------------------------------------
/// Write an integer to a sequence of bytes (big-endian).
///
/// This function handles signed and unsigned integers as well as values of \p
/// length which are not powers of 2. Values of \p length which are greater
/// than necessary to represent \p value will result in zero padding on the
/// left. Assumes there are \p length bytes available in the destination buffer
/// pointed to by \p data.
///
/// \param value Integer to write.
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param length Number of bytes to write.
///
/// \throws metadata_type_overflow When \p value is too large to fit in \p
/// length bytes.
template < class T, class Iterator >
KWIVER_ALGO_KLV_EXPORT
void
klv_write_int( T value, Iterator& data, size_t length );

// ---------------------------------------------------------------------------
/// Return the number of bytes required to store the given signed or unsigned
/// integer.
///
/// \param value Integer whose byte length is being queried.
///
/// \returns Bytes required to store \p value.
template < class T >
KWIVER_ALGO_KLV_EXPORT
size_t
klv_int_length( T value );

// ---------------------------------------------------------------------------
/// Read an integer from a sequence of bytes, decoding it from BER format.
///
/// For an explanation of BER, see the MISB Motion Imagery Handbook, Section
/// 7.3.1.
/// \see
/// https://gwg.nga.mil/misb/docs/misp/MISP-2020.1_Motion_Imagery_Handbook.pdf
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to read.
///
/// \returns Integer decoded from \p data.
///
/// \throws metadata_buffer_overflow When decoding would require reading more
/// than \p max_length bytes.
/// \throws metadata_type_overflow When the decoded value is too large to fit
/// in the return type \c T.
template < class T, class Iterator >
KWIVER_ALGO_KLV_EXPORT
T
klv_read_ber( Iterator& data, size_t max_length );

// ---------------------------------------------------------------------------
/// Write an integer to a sequence of bytes, encoding it into BER format.
///
/// For an explanation of BER, see the MISB Motion Imagery Handbook, Section
/// 7.3.1.
/// \see
/// https://gwg.nga.mil/misb/docs/misp/MISP-2020.1_Motion_Imagery_Handbook.pdf
///
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to write.
///
/// \throws metadata_buffer_overflow When encoding would require writing more
/// than \p max_length bytes.
template < class T, class Iterator >
KWIVER_ALGO_KLV_EXPORT
void
klv_write_ber( T value, Iterator& data, size_t max_length );

// ---------------------------------------------------------------------------
/// Return the number of bytes required to store the given integer in BER
/// format.
///
/// \param value Integer whose byte length is being queried.
///
/// \returns Bytes required to store \p value in BER format.
template < class T >
KWIVER_ALGO_KLV_EXPORT
size_t
klv_ber_length( T value );

// ---------------------------------------------------------------------------
/// Read an integer from a sequence of bytes, decoding it from BER-OID format.
///
/// For an explanation of BER-OID, see the MISB Motion Imagery Handbook,
/// Section 7.3.2.
/// \see
/// https://gwg.nga.mil/misb/docs/misp/MISP-2020.1_Motion_Imagery_Handbook.pdf
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to read.
///
/// \returns Integer decoded from \p data.
///
/// \throws metadata_buffer_overflow When decoding would require reading more
/// than \p max_length bytes.
/// \throws metadata_type_overflow When the decoded value is too large to fit
/// in the return type \c T.
template < class T, class Iterator >
KWIVER_ALGO_KLV_EXPORT
T
klv_read_ber_oid( Iterator& data, size_t max_length );

// ---------------------------------------------------------------------------
/// Write an integer to a sequence of bytes, encoding it into BER-OID format.
///
/// For an explanation of BER-OID, see the MISB Motion Imagery Handbook,
/// Section 7.3.2.
/// \see
/// https://gwg.nga.mil/misb/docs/misp/MISP-2020.1_Motion_Imagery_Handbook.pdf
///
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to write.
///
/// \throws metadata_buffer_overflow When encoding would require writing more
/// than \p max_length bytes.
template < class T, class Iterator >
KWIVER_ALGO_KLV_EXPORT
void
klv_write_ber_oid( T value, Iterator& data, size_t max_length );

// ---------------------------------------------------------------------------
/// Return the number of bytes required to store the given integer in BER-OID
/// format.
///
/// For an explanation of BER-OID, see the MISB Motion Imagery Handbook,
/// Section 7.3.2.
/// \see
/// https://gwg.nga.mil/misb/docs/misp/MISP-2020.1_Motion_Imagery_Handbook.pdf
///
/// \param value Integer whose byte length is being queried.
///
/// \returns Bytes required to store \p value in BER format.
template < class T >
KWIVER_ALGO_KLV_EXPORT
size_t
klv_ber_oid_length( T value );

// ---------------------------------------------------------------------------
/// Read an IMAP-encoded floating-point value from a sequence of bytes.
///
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param minimum Lower end of mapped range.
/// \param maximum Upper end of mapped range.
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param length Number of bytes to read.
///
/// \returns Floating-point number decoded from \p data buffer.
///
/// \throws logic_error When \p minimum is not less than \p maximum, either is
/// nonfinite, or \p length is zero.
/// \throws metadata_type_overflow When \p length is greater than the size of a
/// \c uint64_t or the difference between \p minimum and \p maximum is to large
/// for a \c double to hold.
template < class Iterator >
KWIVER_ALGO_KLV_EXPORT
double
klv_read_imap( double minimum, double maximum, Iterator& data, size_t length );

// ---------------------------------------------------------------------------
/// Write a floating-point value into the IMAP format.
///
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param value Floating-point number to encode into IMAP.
/// \param minimum Lower end of mapped range.
/// \param maximum Upper end of mapped range.
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param length Number of bytes to write. This determines the precision of
/// the encoded value.
///
/// \throws logic_error When \p minimum is not less than \p maximum, either is
/// nonfinite, or \p length is zero.
/// \throws metadata_type_overflow When the difference between \p minimum and
/// \p maximum is to large for a \c double to hold.
template < class Iterator >
KWIVER_ALGO_KLV_EXPORT
void
klv_write_imap( double value, double minimum, double maximum, Iterator& data,
                size_t length );

// ---------------------------------------------------------------------------
/// Return the number of bytes required for the given IMAP specification.
///
/// Precision here is the distance between successive discrete mapped values.
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param minimum Lower end of mapped range.
/// \param maximum Upper end of mapped range.
/// \param precision Desired precision of IMAP value.
///
/// \returns Byte length of an IMAP value meeting the provided parameters.
///
/// \throws logic_error When \p minimum is not less than \p maximum, \p
/// precision is greater than their span, or any of the three is nonfinite.
/// \throws metadata_type_overflow When \p length is greater than the size of a
/// \c uint64_t or the difference between \p minimum and \p maximum is to large
/// for a \c double to hold.
KWIVER_ALGO_KLV_EXPORT
size_t
klv_imap_length( double minimum, double maximum, double precision );

// ---------------------------------------------------------------------------
/// Return the precision offered by the given IMAP specification.
///
/// Precision here is the distance between successive discrete mapped values.
/// For an explanation of IMAP, see the MISB ST1201 document.
/// \see https://gwg.nga.mil/misb/docs/standards/ST1201.4.pdf
///
/// \param minimum Lower end of mapped range.
/// \param maximum Upper end of mapped range.
/// \param length Desired byte length of IMAP value.
///
/// \returns Precision of an IMAP value meeting the provided parameters.
///
/// \throws logic_error When \p minimum is not less than \p maximum, either is
/// nonfinite, or \p length is zero.
/// \throws metadata_type_overflow When the difference between \p minimum and
/// \p maximum is to large for a \c double to hold.
KWIVER_ALGO_KLV_EXPORT
double
klv_imap_precision( double minimum, double maximum, size_t length );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
