// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief vital::image_container C interface implementation

#include "image_container.h"
#include "image_container.hxx"

#include <vital/types/image_container.h>

#include <vital/bindings/c/helpers/c_utils.h>
#include <vital/bindings/c/helpers/image_container.h>

namespace kwiver {
namespace vital_c {

// Allocate our shared pointer cache object
SharedPointerCache< kwiver::vital::image_container, vital_image_container_t >
  IMGC_SPTR_CACHE( "image_container" );

} }

//+ really need a way to display SPTR_CACHE.
// need to verify that pointers are released as needed and cache does not grow without bound

// ----------------------------------------------------------------------------
// These two functions support C++ access to the SPTR_CACHE.

/// @brief Accept shared pointer to image container.
///
/// This function takes a pointer to a shared_pointer and adds it to
/// the SPTR_CACHE in the same way as a constructor (above). This
/// allows us to manage an already existing object.
///
/// @param sptr Pointer to shared pointer
///
/// @return Opaque object pointer/handle
vital_image_container_t* vital_image_container_from_sptr( kwiver::vital::image_container_sptr sptr )
{
  STANDARD_CATCH(
    "C::image_container::from_sptr", 0,

    kwiver::vital_c::IMGC_SPTR_CACHE.store( sptr );
    return reinterpret_cast<vital_image_container_t*>( sptr.get() );
    );
  return 0;
}

vital_image_container_t* vital_image_container_from_c_pointer( kwiver::vital::image_container* ptr )
{
  STANDARD_CATCH(
    "C::image_container::from_c_ptr", 0,
    kwiver::vital::image_container_sptr sptr(ptr);
    kwiver::vital_c::IMGC_SPTR_CACHE.store( sptr );
    return reinterpret_cast<vital_image_container_t*>( ptr );
    );
  return 0;
}

kwiver::vital::image_container_sptr vital_image_container_to_sptr( vital_image_container_t* handle )
{
  STANDARD_CATCH(
    "C::image_container::to_sptr", 0,

    return kwiver::vital_c::IMGC_SPTR_CACHE.get( handle );
    );
  return kwiver::vital::image_container_sptr();
}

// ----------------------------------------------------------------------------
// These following functions support C access to the image container
// and associated SPTR_CACHE
// ----------------------------------------------------------------------------
// / Create a new, simple image container around an image
vital_image_container_t* vital_image_container_new_simple( vital_image_t *img )
{
  STANDARD_CATCH(
    "C::image_container::new_simple", 0,

    kwiver::vital::image *vital_img = reinterpret_cast<kwiver::vital::image*>(img);
    kwiver::vital::image_container_sptr img_sptr( new kwiver::vital::simple_image_container( *vital_img ) );
    kwiver::vital_c::IMGC_SPTR_CACHE.store( img_sptr );
    return reinterpret_cast<vital_image_container_t*>( img_sptr.get() );
  );
  return 0;
}

/// Destroy a vital_image_container_t instance
void vital_image_container_destroy( vital_image_container_t *img_container,
                                    vital_error_handle_t *eh )
{
  STANDARD_CATCH(
    "C::image_container::destroy", eh,
    kwiver::vital_c::IMGC_SPTR_CACHE.erase( img_container );
  );
}

/// Get the size in bytes of an image container
size_t vital_image_container_size( vital_image_container_t *img_c )
{
  STANDARD_CATCH(
    "C::image_container::size", 0,
    return kwiver::vital_c::IMGC_SPTR_CACHE.get( img_c )->size();
  );
  return 0;
}

/// Get the width of the given image in pixels
size_t vital_image_container_width( vital_image_container_t *img_c )
{
  STANDARD_CATCH(
    "C::image_container::width", 0,
    return kwiver::vital_c::IMGC_SPTR_CACHE.get( img_c )->width();
  );
  return 0;
}

/// Get the height of the given image in pixels
size_t vital_image_container_height( vital_image_container_t *img_c )
{
  STANDARD_CATCH(
    "C::image_container::height", 0,
    return kwiver::vital_c::IMGC_SPTR_CACHE.get( img_c )->height();
  );
  return 0;
}

/// Get the depth (number of channels) of the image
size_t vital_image_container_depth( vital_image_container_t *img_c )
{
  STANDARD_CATCH(
    "C::image_container::depth", 0,
    return kwiver::vital_c::IMGC_SPTR_CACHE.get( img_c )->depth();
  );
  return 0;
}

/// Get the in-memory image class to access data
vital_image_t* vital_image_container_get_image( vital_image_container_t *img_c )
{
  STANDARD_CATCH(
    "C::image_container::get_image", NULL,
    kwiver::vital::image_container_sptr ic_sptr( kwiver::vital_c::IMGC_SPTR_CACHE.get( img_c ) );
    return reinterpret_cast<vital_image_t*>(
      new kwiver::vital::image( ic_sptr->get_image() )
    );
  );
  return 0;
}
