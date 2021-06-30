// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Interface to abstract filter metadata algorithm
 */

#ifndef VITAL_ALGO_METADATA_FILTER_H
#define VITAL_ALGO_METADATA_FILTER_H

#include <vital/algo/algorithm.h>

#include <vital/types/image_container.h>
#include <vital/types/metadata.h>

#include <vital/vital_config.h>

namespace kwiver {

namespace vital {

namespace algo {

/// Abstract base class for metadata filter algorithms.
///
/// This interface supports arrows/algorithms that modify image metadata.
class VITAL_ALGO_EXPORT metadata_filter
  : public kwiver::vital::algorithm_def< metadata_filter >
{
public:
  /// Return the name of this algorithm.
  static std::string static_type_name() { return "metadata_filter"; }

  /// Return if this filter uses the image for filtering.
  ///
  /// This method returns whether the algorithm uses the image in its
  /// implementation. It may be more efficient to not obtain the image if the
  /// algorithm does not use it.
  virtual bool uses_image() const = 0;

  /// Filter metadata and return resulting metadata.
  ///
  /// This method implements the filtering operation. The method does not
  /// modify the metadata in place.
  ///
  /// \param input_metadata Metadata to filter.
  /// \param input_image Image associated with the metadata (may be null).
  ///
  /// \returns Filtered version of the input metadata.
  virtual kwiver::vital::metadata_vector filter(
    kwiver::vital::metadata_vector const& input_metadata,
    kwiver::vital::image_container_scptr const& input_image ) = 0;

protected:
  metadata_filter();
};

/// Type alias for shared pointer to a metadata_filter algorithm.
using metadata_filter_sptr = std::shared_ptr< metadata_filter >;

} // namespace algo

} // namespace vital

} // namespace kwiver

#endif
