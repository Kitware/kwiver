// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV blob class.

#ifndef KWIVER_ARROWS_KLV_KLV_BLOB_H_
#define KWIVER_ARROWS_KLV_KLV_BLOB_H_

#include <arrows/klv/klv_types.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <initializer_list>
#include <ostream>
#include <vector>

#include <cstdint>
#include <cstdlib>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Structure to hold explicitly uninterpreted bytes.
///
/// This wrapper type is used to signify that the bytes it holds were unable to
/// be parsed, likely due to an unsupported field or irrecoverably incorrect
/// formatting. Unparsed bytes are still stored, however, to potentially write
/// them back out later.
class KWIVER_ALGO_KLV_EXPORT klv_blob
{
public:
  klv_blob();

  klv_blob( std::initializer_list< uint8_t > const& bytes );

  klv_blob( klv_bytes_t const& bytes );

  // Operator overloads to access internal vector-of-bytes
  klv_bytes_t& operator*();
  klv_bytes_t const& operator*() const;

  // Operator overloads to access methods of internal vector-of-bytes
  klv_bytes_t* operator->();
  klv_bytes_t const* operator->() const;

private:
  klv_bytes_t bytes;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_blob const& blob );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_blob const& lhs, klv_blob const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_blob const& lhs, klv_blob const& rhs );

// While simple in implementation, these utility functions are included for
// consistency with the rest of the KLV read / write API.

// ----------------------------------------------------------------------------
/// Load a sequence of bytes into a \c klv_blob structure.
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to read.
///
/// \returns A blob of \p length bytes from \p data.
KWIVER_ALGO_KLV_EXPORT
klv_blob
klv_read_blob( klv_read_iter_t& data, size_t length );

// ----------------------------------------------------------------------------
/// Write a \c klv_blob structure to sequence of bytes.
///
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to write.
///
/// \throws metadata_buffer_overflow When encoding would require writing more
/// than \p max_length bytes.
KWIVER_ALGO_KLV_EXPORT
void
klv_write_blob(
  klv_blob const& value, klv_write_iter_t& data, size_t max_length );

// ----------------------------------------------------------------------------
/// Return the number of bytes required to write the given blob.
///
/// \param value Blob whose byte length is being queried.
///
/// \returns Bytes required to write \p value.
KWIVER_ALGO_KLV_EXPORT
size_t
klv_blob_length( klv_blob const& value );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
