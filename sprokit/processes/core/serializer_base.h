/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
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
 * @file   serializer_base.h
 *
 * @brief  Interface to the serializer base class.
 */

#ifndef SPROKIT_PROCESS_SERIALIZER_BASE_H
#define SPROKIT_PROCESS_SERIALIZER_BASE_H

#include <vital/algo/data_serializer.h>
#include <vital/logger/logger.h>

#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/process_exception.h>

namespace kwiver {

// -----------------------------------------------------------------
/**
 *
 *
 */
class serializer_base
{
public:
  serializer_base( sprokit::process& proc,
                   kwiver::vital::logger_handle_t log );
  virtual ~serializer_base();

  void base_init();

  /**
   * @brief Processes a port with a vital type.
   *
   * This method is called with a port that has a vital type, not the
   * serialized string type. The internal data structures are created
   * if this is the first time the port has been seen and \b true is
   * returned. If the port has already been created, then false is
   * returned.
   *
   * @param port_name Name of the port to process.
   *
   * @return \b true -f the port should be created. \b false if port
   * should not be created.
   */
  bool vital_typed_port_info( sprokit::process::port_t const& port_name );

  /**
   * @brief Processes a port for serialized messages.
   *
   * This method is called with a port that has serialized string
   * type. The internal data structures are created if this is the
   * first time the port has been seen and \b true is returned. If the
   * port has already been created, then false is returned.
   *
   * @param port_name Name of the port to process.
   *
   * @return \b true -f the port should be created. \b false if port
   * should not be created.
   */
  bool byte_string_port_info( sprokit::process::port_t const& port_name );

  void set_port_type( sprokit::process::port_t const&      port_name,
                      sprokit::process::port_type_t const& port_type );

  /**
   * @brief Analyze message and print components
   *
   * @param message The serialized message
   */
  void decode_message( const std::string& message );

  void dump_msg_spec();


protected:
  sprokit::process& m_proc; // associated process

  // The canonical name string defining the data type we are converting.
  std::string m_serialization_type;

  /*
   * A port group defines a set of ports that provide data to a single
   * serializer algo.
   */
  struct message_spec
  {
    message_spec()
      : m_serialized_port_created(false)
    {}

    // This struct defines a single port.
    struct message_element
    {
      // Full Port name to write datum to
      sprokit::process::port_t m_port_name;

      // canonical (logical) port type name string
      sprokit::process::port_type_t m_port_type;

      // name of data element to pass to serializer
      std::string m_element_name;

      // Algorithm name as constructed from serialization type and data type
      std::string m_algo_name;

      // Algorithm handles a group of data items
      vital::algo::data_serializer_sptr m_serializer;
    };

    // indexed by m_element_name
    std::map< std::string, message_element > m_elements;

    // port to read serialized data from
    sprokit::process::port_t m_serialized_port_name;
    bool m_serialized_port_created;
  };

  // map is indexed by message-name
  std::map< std::string, message_spec > m_message_spec_list;


private:
  kwiver::vital::logger_handle_t m_logger;

}; // end class serializer_base

} // end namespace

#endif // SPROKIT_PROCESS_SERIALIZER_BASE_H
