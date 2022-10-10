// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VObject local set parser.

#include "klv_0903_vobject_set.h"

#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vobject_set_tag tag )
{
  return os << klv_0903_vobject_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_0903_vobject_local_set_format
::klv_0903_vobject_local_set_format()
  : klv_local_set_format{ klv_0903_vobject_set_traits_lookup() } {}

// ----------------------------------------------------------------------------
std::string
klv_0903_vobject_local_set_format
::description() const
{
  return "vobject local set of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vobject_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VOBJECT_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VOBJECT_ONTOLOGY ),
      std::make_shared< klv_string_format >(),
      "Ontology",
      "URI referring to a vObject ontology.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VOBJECT_ONTOLOGY_CLASS ),
      std::make_shared< klv_string_format >(),
      "Ontology Class",
      "Value representing a target class or type, as defined by the Ontology tag.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VOBJECT_ONTOLOGY_ID ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Ontology ID",
      "Identifier for an ontology in the VMTI Ontology Series.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VOBJECT_CONFIDENCE ),
      std::make_shared< klv_imap_format >(
        vital::interval< double >{ 0.0, 100.0 },
        klv_length_constraints{ 1, 3 } ),
      "Confidence",
      "Level of confidence in the classification of the object.",
      { 0, 1 } } };

  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
