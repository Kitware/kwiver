// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Template utility functions for writing length-value pairs.

#ifndef KWIVER_ARROWS_KLV_KLV_LENGTH_VALUE_H_
#define KWIVER_ARROWS_KLV_KLV_LENGTH_VALUE_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/optional.h>

#include <tuple>

namespace kwiver {

namespace arrows {

namespace klv {

namespace lv_detail {

// ----------------------------------------------------------------------------
// Recursively copies all but the first element of a reference tuple.
template < size_t n, class... Ts >
struct pop_tuple_helper
{
  using tuple_t = std::tuple< Ts... >;
  using element_t = typename std::tuple_element< n, tuple_t >::type;
  decltype( std::tuple_cat(
    std::declval< pop_tuple_helper< n - 1, Ts... > >()(),
    std::declval< std::tuple< element_t > >() ) )
  operator()() const
  {
    auto const element = std::tuple< element_t >( std::get< n >( source ) );
    auto const helper = pop_tuple_helper< n - 1, Ts... >{ source };
    auto const remaining = helper();
    return std::tuple_cat( remaining, element );
  }

  std::tuple< Ts... > const& source;
};

// ----------------------------------------------------------------------------
// Base case - don't copy the first element.
template < class... Ts >
struct pop_tuple_helper< 0, Ts... >
{
  std::tuple< >
  operator()() const
  {
    return std::make_tuple();
  }

  std::tuple< Ts... > const& source;
};

// ----------------------------------------------------------------------------
// Return a copy of the given tuple with the first element removed.
template < class T, class... Ts >
std::tuple< Ts... >
pop_tuple( std::tuple< T, Ts... > const& value )
{
  auto const helper = pop_tuple_helper< sizeof...( Ts ), T, Ts... >{ value };
  return helper();
}

// ----------------------------------------------------------------------------
// Base case - single-element tuple.
template < class Format >
void
write_trunc_lv_impl(
  std::tuple<
    kwiver::vital::optional< typename Format::data_type > const&
    > const& value,
  klv_write_iter_t& data, size_t length,
  Format const& format )
{
  auto const& item = std::get< 0 >( value );
  if( item )
  {
    klv_write_lv( *item, data, length, format );
  }
}

// ----------------------------------------------------------------------------
// Internal version of write_trunc_lv where we have a guarantee that `length`
// is the exact length, and not just a maximum length.
template < class Format, class... Formats,
           typename std::enable_if< ( sizeof...( Formats ) > 0 ),
                                    bool >::type = true >
void
write_trunc_lv_impl(
  std::tuple< kwiver::vital::optional< typename Format::data_type > const&,
              kwiver::vital::optional< typename Formats::data_type > const&...
              > const& value,
  klv_write_iter_t& data, size_t length,
  Format format, Formats const&... formats )
{
  auto const tracker = track_it( data, length );
  auto const& item = std::get< 0 >( value );
  klv_write_opt_lv( item, data, tracker.remaining(), format );
  if( tracker.remaining() )
  {
    auto const remaining = pop_tuple( value );
    write_trunc_lv_impl( remaining, data, tracker.remaining(), formats... );
  }
}

} // namespace lv_detail

// ----------------------------------------------------------------------------
// Read in a value, defined by `format`, preceded by its BER-encoded length.
// Both `max_length` and the actual length should be greater than 0.
template < class Format >
typename Format::data_type
klv_read_lv(
  klv_read_iter_t& data, size_t max_length,
  Format const& format )
{
  auto const tracker = track_it( data, max_length );
  auto const length = klv_read_ber< size_t >( data, tracker.remaining() );
  auto const value = format.read_( data, tracker.verify( length ) );
  return value;
}

// ----------------------------------------------------------------------------
// Read in a value, defined by `format`, preceded by its BER-encoded length.
// If the length is 0, returns `nullopt`. `max_length` should be
/// greater than 0.
template < class Format >
kwiver::vital::optional< typename Format::data_type >
klv_read_opt_lv(
  klv_read_iter_t& data, size_t max_length,
  Format const& format )
{
  auto const tracker = track_it( data, max_length );
  auto const length = klv_read_ber< size_t >( data, tracker.remaining() );
  if( !length )
  {
    return kwiver::vital::nullopt;
  }

  auto const value = format.read_( data, tracker.verify( length ) );
  return value;
}

// ----------------------------------------------------------------------------
// Read in a value, defined by `format`, preceded by its BER-encoded length.
// If `max_length` or the actual length is 0, returns `nullopt`.
template < class Format >
kwiver::vital::optional< typename Format::data_type >
klv_read_trunc_lv(
  klv_read_iter_t& data, size_t max_length,
  Format const& format )
{
  return max_length
         ? klv_read_opt_lv( data, max_length, format )
         : kwiver::vital::nullopt;
}

// ----------------------------------------------------------------------------
// Write a value, defined by `format`, preceded by its BER-encoded length.
// `max_length` should be greater than 0.
template < class Format >
void
klv_write_lv(
  typename Format::data_type const& value,
  klv_write_iter_t& data, size_t max_length,
  Format const& format )
{
  auto const tracker = track_it( data, max_length );
  auto const length = format.length_of_( value );
  klv_write_ber( length, data, tracker.remaining() );
  format.write_( value, data, tracker.verify( length ) );
}

// ----------------------------------------------------------------------------
// Write a value, defined by `format`, preceded by its BER-encoded length.
// `max_length` should be greater than 0. A `nullopt` `value` will write only
// the length field, with a value of 0.
template < class Format >
void
klv_write_opt_lv(
  kwiver::vital::optional< typename Format::data_type > const& value,
  klv_write_iter_t& data, size_t max_length,
  Format const& format )
{
  auto const tracker = track_it( data, max_length );
  auto const length = value ? format.length_of_( *value ) : 0;
  klv_write_ber( length, data, tracker.remaining() );
  if( value )
  {
    format.write_( *value, data, tracker.verify( length ) );
  }
}

// ----------------------------------------------------------------------------
// Base case of `klv_write_trunc_lv`: single-element tuple.
template < class Format >
void
klv_write_trunc_lv(
  std::tuple<
    kwiver::vital::optional< typename Format::data_type const& >
    > const& value,
  klv_write_iter_t& data, size_t length,
  Format const& format )
{
  return lv_detail::write_trunc_lv_impl( value, data, length, format );
}

// ----------------------------------------------------------------------------
// Write a series of length-value pairs according to the provided series of
// formats. All `nullopt` items at the end will be truncated/omitted entirely,
// including their length fields. Any `nullopt` items which have valid items
// following them will be written as just the lengths fields with values of 0.
template < class Format, class... Formats,
          typename std::enable_if< ( sizeof...( Formats ) > 0 ),
                                   bool >::type = true >
void
klv_write_trunc_lv(
  std::tuple<
    kwiver::vital::optional< typename Format::data_type > const&,
    kwiver::vital::optional< typename Formats::data_type > const&...
    > const& value,
  klv_write_iter_t& data, size_t max_length,
  Format const& format, Formats const&... formats )
{
  auto const length =
    klv_length_of_trunc_lv( value, format, formats... );
  if( length > max_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing trunc length-value would overflow data buffer" );
  }
  lv_detail::write_trunc_lv_impl( value, data, length, format, formats... );
}

// ----------------------------------------------------------------------------
// Return the length of `value` plus the length of its BER-encoded length.
template < class Format >
size_t
klv_length_of_lv( typename Format::data_type const& value,
                  Format const& format )
{
  auto const length = format.length_of_( value );
  return klv_ber_length( length ) + length;
}

// ----------------------------------------------------------------------------
// Return the length of `value` plus the length of its BER-encoded length.
template < class Format >
size_t
klv_length_of_opt_lv(
  kwiver::vital::optional< typename Format::data_type > const& value,
  Format const& format )
{
  auto const length = value ? format.length_of_( *value ) : 0;
  return klv_ber_length( length ) + length;
}

// ----------------------------------------------------------------------------
// Base case of `klv_length_of_trunc_lv`: single-element tuple.
template < class Format >
size_t
klv_length_of_trunc_lv(
  std::tuple< kwiver::vital::optional< typename Format::data_type > const&
              > const& value,
  Format const& format )
{
  auto const& item = std::get< 0 >( value );
  auto const length = item ? format.length_of_( *item ) : 0;
  return length ? ( klv_ber_length( length ) + length ) : 0;
}

// ----------------------------------------------------------------------------
// Return the length of `value` plus the length of its BER-encoded length,
// or 0 if the length is 0.
template < class Format, class... Formats,
           typename std::enable_if< ( sizeof...( Formats ) > 0 ),
                                    bool >::type = true >
size_t
klv_length_of_trunc_lv(
  std::tuple< kwiver::vital::optional< typename Format::data_type > const&,
              kwiver::vital::optional< typename Formats::data_type > const&...
              > const& value,
  Format const& format, Formats const&... formats )
{
  auto const& item = std::get< 0 >( value );
  auto const remaining = lv_detail::pop_tuple( value );
  auto const length_of_remaining =
    klv_length_of_trunc_lv( remaining, formats... );
  return length_of_remaining +
         ( length_of_remaining
           ? klv_length_of_opt_lv( item, format )
           : klv_length_of_trunc_lv( std::tie( item ), format ) );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
