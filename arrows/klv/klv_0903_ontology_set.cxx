// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 ontology local set parser.

#include "klv_0903_ontology_set.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_ontology_set_tag tag )
{
  return os << klv_0903_ontology_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_ontology_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_ID ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "ID",
      "Identifier for the ontology used.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_PARENT_ID ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Parent ID",
      "Defines a link between two related ontology local sets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_URI ),
      std::make_shared< klv_string_format >(),
      "URI",
      "Uniform Resource Identifier according to the OWL Web Ontology "
      "Language.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_CLASS ),
      std::make_shared< klv_string_format >(),
      "Class",
      "Target class or type, as defined by the ontology.",
      1 } };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_ontology_local_set_format
::klv_0903_ontology_local_set_format()
  : klv_local_set_format{ klv_0903_ontology_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_ontology_local_set_format
::description() const
{
  return "ontology local set of " + m_length_constraints.description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
