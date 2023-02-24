// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1303 parser's non-templated functions.

#include "klv_1303.hpp"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1303_apa value )
{
  static std::string strings[ KLV_1303_APA_ENUM_END ] = {
    "Unknown APA",
    "Natural",
    "IMAP",
    "Boolean",
    "Uint",
    "RLE" };

  return os << strings[ ( value >= KLV_1303_APA_ENUM_END )
                        ? KLV_1303_APA_UNKNOWN
                        : value ];
}

#define KLV_INSTANTIATE( T )                                                              \
  template struct klv_1303_mdap< T >;                                                     \
  template std::ostream& operator<< < T >( std::ostream&, klv_1303_mdap< T > const& );    \
  template bool operator< < T >( klv_1303_mdap< T > const&, klv_1303_mdap< T > const& );  \
  template bool operator> < T >( klv_1303_mdap< T > const&, klv_1303_mdap< T > const& );  \
  template bool operator<= < T >( klv_1303_mdap< T > const&, klv_1303_mdap< T > const& ); \
  template bool operator>= < T >( klv_1303_mdap< T > const&, klv_1303_mdap< T > const& ); \
  template bool operator!= < T >( klv_1303_mdap< T > const&, klv_1303_mdap< T > const& ); \
  template bool operator== < T >( klv_1303_mdap< T > const&, klv_1303_mdap< T > const& );

KLV_INSTANTIATE( bool );
KLV_INSTANTIATE( double );
KLV_INSTANTIATE( int64_t );
KLV_INSTANTIATE( std::string );
KLV_INSTANTIATE( uint64_t );

#undef KLV_INSTANTIATE

#define KLV_INSTANTIATE( FORMAT ) \
  template class klv_1303_mdap_format< FORMAT >;

KLV_INSTANTIATE( klv_ber_format );
KLV_INSTANTIATE( klv_ber_oid_format );
KLV_INSTANTIATE( klv_bool_format );
KLV_INSTANTIATE( klv_lengthless_format< klv_float_format > );
KLV_INSTANTIATE( klv_lengthless_format< klv_imap_format > );
KLV_INSTANTIATE( klv_lengthless_format< klv_sflint_format > );
KLV_INSTANTIATE( klv_sint_format );
KLV_INSTANTIATE( klv_string_format );
KLV_INSTANTIATE( klv_lengthless_format< klv_uflint_format > );
KLV_INSTANTIATE( klv_uint_format );

#undef KLV_INSTANTIATE


} // namespace klv

} // namespace arrows

} // namespace kwiver
