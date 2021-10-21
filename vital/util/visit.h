// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Utility methods for visiting types at runtime.

// TODO(C++17): replace with std::variant
#include <vital/internal/variant/variant.hpp>
#include <vital/vital_config.h>

#include <map>
#include <typeindex>

namespace kwiver {

namespace vital {

// ----------------------------------------------------------------------------
/// Call \p visitor with template type parameter corresponding to \p type.
///
/// \param visitor Object with overloaded <code> template< class > operator()
/// </code>.
/// \param type Type to call \p visitor with.
/// \tparam Visitor Type of \p visitor.
/// \tparam Types Possible types supported by this function.
///
/// \throws out_of_range If \p type is not in \c Types.
template < class Visitor, class... Types >
void
visit_types( Visitor&& visitor, std::type_info const& type );

// ----------------------------------------------------------------------------
/// Call \p visitor with template type parameter corresponding to \p type.
///
/// \param visitor Object with overloaded <code> template< class > operator()
/// </code>.
/// \param type Type to call \p visitor with.
/// \tparam ReturnT Return type.
/// \tparam Visitor Type of \p visitor.
/// \tparam Types Possible types supported by this function.
///
/// \throws out_of_range If \p type is not in \c Types.
///
/// \returns Result of calling \p visitor with template parameter \p type,
/// converted to \c ReturnT.
template < class ReturnT, class Visitor, class... Types >
ReturnT
visit_types_return( Visitor&& visitor, std::type_info const& type );

namespace visit_detail {

// ----------------------------------------------------------------------------
template < class Visitor, class T >
void
invoke_visitor( Visitor&& visitor )
{
  visitor.template operator()< T >( );
}

// ----------------------------------------------------------------------------
template < class ReturnT, class Visitor, class T >
ReturnT
invoke_visitor_return( Visitor&& visitor )
{
  return static_cast< ReturnT >( visitor.template operator()< T >( ) );
}

// ----------------------------------------------------------------------------
template < class Visitor, class... Types >
void
visit_variant_types( Visitor&& visitor, std::type_info const& type,
                     VITAL_UNUSED variant< Types... > const* )
{
  return visit_types< Visitor, Types... >(
    std::forward< Visitor >( visitor ), type );
}

// ----------------------------------------------------------------------------
template < class ReturnT, class Visitor, class... Types >
ReturnT
visit_variant_types_return( Visitor&& visitor, std::type_info const& type,
                            VITAL_UNUSED variant< Types... > const* )
{
  return visit_types_return< ReturnT, Visitor, Types... >(
    std::forward< Visitor >( visitor ), type );
}

} // namespace visit_detail

// ----------------------------------------------------------------------------
template < class Visitor, class... Types >
void
visit_types( Visitor&& visitor, std::type_info const& type )
{
  using invoke_fn_t = void ( * )( Visitor&& );

  static std::map< std::type_index, invoke_fn_t > const map = {
    { typeid( Types ),
      &visit_detail::invoke_visitor< Visitor, Types > } ... };
  map.at( type )( std::forward< Visitor >( visitor ) );
}

// ----------------------------------------------------------------------------
template < class ReturnT, class Visitor, class... Types >
ReturnT
visit_types_return( Visitor&& visitor, std::type_info const& type )
{
  using invoke_fn_t = ReturnT ( * )( Visitor&& );

  static std::map< std::type_index, invoke_fn_t > const map = {
    { typeid( Types ),
      &visit_detail::invoke_visitor_return< ReturnT, Visitor, Types > } ... };
  return map.at( type )( std::forward< Visitor >( visitor ) );
}

// ----------------------------------------------------------------------------
/// Call \p visitor with template type parameter corresponding to \p type.
///
/// \param visitor Object with overloaded <code> template< class > operator()
/// </code>.
/// \param type Type to call \p visitor with.
/// \tparam Variant Variant type from which to extract the list of types
/// supported by this function.
/// \tparam Visitor Type of \p visitor.
///
/// \throws out_of_range If \p type is not one of \c Variant's supported types.
template < class Variant, class Visitor >
void
visit_variant_types( Visitor&& visitor, std::type_info const& type )
{
  visit_detail::visit_variant_types(
    std::forward< Visitor >( visitor ), type,
    static_cast< Variant const* >( nullptr ) );
}

// ----------------------------------------------------------------------------
/// Call \p visitor with template type parameter corresponding to \p type.
///
/// \param visitor Object with overloaded <code> template< class > operator()
/// </code>.
/// \param type Type to call \p visitor with.
/// \tparam ReturnT Return type.
/// \tparam Variant Variant type from which to extract the list of types
/// supported by this function.
/// \tparam Visitor Type of \p visitor.
///
/// \throws out_of_range If \p type is not one of \c Variant's supported types.
///
/// \returns Result of calling \p visitor with template parameter \p type,
/// converted to \c ReturnT.
template < class ReturnT, class Variant, class Visitor >
ReturnT
visit_variant_types_return( Visitor&& visitor,
                            std::type_info const& type )
{
  return visit_detail::visit_variant_types_return< ReturnT >(
    std::forward< Visitor >( visitor ), type,
    static_cast< Variant const* >( nullptr ) );
}

} // namespace vital

} // namespace kwiver
