// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of the KLV 0102 parser.

#include "klv_0102.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_uds_key
klv_0102_key()
{
  return { 0x060E2B3402030101, 0x0E01030302000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0102_traits_lookup()
{
  // Constants here are taken from
  // https://gwg.nga.mil/misb/docs/standards/ST0102.12.pdf
  // Descriptions are edited for clarity, brevity, consistency, etc.
  static klv_tag_traits_lookup const lookup = {
#define ENUM_AND_NAME( X ) X, #X
    { {},
      ENUM_AND_NAME( KLV_0102_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_SECURITY_CLASSIFICATION ),
      std::make_shared< klv_0102_security_classification_format >(),
      "Security Classification",
      "Overall security classification of the Motion Imagery in accordance "
      "with U.S. and NATO classification guidance.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0102_COUNTRY_CODING_METHOD ),
      std::make_shared< klv_0102_country_coding_method_format >(),
      "Country Coding Method for 'Classifying Country' and "
      "'Releasing Instructions'",
      "Method by which the classifying country and releasing instructions "
      "identify countries in text form. GENC administrative subdivision codes "
      "are not applicable.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0102_CLASSIFYING_COUNTRY ),
      std::make_shared< klv_string_format >(),
      "Classifying Country",
      "Country providing the security classification, preceded by '//'.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0102_SCI_SHI_INFORMATION ),
      std::make_shared< klv_string_format >(),
      "SCI / SHI Information",
      "Sensitive compartmented information or special handling instructions. "
      "Multiple digraphs, trigraphs, or compartment names are separated by "
      "the '/' character. Always ends in '//'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_CAVEATS ),
      std::make_shared< klv_string_format >(),
      "Caveats",
      "Pertinent caveats or code words from each category of the appropriate "
      "security entity register. May be abbreviated or spelled out.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_RELEASING_INSTRUCTIONS ),
      std::make_shared< klv_string_format >(),
      "Releasing Instructions",
      "List of country codes, separated by blank spaces, indicating the "
      "countries to which the Motion Imagery is releasable.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_CLASSIFIED_BY ),
      std::make_shared< klv_string_format >(),
      "Classified By",
      "Name and type of authority used to classify the Motion Imagery.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_DERIVED_FROM ),
      std::make_shared< klv_string_format >(),
      "Derived From",
      "Information about the original source of data from which "
      "classification was derived.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_CLASSIFICATION_REASON ),
      std::make_shared< klv_string_format >(),
      "Classification Reason",
      "Reason for classification of the Motion Imagery, or citation from a "
      "document.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_DECLASSIFICATION_DATE ),
      std::make_shared< klv_string_format >( 8 ),
      "Declassification Date",
      "Date when the classified material may be automatically declassified",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_CLASSIFICATION_AND_MARKING_SYSTEM ),
      std::make_shared< klv_string_format >(),
      "Classification and Marking System",
      "Classification or marking system used in this set as determined by the "
      "appropriate security entity for the country originating the data.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_OBJECT_COUNTRY_CODING_METHOD ),
      std::make_shared< klv_0102_country_coding_method_format >(),
      "Country Coding Method for 'Object Country Codes'",
      "Method by which the country which is the object of the Motion Imagery "
      "is identified in text form.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0102_OBJECT_COUNTRY_CODES ),
      std::make_shared< klv_string_format >(),
      "Object Country Codes",
      "Country or countries which are the object of the Motion Imagery, "
      "separated the ';' character.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0102_CLASSIFICATION_COMMENTS ),
      std::make_shared< klv_string_format >(),
      "Classification Comments",
      "Security related comments and future format changes.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_UMID_VIDEO ),
      std::make_shared< klv_blob_format >( 32 ),
      "UMID Video",
      "Deprecated. SMPTE RP210 32-byte identifier for the video stream.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_UMID_AUDIO ),
      std::make_shared< klv_blob_format >( 32 ),
      "UMID Audio",
      "Deprecated. SMPTE RP210 32-byte identifier for the audio stream.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_UMID_DATA ),
      std::make_shared< klv_blob_format >( 32 ),
      "UMID Data",
      "Deprecated. SMPTE RP210 32-byte identifier for the data stream.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_UMID_SYSTEM ),
      std::make_shared< klv_blob_format >( 32 ),
      "UMID System",
      "Deprecated. SMPTE RP210 32-byte identifier for the MI system.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_STREAM_ID ),
      std::make_shared< klv_uint_format >( 1 ),
      "Stream ID",
      "Deprecated. Any valid value specifying the Elementary Stream.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_TRANSPORT_STREAM_ID ),
      std::make_shared< klv_uint_format >( 2 ),
      "Transport Stream ID",
      "Deprecated. Value defined by the originator uniquely identifying a "
      "Transport Stream in a network environment.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_ITEM_DESIGNATOR_ID ),
      std::make_shared< klv_blob_format >( 16 ),
      "Item Designator ID",
      "Deprecated. 16-byte Universal Label Key for the element, set, or pack "
      "to which this set is linked.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0102_VERSION ),
      std::make_shared< klv_uint_format >( 2 ),
      "Version",
      "Version number of MISB ST 0102 used to encode this set.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0102_COUNTRY_CODING_METHOD_VERSION_DATE ),
      std::make_shared< klv_string_format >( 10 ),
      "Country Coding Method for 'Classifying Country' and "
      "'Releasing Instructions' Version Date",
      "Effective date of the source standard defining the country coding "
      "method used for the 'Classifying Country' and "
      "'Releasing Instructions' fields.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0102_OBJECT_COUNTRY_CODING_METHOD_VERSION_DATE ),
      std::make_shared< klv_string_format >( 10 ),
      "Country Coding Method for 'Object Country Codes' Version Date",
      "Effective date of the source standard defining the country coding "
      "method used for the 'Object Country Codes' field.",
      { 0, 1 } },
#undef ENUM_AND_NAME
  };

  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0102_tag tag )
{
  return os << klv_0102_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0102_security_classification value )
{
  static std::string const
  strings[ KLV_0102_SECURITY_CLASSIFICATION_ENUM_END ] = {
    "Unknown Security Classification",
    "Unclassified",
    "Restricted",
    "Confidential",
    "Secret",
    "Top Secret", };

  auto const index =
    ( value < KLV_0102_SECURITY_CLASSIFICATION_ENUM_END )
    ? value
    : KLV_0102_SECURITY_CLASSIFICATION_UNKNOWN;
  return os << strings[ index ];
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0102_country_coding_method value )
{
  static std::map< klv_0102_country_coding_method,
                   std::string > const strings = {
    { KLV_0102_COUNTRY_CODING_METHOD_UNKNOWN,
      "Unknown Country Coding Method" },
    { KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_TWO_LETTER,
      "ISO-3166 Two Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_THREE_LETTER,
      "ISO-3166 Three Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_FIPS_10_4_TWO_LETTER,
      "FIPS 10-4 Two Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_FIPS_10_4_FOUR_LETTER,
      "FIPS 10-4 Four Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_NUMERIC,
      "ISO-3166 Numeric" },
    { KLV_0102_COUNTRY_CODING_METHOD_1059_TWO_LETTER,
      "1059 Two Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_1059_THREE_LETTER,
      "1059 Three Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_FIPS_10_4_MIXED,
      "FIPS 10-4 Mixed" },
    { KLV_0102_COUNTRY_CODING_METHOD_ISO_3166_MIXED,
      "ISO-3166 Mixed" },
    { KLV_0102_COUNTRY_CODING_METHOD_STANAG_1059_MIXED,
      "STANAG-1059 Mixed" },
    { KLV_0102_COUNTRY_CODING_METHOD_GENC_TWO_LETTER,
      "GENC Two Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_GENC_THREE_LETTER,
      "GENC Three Letter" },
    { KLV_0102_COUNTRY_CODING_METHOD_GENC_NUMERIC,
      "GENC Numeric" },
    { KLV_0102_COUNTRY_CODING_METHOD_GENC_MIXED,
      "GENC Mixed" },
    { KLV_0102_COUNTRY_CODING_METHOD_GENC_ADMIN_SUB,
      "GENC AdminSub" }, };

  auto const it = strings.find( value );
  auto const string =
    ( it != strings.end() )
    ? it->second
    : strings.at( KLV_0102_COUNTRY_CODING_METHOD_UNKNOWN );
  return os << string;
}

// ----------------------------------------------------------------------------
klv_0102_local_set_format
::klv_0102_local_set_format()
  : klv_local_set_format{ klv_0102_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0102_local_set_format
::description() const
{
  return "security local set of " + length_description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
