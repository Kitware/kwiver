// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Utility methods for performing pixelwise operations.

#ifndef KWIVER_VITAL_UTIL_VISIT_IMAGE_TXX_
#define KWIVER_VITAL_UTIL_VISIT_IMAGE_TXX_

#include <vital/util/visit_image.h>

#include <vital/util/tuple_utils.h>

#include <algorithm>
#include <limits>
#include <type_traits>

#include <cmath>
#include <cstdint>

namespace kwiver {

namespace vital {

// Needed for uncrustify to parse this file
#define template_operator template operator

namespace _visit_image_detail
{
// ----------------------------------------------------------------------------
  template < class T1, class T2 >
  using _copy_const_t =
    std::conditional_t<
      std::is_const_v< std::remove_reference_t< T1 > >,
      std::add_const_t< T2 >,
      std::remove_const_t< T2 > >;

// ----------------------------------------------------------------------------
  template < class Visitor, class T1 >
  struct _visit_pixel_traits2_helper2
  {
    _visit_pixel_traits2_helper2( Visitor&& visitor )
      : visitor{ std::forward< Visitor >( visitor ) }
    {}

    template < class T2, class... Args >
    inline
    void
    operator()( Args&&... args ) const
    {
      visitor.template_operator() < T1,
        T2 > ( std::forward< Args >( args )... );
    }

    Visitor && visitor;
  };

// ----------------------------------------------------------------------------
  template < class Visitor, class Image2 >
  struct _visit_pixel_traits2_helper1
  {
    _visit_pixel_traits2_helper1( Visitor&& visitor, Image2&& img2 )
      : visitor{ std::forward< Visitor >( visitor ) },
        img2{ img2 }
    {}

    template < class T1, class... Args >
    inline
    void
    operator()( Args&&... args ) const
    {
      _visit_pixel_traits2_helper2< Visitor, T1 > helper{ visitor };
      visit_pixel_traits( helper, img2, std::forward< Args >( args )... );
    }

    Visitor && visitor;
    Image2 && img2;
  };

// ----------------------------------------------------------------------------
  struct _layout_info
  {
    size_t indices[ 3 ];
    size_t reverse_indices[ 3 ];
    size_t sizes[ 3 ];
    ptrdiff_t steps[ 3 ];
    size_t total_sizes[ 3 ];
    uint8_t contiguous_rank;
    bool is_contiguous[ 3 ];
  };

// ----------------------------------------------------------------------------
  void
  _fill_layout_from_indices( _layout_info& layout, image const& img )
  {
    layout.reverse_indices[ layout.indices[ 0 ] ] = 0;
    layout.reverse_indices[ layout.indices[ 1 ] ] = 1;
    layout.reverse_indices[ layout.indices[ 2 ] ] = 2;

    layout.sizes[ layout.indices[ 0 ] ] = img.width();
    layout.sizes[ layout.indices[ 1 ] ] = img.height();
    layout.sizes[ layout.indices[ 2 ] ] = img.depth();

    layout.steps[ layout.indices[ 0 ] ] = img.w_step();
    layout.steps[ layout.indices[ 1 ] ] = img.h_step();
    layout.steps[ layout.indices[ 2 ] ] = img.d_step();

    size_t const element_size = img.pixel_traits().num_bytes;
    layout.contiguous_rank = 0;
    for( size_t i = 0; i < 3; ++i )
    {
      layout.total_sizes[ i ] =
        i == 0 ? 1 : layout.total_sizes[ i - 1 ] * layout.sizes[ i - 1 ];
      if(
        ( i == 0 || layout.is_contiguous[ i - 1 ] ) &&
        ( layout.steps[ i ] == layout.total_sizes[ i ] * element_size ||
          layout.sizes[ i ] == 1 ) )
      {
        layout.is_contiguous[ i ] = true;
        ++layout.contiguous_rank;
      }
      else
      {
        layout.is_contiguous[ i ] = false;
      }
    }
  }

// ----------------------------------------------------------------------------
  _layout_info
  _get_element_layout_info( image const& img )
  {
    _layout_info layout;

    auto const cmp_wh = std::abs( img.w_step() ) < std::abs( img.h_step() );
    auto const cmp_dh = std::abs( img.d_step() ) < std::abs( img.h_step() );
    auto const cmp_dw = std::abs( img.d_step() ) < std::abs( img.w_step() );

    layout.indices[ 0 ] =
      static_cast< size_t >( !cmp_wh ) + static_cast< size_t >( cmp_dw );
    layout.indices[ 1 ] =
      static_cast< size_t >( cmp_wh ) + static_cast< size_t >( cmp_dh );
    layout.indices[ 2 ] =
      static_cast< size_t >( !cmp_dw ) + static_cast< size_t >( !cmp_dh );

    _fill_layout_from_indices( layout, img );

    return layout;
  }

// ----------------------------------------------------------------------------
  _layout_info
  _get_pixel_layout_info( image const& img )
  {
    _layout_info layout{};

    auto const cmp_wh = std::abs( img.w_step() ) < std::abs( img.h_step() );

    layout.indices[ 0 ] = static_cast< size_t >( !cmp_wh ) + 1;
    layout.indices[ 1 ] = static_cast< size_t >( cmp_wh ) + 1;
    layout.indices[ 2 ] = 0;

    _fill_layout_from_indices( layout, img );

    return layout;
  }

// ----------------------------------------------------------------------------
  template < class Visitor >
  struct _visit_image_scalars_helper
  {
    _visit_image_scalars_helper( Visitor&& visitor )
      : visitor{ std::forward< Visitor >( visitor ) }
    {}

    template < class T, class Image >
    void
    operator()( Image&& img ) const
    {
      auto const layout = _get_element_layout_info( img );

      auto ptr = static_cast< T* >( img.first_pixel() );
      if( layout.is_contiguous[ 2 ] )
      {
        auto const size =
          layout.sizes[ 0 ] * layout.sizes[ 1 ] * layout.sizes[ 2 ];
        for( size_t i = 0; i < size; ++i )
        {
          visitor( *ptr );
          ++ptr;
        }
      }
      else if( layout.is_contiguous[ 1 ] )
      {
        auto const size_i = layout.sizes[ 2 ];
        auto const size_j = layout.sizes[ 0 ] * layout.sizes[ 1 ];
        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( *ptr );
            ++ptr;
          }
          ptr += layout.steps[ 2 ] - size_j;
        }
      }
      else if( layout.is_contiguous[ 0 ] )
      {
        auto const size_i = layout.sizes[ 2 ];
        auto const size_j = layout.sizes[ 1 ];
        auto const size_k = layout.sizes[ 0 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            for( size_t k = 0; k < size_k; ++k )
            {
              visitor( *ptr );
              ++ptr;
            }
            ptr += layout.steps[ 1 ] - size_k;
          }
          ptr += layout.steps[ 2 ] - size_j * layout.steps[ 1 ];
        }
      }
      else
      {
        auto const size_i = layout.sizes[ 2 ];
        auto const size_j = layout.sizes[ 1 ];
        auto const size_k = layout.sizes[ 0 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            for( size_t k = 0; k < size_k; ++k )
            {
              visitor( *ptr );
              ptr += layout.steps[ 0 ];
            }
            ptr += layout.steps[ 1 ] - size_k * layout.steps[ 0 ];
          }
          ptr += layout.steps[ 2 ] - size_j * layout.steps[ 1 ];
        }
      }
    }

    Visitor && visitor;
  };

// ----------------------------------------------------------------------------
  template < class Visitor >
  struct _visit_image_scalars2_across_type_helper
  {
    _visit_image_scalars2_across_type_helper( Visitor&& visitor )
      : visitor{ std::forward< Visitor >( visitor ) }
    {}

    template < class T1, class T2, class Image1, class Image2 >
    void
    operator()( Image1&& img1, Image2&& img2 ) const
    {
      auto layout1 = _get_element_layout_info( img1 );
      auto layout2 = _get_element_layout_info( img2 );
      if( layout1.contiguous_rank < layout2.contiguous_rank )
      {
        std::copy_n( layout2.indices, 3, layout1.indices );
        _fill_layout_from_indices( layout1, img1 );
      }
      else
      {
        std::copy_n( layout1.indices, 3, layout2.indices );
        _fill_layout_from_indices( layout2, img2 );
      }

      auto ptr1 = static_cast< T1* >( img1.first_pixel() );
      auto ptr2 = static_cast< T2* >( img2.first_pixel() );
      if( layout1.is_contiguous[ 2 ] && layout2.is_contiguous[ 2 ] )
      {
        auto const size =
          layout1.sizes[ 0 ] * layout1.sizes[ 1 ] * layout1.sizes[ 2 ];
        for( size_t i = 0; i < size; ++i )
        {
          visitor( *ptr1, *ptr2 );
          ++ptr1;
          ++ptr2;
        }
      }
      else if( layout1.is_contiguous[ 1 ] && layout2.is_contiguous[ 1 ] )
      {
        auto const size_i = layout1.sizes[ 2 ];
        auto const size_j = layout1.sizes[ 0 ] * layout1.sizes[ 1 ];
        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( *ptr1, *ptr2 );
            ++ptr1;
            ++ptr2;
          }
          ptr1 += layout1.steps[ 2 ] - size_j;
          ptr2 += layout2.steps[ 2 ] - size_j;
        }
      }
      else if( layout1.is_contiguous[ 0 ] && layout2.is_contiguous[ 0 ] )
      {
        auto const size_i = layout1.sizes[ 2 ];
        auto const size_j = layout1.sizes[ 1 ];
        auto const size_k = layout1.sizes[ 0 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            for( size_t k = 0; k < size_k; ++k )
            {
              visitor( *ptr1, *ptr2 );
              ++ptr1;
              ++ptr2;
            }
            ptr1 += layout1.steps[ 1 ] - size_k;
            ptr2 += layout2.steps[ 1 ] - size_k;
          }
          ptr1 += layout1.steps[ 2 ] - size_j * layout1.steps[ 1 ];
          ptr2 += layout2.steps[ 2 ] - size_j * layout2.steps[ 1 ];
        }
      }
      else
      {
        auto const size_i = layout1.sizes[ 2 ];
        auto const size_j = layout1.sizes[ 1 ];
        auto const size_k = layout1.sizes[ 0 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            for( size_t k = 0; k < size_k; ++k )
            {
              visitor( *ptr1, *ptr2 );
              ptr1 += layout1.steps[ 0 ];
              ptr2 += layout2.steps[ 0 ];
            }
            ptr1 += layout1.steps[ 1 ] - size_k * layout1.steps[ 0 ];
            ptr2 += layout2.steps[ 1 ] - size_k * layout2.steps[ 0 ];
          }
          ptr1 += layout1.steps[ 2 ] - size_j * layout1.steps[ 1 ];
          ptr2 += layout2.steps[ 2 ] - size_j * layout2.steps[ 1 ];
        }
      }
    }

    Visitor && visitor;
  };

// ----------------------------------------------------------------------------
  template < class Visitor, class T1 = void, class T2 = void >
  struct _visit_image_scalars2_helper
  {
    _visit_image_scalars2_helper( Visitor&& visitor )
      : helper{ std::forward< Visitor >( visitor ) }
    {}

    template < class T, class Image1, class Image2 >
    void
    operator()( Image1&& img1, Image2&& img2 ) const
    {
      helper.template_operator() <
      _copy_const_t<
        Image1, std::conditional_t< std::is_same_v< T1, void >, T, T1 > >,
      _copy_const_t<
        Image2, std::conditional_t< std::is_same_v< T2, void >, T, T2 > > > (
        std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ) );
    }

    _visit_image_scalars2_across_type_helper< Visitor > helper;
  };

// ----------------------------------------------------------------------------
  template < class Visitor, size_t Depth >
  struct _visit_image_pixels_helper
  {
    _visit_image_pixels_helper( Visitor&& visitor )
      : visitor{ std::forward< Visitor >( visitor ) }
    {}

    template < class T, class Image >
    void
    operator()( Image&& img ) const
    {
      auto const layout = _get_element_layout_info( img );

      auto ptr = static_cast< T* >( img.first_pixel() );
      if( layout.is_contiguous[ 2 ] )
      {
        auto const size = img.width() * img.height();
        for( size_t i = 0; i < size; ++i )
        {
          visitor( tie_n< Depth >( ptr ) );
          ptr += Depth;
        }
      }
      else if( layout.is_contiguous[ 1 ] )
      {
        auto const size_i = layout.sizes[ 2 ];
        auto const size_j = layout.sizes[ 1 ];
        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( tie_n< Depth >( ptr ) );
            ptr += Depth;
          }
          ptr += layout.steps[ 2 ] - size_j * Depth;
        }
      }
      else if( layout.is_contiguous[ 0 ] )
      {
        auto const size_i = layout.sizes[ 2 ];
        auto const size_j = layout.sizes[ 1 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( tie_n< Depth >( ptr ) );
            ptr += layout.steps[ 1 ];
          }
          ptr += layout.steps[ 2 ] - size_j * layout.steps[ 1 ];
        }
      }
      else
      {
        auto const size_i = layout.sizes[ 2 ];
        auto const size_j = layout.sizes[ 1 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( tie_stepped_n< Depth >( ptr, layout.steps[ 0 ] ) );
            ptr += layout.steps[ 1 ];
          }
          ptr += layout.steps[ 2 ] - size_j * layout.steps[ 1 ];
        }
      }
    }

    Visitor && visitor;
  };

// ----------------------------------------------------------------------------
  template < size_t Depth1, size_t Depth2, class Visitor >
  struct _visit_image_pixels2_across_type_helper
  {
    _visit_image_pixels2_across_type_helper( Visitor&& visitor )
      : visitor{ std::forward< Visitor >( visitor ) }
    {}

    template < class T1, class T2, class Image1, class Image2 >
    void
    operator()( Image1&& img1, Image2&& img2 ) const
    {
      auto layout1 = _get_pixel_layout_info( img1 );
      auto layout2 = _get_pixel_layout_info( img2 );
      if( layout1.contiguous_rank < layout2.contiguous_rank )
      {
        std::copy_n( layout2.indices, 3, layout1.indices );
        _fill_layout_from_indices( layout1, img1 );
      }
      else
      {
        std::copy_n( layout1.indices, 3, layout2.indices );
        _fill_layout_from_indices( layout2, img2 );
      }

      auto ptr1 = static_cast< T1* >( img1.first_pixel() );
      auto ptr2 = static_cast< T2* >( img2.first_pixel() );
      if( layout1.is_contiguous[ 2 ] && layout2.is_contiguous[ 2 ] )
      {
        auto const size = img1.width() * img1.height();
        for( size_t i = 0; i < size; ++i )
        {
          visitor( tie_n< Depth1 >( ptr1 ), tie_n< Depth2 >( ptr2 ) );
          ptr1 += Depth1;
          ptr2 += Depth2;
        }
      }
      else if( layout1.is_contiguous[ 1 ] && layout2.is_contiguous[ 1 ] )
      {
        auto const size_i = layout1.sizes[ 2 ];
        auto const size_j = layout1.sizes[ 1 ];
        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( tie_n< Depth1 >( ptr1 ), tie_n< Depth2 >( ptr2 ) );
            ptr1 += Depth1;
            ptr2 += Depth2;
          }
          ptr1, layout1.steps[ 2 ] - size_j * Depth1;
          ptr2, layout2.steps[ 2 ] - size_j * Depth2;
        }
      }
      else if( layout1.is_contiguous[ 0 ] && layout2.is_contiguous[ 0 ] )
      {
        auto const size_i = layout1.sizes[ 2 ];
        auto const size_j = layout1.sizes[ 1 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor( tie_n< Depth1 >( ptr1 ), tie_n< Depth2 >( ptr2 ) );
            ptr1 += layout1.steps[ 1 ];
            ptr2 += layout2.steps[ 1 ];
          }
          ptr1 += layout1.steps[ 2 ] - size_j * layout1.steps[ 1 ];
          ptr2 += layout2.steps[ 2 ] - size_j * layout2.steps[ 1 ];
        }
      }
      else
      {
        auto const size_i = layout1.sizes[ 2 ];
        auto const size_j = layout1.sizes[ 1 ];

        for( size_t i = 0; i < size_i; ++i )
        {
          for( size_t j = 0; j < size_j; ++j )
          {
            visitor(
              tie_stepped_n< Depth1 >( ptr1, layout1.steps[ 0 ] ),
              tie_stepped_n< Depth2 >( ptr2, layout2.steps[ 0 ] ) );
            ptr1 += layout1.steps[ 1 ];
            ptr2 += layout2.steps[ 1 ];
          }
          ptr1 += layout1.steps[ 2 ] - size_j * layout1.steps[ 1 ];
          ptr2 += layout2.steps[ 2 ] - size_j * layout2.steps[ 1 ];
        }
      }
    }

    Visitor && visitor;
  };

// ----------------------------------------------------------------------------
  template <
    size_t Depth1, size_t Depth2,
    class Visitor, class T1 = void, class T2 = void >
  struct _visit_image_pixels2_helper
  {
    _visit_image_pixels2_helper( Visitor&& visitor )
      : helper{ std::forward< Visitor >( visitor ) }
    {}

    template < class T, class Image1, class Image2 >
    void
    operator()( Image1&& img1, Image2&& img2 ) const
    {
      helper.template_operator() <
      _copy_const_t<
        Image1, std::conditional_t< std::is_same_v< T1, void >, T, T1 > >,
      _copy_const_t<
        Image2, std::conditional_t< std::is_same_v< T2, void >, T, T2 > > > (
        std::forward< Image1 >( img1 ), std::forward< Image2 >( img2 ) );
    }

    _visit_image_pixels2_across_type_helper< Depth1, Depth2, Visitor > helper;
  };
}

// ----------------------------------------------------------------------------
template < class Visitor, class Image, class... Args >
void
visit_pixel_traits( Visitor&& visitor, Image&& img, Args&&... args )
{
  using namespace _visit_image_detail;

  auto const traits = img.pixel_traits();
  switch( traits.type )
  {
    case image_pixel_traits::BOOL:
      if( traits.num_bytes == sizeof( bool ) )
      {
        visitor.template_operator() < _copy_const_t< Image, bool > > (
          std::forward< Args >( args )... );
        return;
      }
      break;
    case image_pixel_traits::SIGNED:
      switch( traits.num_bytes )
      {
        case sizeof( int8_t ):
          visitor.template_operator() < _copy_const_t< Image, int8_t > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( int16_t ):
          visitor.template_operator() < _copy_const_t< Image, int16_t > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( int32_t ):
          visitor.template_operator() < _copy_const_t< Image, int32_t > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( int64_t ):
          visitor.template_operator() < _copy_const_t< Image, int64_t > > (
            std::forward< Args >( args )... );
          return;
        default:
          break;
      }
      break;
    case image_pixel_traits::UNSIGNED:
      switch( traits.num_bytes )
      {
        case sizeof( uint8_t ):
          visitor.template_operator() < _copy_const_t< Image, uint8_t > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( uint16_t ):
          visitor.template_operator() < _copy_const_t< Image, uint16_t > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( uint32_t ):
          visitor.template_operator() < _copy_const_t< Image, uint32_t > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( uint64_t ):
          visitor.template_operator() < _copy_const_t< Image, uint64_t > > (
            std::forward< Args >( args )... );
          return;
        default:
          break;
      }
      break;
    case image_pixel_traits::FLOAT:
      switch( traits.num_bytes )
      {
        case sizeof( float ):
          visitor.template_operator() < _copy_const_t< Image, float > > (
            std::forward< Args >( args )... );
          return;
        case sizeof( double ):
          visitor.template_operator() < _copy_const_t< Image, double > > (
            std::forward< Args >( args )... );
          return;
      }
    default:
      break;
  }

  throw std::runtime_error( "Image pixel traits not supported" );
}

// ----------------------------------------------------------------------------
template < class Visitor, class Image1, class Image2, class... Args >
void
visit_pixel_traits2(
  Visitor&& visitor, Image1&& img1, Image2&& img2, Args&&... args )
{
  using namespace _visit_image_detail;

  _visit_pixel_traits2_helper1< Visitor, Image2 > helper{
    std::forward< Visitor >( visitor ), img2 };
  visit_pixel_traits( helper, img1, std::forward< Args >( args )... );
}

// ----------------------------------------------------------------------------
template < class T, class Visitor, class Image >
void
visit_image_scalars( Visitor&& visitor, Image&& img )
{
  using namespace _visit_image_detail;

  if constexpr( std::is_same_v< T, void > )
  {
    _visit_image_scalars_helper< Visitor > helper{
      std::forward< Visitor >( visitor ) };
    visit_pixel_traits(
      helper, std::forward< Image >( img ),
      std::forward< Image >( img ) );
  }
  else
  {
    if( img.pixel_traits() != image_pixel_traits_of< T >() )
    {
      throw std::runtime_error(
        "Image pixel traits do not match compile-time specification" );
    }

    _visit_image_scalars_helper< Visitor > helper{ std::forward< Visitor >(
      visitor ) };
    helper.template_operator() < _copy_const_t< Image,
      T > > ( std::forward< Image >( img ) );
  }
}

// ----------------------------------------------------------------------------
template <
  class T1, class T2, bool TypesMatch,
  class Visitor, class Image1, class Image2 >
void
visit_image_scalars2( Visitor&& visitor, Image1&& img1, Image2&& img2 )
{
  using namespace _visit_image_detail;

  constexpr auto t1_known = !std::is_same_v< T1, void >;
  constexpr auto t2_known = !std::is_same_v< T2, void >;
  if constexpr( t1_known )
  {
    if( img1.pixel_traits() != image_pixel_traits_of< T1 >() )
    {
      throw std::runtime_error{
              "Image pixel traits do not match compile-time specification" };
    }
  }

  if constexpr( t2_known )
  {
    if( img2.pixel_traits() != image_pixel_traits_of< T2 >() )
    {
      throw std::runtime_error{
              "Image pixel traits do not match compile-time specification" };
    }
  }

  if constexpr( ( !t1_known || !t2_known ) && TypesMatch )
  {
    if( img1.pixel_traits() != img2.pixel_traits() )
    {
      throw std::runtime_error{ "Image pixel traits do not match" };
    }
  }

  if( img1.width() != img2.width() ||
      img1.height() != img2.height() ||
      img1.depth() != img2.depth() )
  {
    throw std::runtime_error{ "Image dimensions do not match" };
  }

  if constexpr( t1_known && t2_known )
  {
    _visit_image_scalars2_across_type_helper< Visitor > helper{
      std::forward< Visitor >( visitor ) };
    helper.template_operator() <
    _copy_const_t< Image1, T1 >,
    _copy_const_t< Image2, T2 > > ( std::forward< Image1 >( img1 ),
                                    std::forward< Image2 >( img2 ) );
  }
  else if constexpr( !t1_known && !t2_known )
  {
    if constexpr( TypesMatch )
    {
      _visit_image_scalars2_helper< Visitor > helper{
        std::forward< Visitor >( visitor ) };
      visit_pixel_traits(
        helper, img1, std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ) );
    }
    else
    {
      _visit_image_scalars2_across_type_helper< Visitor > helper{
        std::forward< Visitor >( visitor ) };
      visit_pixel_traits2(
        helper, std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ), std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ) );
    }
  }
  else
  {
    _visit_image_scalars2_helper< Visitor, T1, T2 > helper{
      std::forward< Visitor >( visitor ) };
    visit_pixel_traits(
      helper, t1_known ? img2 : img1,
      std::forward< Image1 >( img1 ), std::forward< Image2 >( img2 ) );
  }
}

// ----------------------------------------------------------------------------
template < class T1, class T2, class Visitor, class Image1 >
image
visit_image_scalars2_create( Visitor&& visitor, Image1&& img1 )
{
  using namespace _visit_image_detail;

  using derived_t = std::conditional_t< std::is_same_v< T2, void >, T1, T2 >;

  auto pixel_traits = img1.pixel_traits();
  if constexpr( !std::is_same_v< T2, void > )
  {
    pixel_traits = vital::image_pixel_traits_of< T2 >();
  }

  vital::image img2{
    img1.width(), img1.height(), img1.depth(), img1.w_step() != 1,
    pixel_traits };
  visit_image_scalars2< T1, derived_t, std::is_same_v< T2, void >, Visitor,
    Image1 >(
    std::forward< Visitor >( visitor ), std::forward< Image1 >( img1 ), img2 );
  return img2;
}

// ----------------------------------------------------------------------------
template < size_t Depth, class T, class Visitor, class Image >
void
visit_image_pixels( Visitor&& visitor, Image&& img )
{
  using namespace _visit_image_detail;

  if( img.depth() != Depth )
  {
    throw std::runtime_error(
      "Image depth does not match compile-time specification" );
  }

  if constexpr( std::is_same_v< T, void > )
  {
    _visit_image_pixels_helper< Visitor, Depth > helper{
      std::forward< Visitor >( visitor ) };
    visit_pixel_traits( helper, img, img );
  }
  else
  {
    if( img.pixel_traits() != image_pixel_traits_of< T >() )
    {
      throw std::runtime_error(
        "Image pixel traits do not match compile-time specification" );
    }

    _visit_image_pixels_helper< Visitor, Depth > helper{
      std::forward< Visitor >( visitor ) };
    helper.template_operator() < _copy_const_t< Image,
      T > > ( std::forward< Image >( img ) );
  }
}

// ----------------------------------------------------------------------------
template <
  size_t Depth1, size_t Depth2, class T1, class T2, bool TypesMatch,
  class Visitor, class Image1, class Image2 >
void
visit_image_pixels2( Visitor&& visitor, Image1&& img1, Image2&& img2 )
{
  using namespace _visit_image_detail;

  constexpr auto t1_known = !std::is_same_v< T1, void >;
  constexpr auto t2_known = !std::is_same_v< T2, void >;

  if constexpr( t1_known )
  {
    if( img1.pixel_traits() != image_pixel_traits_of< T1 >() )
    {
      throw std::runtime_error{
              "Image pixel traits do not match compile-time specification" };
    }
  }

  if constexpr( t2_known )
  {
    if( img2.pixel_traits() != image_pixel_traits_of< T2 >() )
    {
      throw std::runtime_error{
              "Image pixel traits do not match compile-time specification" };
    }
  }

  if constexpr( ( !t1_known || !t2_known ) && TypesMatch )
  {
    if( img1.pixel_traits() != img2.pixel_traits() )
    {
      throw std::runtime_error{ "Image pixel traits do not match" };
    }
  }

  if( img1.width() != img2.width() ||
      img1.height() != img2.height() )
  {
    throw std::runtime_error{ "Image dimensions do not match" };
  }

  if( img1.depth() != Depth1 || img2.depth() != Depth2 )
  {
    throw std::runtime_error(
      "Image depth does not match compile-time specification" );
  }

  if constexpr( t1_known && t2_known )
  {
    _visit_image_pixels2_across_type_helper<
      Depth1, Depth2, Visitor > helper{ std::forward< Visitor >( visitor ) };
    helper.template_operator() <
    _copy_const_t< Image1, T1 >,
    _copy_const_t< Image2, T2 > > ( std::forward< Image1 >( img1 ),
                                    std::forward< Image2 >( img2 ) );
  }
  else if constexpr( !t1_known && !t2_known )
  {
    if constexpr( TypesMatch )
    {
      _visit_image_pixels2_helper< Depth1, Depth2, Visitor > helper{
        std::forward< Visitor >( visitor ) };
      visit_pixel_traits(
        helper, img1, std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ) );
    }
    else
    {
      _visit_image_pixels2_across_type_helper<
        Depth1, Depth2, Visitor > helper{ std::forward< Visitor >( visitor ) };
      visit_pixel_traits2(
        helper, std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ), std::forward< Image1 >( img1 ),
        std::forward< Image2 >( img2 ) );
    }
  }
  else
  {
    _visit_image_pixels2_helper< Depth1, Depth2, Visitor, T1, T2 > helper{
      std::forward< Visitor >( visitor ) };
    visit_pixel_traits(
      helper, t1_known ? img2 : img1,
      std::forward< Image1 >( img1 ), std::forward< Image2 >( img2 ) );
  }
}

// ----------------------------------------------------------------------------
template <
  size_t Depth1, size_t Depth2, class T1, class T2, bool TypesMatch,
  class Visitor, class Image1 >
image
visit_image_pixels2_create( Visitor&& visitor, Image1&& img1 )
{
  using namespace _visit_image_detail;

  using derived_t = std::conditional_t< std::is_same_v< T2, void >, T1, T2 >;

  auto pixel_traits = img1.pixel_traits();
  if constexpr( !std::is_same_v< T2, void > )
  {
    pixel_traits = vital::image_pixel_traits_of< T2 >();
  }

  vital::image img2{
    img1.width(), img1.height(), Depth2, img1.w_step() != 1, pixel_traits };
  visit_image_pixels2<
    Depth1, Depth2, T1, derived_t, std::is_same_v< T2, void >, Visitor,
    Image1 >(
    std::forward< Visitor >( visitor ), std::forward< Image1 >( img1 ), img2 );
  return img2;
}

#undef template_operator

} // namespace vital

} // namespace kwiver

#endif
