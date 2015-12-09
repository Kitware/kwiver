/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
\file This file contains functions to support converting sprokit datum types to
more python friendly types.
 */

#include "vital_type_converters.h"

#include <vital/types/image_container.h>
#include <vital/types/track_set.h>
#include <vital/logger/logger.h>

#include <vital/bindings/c/image_container.hxx>

#include <sprokit/pipeline/datum.h>

#include <boost/any.hpp>

#include <vector>
#include <memory>
#include <stdexcept>

typedef std::vector< double >  double_vector;
typedef std::shared_ptr< double_vector > double_vector_sptr;

static kwiver::vital::logger_handle_t logger( kwiver::vital::get_logger( "vital.type_converters" ) );


namespace {

/**
 * @brief Wrap value in PyCapsule for python return.
 *
 * This function wraps the the supplied value in a new datum and
 * returns that address as a PyCapsule.
 *
 * @param value Value to put in a datum.
 *
 * @return Pointer to new datum holding value.
 */
template< typename T >
PyObject*
put_in_datum_capsule( T value )
{
  // Create a new datum that contains the sptr. Since that is not
  // directly available due to access restrictions on datum CTOR, we
  // have to solve that problem indirectly.

  // We don't want an sptr to a datum (datum_t), just a datum. That's
  // why there's all this foolin' around.

  // Build datum_t that contains sptr controlling vector of doubles
  sprokit::datum_t dsp = sprokit::datum::new_datum( value );

  // get a copy of the datum from datum_t that is not controlled by sptr
  sprokit::datum* datum = new sprokit::datum( *dsp.get() );

  // Return address of datum through PyCapsule object.
  // The caller now owns the datum.
  PyObject* cap = PyCapsule_New( const_cast< sprokit::datum* > ( datum ), "sprokit::datum", NULL );

  return cap;
}

} // end anon namespace


// ==================================================================
/**
 * @brief Convert datum to image container handle
 *
 * The item held in the datum is extracted and registered as an image
 * container.
 *
 * The PyCapsule contains a raw pointer to the datum. The datum_t
 * (sptr to datum) is held by the caller while we extract its contents
 * (an sptr). After this, the datum can be deleted.
 *
 * @param args PyCapsule object
 *
 * @return image container handle
 */
vital_image_container_t*
vital_image_container_from_datum( PyObject* args )
{
  // arg is the capsule
  sprokit::datum* dptr = (sprokit::datum*) PyCapsule_GetPointer( args, "sprokit::datum" );

  try
  {
    // Get boost::any from the datum
    boost::any const any = dptr->get_datum< boost::any > ();

    // Get sptr from boost::any
    kwiver::vital::image_container_sptr sptr = boost::any_cast< kwiver::vital::image_container_sptr > ( any );

    // Register this object with the main image_container interface
    vital_image_container_t* ptr =  vital_image_container_from_sptr( sptr );
    return ptr;
  }
  catch ( boost::bad_any_cast const& e )
  {
    // This is a warning because this converter should only be called
    // if there is good reason to believe that the object really is an
    // image_container.
    LOG_WARN( logger, "Conversion error" << e.what() );
  }

  return NULL;
}


// ------------------------------------------------------------------
/**
 * @brief Convert image container handle to PyCapsule
 *
 * @param handle Opaque handle to image container
 *
 * @return boost::python wrapped Pointer to PyCapsule as PyObject.
 */
PyObject*
vital_image_container_to_datum( vital_image_container_t* handle )
{
  // Get sptr from handle. Use sptr cache access interface
  kwiver::vital::image_container_sptr sptr = vital_image_container_to_sptr( handle );

  if ( ! sptr )
  {
    // Could not find sptr for supplied handle.
    Py_RETURN_NONE;
  }

  // Return address of datum through PyCapsule object.
  // The caller now owns the datum.
  PyObject* cap = put_in_datum_capsule( sptr );
  return cap;
}


// ==================================================================
/**
 * @brief Convert sprokit::datum to array of doubles.
 *
 * The caller is responsible for managing the returned memory.
 *
 * @param args PyCapsule containing the pointer to sprokit::datum
 *
 * @return Address of array of doubles.
 */
double*
double_vector_from_datum( PyObject* args )
{
  // arg is the capsule
  sprokit::datum* dptr = (sprokit::datum*) PyCapsule_GetPointer( args, "sprokit::datum" );

  try
  {
    // Get boost::any from the datum
    boost::any const any = dptr->get_datum< boost::any > ();

    // Get sptr from boost::any
    double_vector_sptr sptr = boost::any_cast< double_vector_sptr > ( any );
    size_t num_elem = sptr->size();

    // make C compatible array of doubles
    double* retval = static_cast< double* > ( calloc( num_elem, sizeof( double ) ) );

    // Copy values from vector yo array
    for ( size_t i = 0; i < num_elem; i++ )
    {
      retval[i] = sptr->at( i );
    }

    return retval;
  }
  catch ( boost::bad_any_cast const& e )
  {
    // This is a warning because this converter should only be called
    // if there is good reason to believe that the object really is an
    // image_container.
    LOG_WARN( logger, "Conversion error: " << e.what() );
  }
  catch ( std::out_of_range const& e )
  {
    LOG_WARN( logger, "Vector access error: " << e.what() );
  }

  // not implemented
  LOG_ERROR( logger, "Function not implemented" );
  return 0;
}


// ------------------------------------------------------------------
/**
 * @brief Convert list of doubles to sprokit::datum containing std::vector<double>
 *
 * This function converts a python list to a sprokit::datum* suitable
 * to go in a pipe. The input array is unmodified and memory
 * management remains with the caller.
 *
 * The returned memory is managed by a shared_ptr that is carried in
 * the allocated datum.
 *
 * @param num_elem   Number of elements in the list
 * @param array      Start address of list
 *
 * @return PyCapsule containing address of sprokit::datum
 */
PyObject*
double_vector_to_datum( PyObject* list )
{
  double_vector_sptr vect( new double_vector () );

  // if ( ! PyList_Check( list ) { log message }

  int num_elem = PyList_Size( list );

  // Copy input values into a new vector.
  for ( size_t i = 0; i < num_elem; i++ )
  {
    PyObject* item = PyList_GetItem( list, i );
    double d = PyFloat_AsDouble( item );
    vect->push_back( d  );
  }

  PyObject* cap = put_in_datum_capsule( vect );
  return cap;
}


// ==================================================================
/**
 * @brief Convert from datum to track_set handle.
 *
 * @param args PyCapsule object
 *
 * @return track_set handle
 */
vital_trackset_t*
vital_trackset_from_datum( PyObject* args )
{
  // Get capsule from args - or arg may be the capsule
  sprokit::datum* dptr = (sprokit::datum*) PyCapsule_GetPointer( args, "sprokit::datum" );

  try
  {
    boost::any const any = dptr->get_datum< boost::any > ();
    kwiver::vital::track_set_sptr sptr = boost::any_cast< kwiver::vital::track_set_sptr > ( any );

    // Register this object with the main track_set interface
    vital_trackset_t* ptr =  vital_trackset_from_sptr( reinterpret_cast< void* > ( &sptr ) );
    return ptr;
  }
  catch ( boost::bad_any_cast const& e )
  {
    // This is a warning because this converter should only be called
    // if there is good reason to believe that the object really is an
    // track_set.
    LOG_WARN( logger, "Conversion error" << e.what() );
  }

  return 0;
}


// more to come
//
