// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV blob class.

#include "klv_blob.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_blob
::klv_blob()
{}

// ----------------------------------------------------------------------------
klv_blob
::klv_blob( klv_bytes_t const& bytes ) : bytes{ bytes }
{}

// ----------------------------------------------------------------------------
klv_bytes_t&
klv_blob
::operator*()
{
  return bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t const&
klv_blob
::operator*() const
{
  return bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t*
klv_blob
::operator->()
{
  return &bytes;
}

// ----------------------------------------------------------------------------
klv_bytes_t const*
klv_blob
::operator->() const
{
  return &bytes;
}

// ----------------------------------------------------------------------------
size_t
klv_blob_length( klv_blob const& value )
{
  return value->size();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
