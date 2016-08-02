/*ckwg +5
 * Copyright 2014-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef INCL_TRACK_TMP_UTILS_H
#define INCL_TRACK_TMP_UTILS_H

#include <vital/vital_config.h>
#include <track_oracle/data_terms/data_terms_export.h>

//
// These are some template metaprogramming (TMP) routines to support
// mixing new-style data_terms with old-style data_field (type, name)
// definitions

#include <string>

namespace kwiver {
namespace track_oracle {

//
//
//


struct data_term_base
{
};


//
// value is true if the template parameter is derived from
// data_term_base
//

template< typename T >
struct is_data_term
{
  typedef class dummy{ char dummy_vals[2]; } yes_type;
  typedef char no_type;

  static yes_type test( data_term_base* );

  static no_type test( ... );

  static T* PhonyMakeT();

public:
  static const bool value = ( sizeof(test( PhonyMakeT() )) == sizeof(yes_type));
};


//
// data_term_traits exposes the type and name, either from the data
// term or the supplied type and name
//
// The first approach almost worked but failed for vector<T>:
// ...template< typename T > struct foo<T, true>...
// ...template< typename T > struct foo<T, false> ...
//
// After reviewing ModernC++Design, went with
// ...template< bool, typename T > struct {...  // 'true'
// ...template< typename T> struct  <false, T> {... // 'false'
//
// Not 100% sure what was going on there yet.
//

template< bool T_is_a_delegate, typename T >
struct data_term_traits
{
  typedef typename T::Type Type;
  static std::string get_name() { return T::c.name; }
};

template< typename T >
struct data_term_traits< false, T >
{
  typedef T Type;
  static std::string get_name() { return ""; }
};

} // ...track_oracle
} // ...kwiver

#endif
