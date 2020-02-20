/*ckwg +29
 * Copyright 2015, 2019 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

#ifndef KWIVER_LOGGER_LOCATION_INFO_H_
#define KWIVER_LOGGER_LOCATION_INFO_H_

#include <vital/logger/vital_logger_export.h>

#include <string>

namespace kwiver {
namespace vital {
namespace logger_ns {

// ----------------------------------------------------------------
/** Location of logging call.
 *
 * This class represents the location of the logging call.
 *
 */
class VITAL_LOGGER_EXPORT location_info
{
public:
  /** Constructor. Create a default of unknown location */
  location_info();

  /** Constructor. Create a location object for the current site */
  location_info( char const* filename, char const* method, int line );

  //@{
  /** Default values for unknown locations */
  static const char * const NA;
  static const char * const NA_METHOD;
  //@}

  /**
   * @brief Get file name.
   *
   * The file name for the current location is returned without
   * leading path components and with file extension.
   *
   * @return file name, may be null.
   */
  std::string get_file_name() const;
  char const * get_file_name_ptr() const { return m_fileName; }

  /**
   * @brief Get path part of file spec.
   *
   * The path or base name portion of the file path is returned
   * without the file name.
   *
   * @return file name base. May be null.
   */
  std::string get_file_path() const;

  /**
   * @brief Get full function/method signature.
   *
   * The whole signature, as captured by the macro, is returned.
   *
   * @return function/method signature
   */
  std::string get_signature() const;

  /**
   * @brief Get method name.
   *
   * The method name for the current location is returned.
   *
   * @return method name, may be null.
   */
  std::string get_method_name() const;
  char const * get_method_name_ptr() const { return m_methodName; }

  /**
   * @brief Get class name.
   *
   * This method returns the method name for the current location.
   *
   * @return class name.
   */
  std::string get_class_name() const;

  /**
   * @brief Get line number.
   *
   * The line number for the current location is returned.
   *
   * @return line number, -1 indicates unknown line.
   */
  int get_line_number() const;


private:
  const char * const m_fileName;
  const char * const m_methodName;
  int m_lineNumber;

}; // end class location_info

} } } // end namespace


#if defined(_MSC_VER)
#if _MSC_VER >= 1300
      #define __KWIVER_LOGGER_FUNC__ __FUNCSIG__
#endif
#else
#if defined(__GNUC__)
      #define __KWIVER_LOGGER_FUNC__ __PRETTY_FUNCTION__
#endif
#endif
#if !defined(__KWIVER_LOGGER_FUNC__)
#define __KWIVER_LOGGER_FUNC__ ""
#endif

#define KWIVER_LOGGER_SITE ::kwiver::vital::logger_ns::location_info(__FILE__, \
           __KWIVER_LOGGER_FUNC__,                                       \
           __LINE__)

#endif
