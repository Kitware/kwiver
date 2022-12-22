// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Declaration of internal STANAG utility functions.

#ifndef KWIVER_ARROWS_STANAG_STANAG_UTIL_H_
#define KWIVER_ARROWS_STANAG_STANAG_UTIL_H_

#include <arrows/stanag/kwiver_algo_stanag_export.h>

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_read_write.txx>
#include <arrows/klv/klv_util.h>

#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <variant>

#include <cstddef>

namespace klv = kwiver::arrows::klv;

namespace kwiver {

namespace arrows {

namespace stanag {

using ptr_t = uint8_t const*;

namespace klv = kwiver::arrows::klv;

// Redefine some statements from klv_util.h in order to change the EXPORT
// ----------------------------------------------------------------------------
#define DECLARE_STANAG_CMP( T )                                      \
        KWIVER_ALGO_STANAG_EXPORT bool operator<( T const&, T const& ); \
        KWIVER_ALGO_STANAG_EXPORT bool operator>( T const&, T const& ); \
        KWIVER_ALGO_STANAG_EXPORT bool operator<=( T const&, T const& ); \
        KWIVER_ALGO_STANAG_EXPORT bool operator>=( T const&, T const& ); \
        KWIVER_ALGO_STANAG_EXPORT bool operator==( T const&, T const& );

// ----------------------------------------------------------------------------
#define DECLARE_STANAG_TEMPLATE_CMP( TYPE )                                                    \
        template < class T > KWIVER_ALGO_STANAG_EXPORT bool operator<( \
          TYPE const&, TYPE const& ); \
        template < class T > KWIVER_ALGO_STANAG_EXPORT bool operator>( \
          TYPE const&, TYPE const& ); \
        template < class T > KWIVER_ALGO_STANAG_EXPORT bool operator<=( \
          TYPE const&, TYPE const& ); \
        template < class T > KWIVER_ALGO_STANAG_EXPORT bool operator>=( \
          TYPE const&, TYPE const& ); \
        template < class T > KWIVER_ALGO_STANAG_EXPORT bool operator==( \
          TYPE const&, TYPE const& );

// ----------------------------------------------------------------------------
#define DEFINE_STANAG_STRUCT_CMP( T, ... )                                                          \
        bool \
        operator<( T const& lhs, T const& rhs ) { return klv::struct_lt( lhs, \
                                                                         rhs, \
                                                                         __VA_ARGS__ ); \
        } \
        bool \
        operator>( T const& lhs, T const& rhs ) { return klv::struct_gt( lhs, \
                                                                         rhs, \
                                                                         __VA_ARGS__ ); \
        } \
        bool \
        operator<=( T const& lhs, T const& rhs ) { return klv::struct_le( lhs, \
                                                                          rhs, \
                                                                          __VA_ARGS__ ); \
        } \
        bool \
        operator>=( T const& lhs, T const& rhs ) { return klv::struct_ge( lhs, \
                                                                          rhs, \
                                                                          __VA_ARGS__ ); \
        } \
        bool \
        operator==( T const& lhs, T const& rhs ) { return klv::struct_eq( lhs, \
                                                                          rhs, \
                                                                          __VA_ARGS__ ); \
        }

// ----------------------------------------------------------------------------
#define DEFINE_STANAG_TEMPLATE_CMP( TYPE, ... )                                                                               \
        template < class T > bool \
        operator<( TYPE const& lhs, TYPE const& rhs ) { return klv::struct_lt( \
                                                          lhs, rhs, \
                                                          __VA_ARGS__ ); } \
        template < class T > bool \
        operator>( TYPE const& lhs, TYPE const& rhs ) { return klv::struct_gt( \
                                                          lhs, rhs, \
                                                          __VA_ARGS__ ); } \
        template < class T > bool \
        operator<=( TYPE const& lhs, TYPE const& rhs ) { return klv::struct_le( \
                                                           lhs, rhs, \
                                                           __VA_ARGS__ ); } \
        template < class T > bool \
        operator>=( TYPE const& lhs, TYPE const& rhs ) { return klv::struct_ge( \
                                                           lhs, rhs, \
                                                           __VA_ARGS__ ); } \
        template < class T > bool \
        operator==( TYPE const& lhs, TYPE const& rhs ) { return klv::struct_eq( \
                                                           lhs, rhs, \
                                                           __VA_ARGS__ ); }

// ----------------------------------------------------------------------------
#define DEFINE_STANAG_STRUCT_CMP_TUPLIZE( T )                                                     \
        bool \
        operator<( T const& lhs, \
                   T const& rhs ) { return tuplize( lhs ) <  tuplize( rhs ); } \
        bool \
        operator>( T const& lhs, \
                   T const& rhs ) { return tuplize( lhs ) >  tuplize( rhs ); } \
        bool \
        operator<=( T const& lhs, \
                    T const& rhs ) { return tuplize( lhs ) <= tuplize( rhs ); } \
        bool \
        operator>=( T const& lhs, \
                    T const& rhs ) { return tuplize( lhs ) >= tuplize( rhs ); } \
        bool \
        operator==( T const& lhs, \
                    T const& rhs ) { return tuplize( lhs ) == tuplize( rhs ); }

// ----------------------------------------------------------------------------
/// Trim leading and traling spaces from strings
KWIVER_ALGO_STANAG_EXPORT
std::string
trim_whitespace( std::string input );

} // namespace stanag

} // namespace arrows

} // namespace kwiver

#endif
