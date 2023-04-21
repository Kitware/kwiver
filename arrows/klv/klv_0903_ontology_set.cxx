// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 ontology local set parser.

#include <arrows/klv/klv_0903_ontology_set.h>

#include <arrows/klv/klv_string.h>

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
      std::make_shared< klv_uint_format >(),
      "ID",
      "Identifier for the ontology used.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_PARENT_ID ),
      std::make_shared< klv_uint_format >(),
      "Parent ID",
      "Defines a link between two related ontology local sets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_IRI ),
      std::make_shared< klv_utf_8_format >(),
      "IRI",
      "Internationalized Resource Identifier identifying the ontology.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_ENTITY ),
      std::make_shared< klv_utf_8_format >(),
      "Entity",
      "IRI identifying an entity within the ontology.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_VERSION ),
      std::make_shared< klv_utf_8_format >(),
      "Version",
      "IRI identifying the version of the ontology.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_LABEL ),
      std::make_shared< klv_utf_8_format >(),
      "Label",
      "Name of the entity, as defined by the ontology.",
      { 0, 1 } } };

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
::description_() const
{
  return "ST0903 Ontology LS";
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
