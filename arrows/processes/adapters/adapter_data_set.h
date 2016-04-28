/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
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


#ifndef PROCESS_ADAPTER_DATA_SET_H
#define PROCESS_ADAPTER_DATA_SET_H

#include "adapter_types.h"

#include <arrows/processes/adapters/kwiver_adapter_export.h>

#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/datum.h>

#include <map>
#include <string>

namespace kwiver {
namespace adapter{

// -----------------------------------------------------------------
/**
 * @brief Adapter datum to or from sprokit external adapter process.
 *
 * This class represents a set of data to be pushed into a sprokit
 * pipeline. Each datum in this set is pushed into the corresponding
 * named port.
 *
 */
class KWIVER_ADAPTER_EXPORT adapter_data_set
{
public:
  typedef std::map< sprokit::process::port_t, sprokit::datum_t > datum_map_t;

  /**
   * @brief Type of data set.
   *
   * These are used to specify the payload in this data set. Usually
   * it contains data for the ports, but at then end it is marked with
   * end_of_input. End could be done by add all required datum's with
   * the \b complete type, but requires the user to create a whole
   * bunch of datum's.
   */
  enum data_set_type
  {
    data = 1,
    end_of_input                // indicates end of input
  };

  /**
   * @brief Create a new data set object.
   *
   * This factory method returns a newly allocated object managed by
   * smart pointer.
   *
   * @param type Data set type (data or input end marker)
   *
   * @return New data set object managed by smart pointer.
   */
  static adapter_data_set_t create( data_set_type type = data_set_type::data );

  ~adapter_data_set();

  /**
   * @brief Get data set type.
   *
   * This method returns the data set type. Valid types are defined in
   * the data_set_type enum.
   *
   * @return data set type enum.
   */
  data_set_type type() const { return m_set_type; }

  /**
   * @brief Test if this object has end of data marker.
   *
   * This method is a convenient way to check if the type is the end
   * marker.
   *
   * @return \b true if this is end of data element.
   */
  bool is_end_of_data() const { return (m_set_type == end_of_input); }

  /**
   * @brief Add datum to this data set.
   *
   * @param port Name of the port where data is sent.
   * @param datum Sprokit datum object to be pushed to port.
   */
  void add_datum( sprokit::process::port_t const& port, sprokit::datum_t const& datum );

  /**
   * @brief Add typed value to data set.
   *
   * This method adds the specified value to the adapter data set. The
   * value is copied into the data set.
   *
   * @param port Name of the port where data is sent.
   * @param val Value to be wrapped in datum for port.
   */
  template <typename T>
  void add_value( sprokit::process::port_t const& port, T const& val )
  {
    m_port_datum_set.insert(
      std::pair<std::string, sprokit::datum_t> (port, sprokit::datum::new_datum<T>( val ) ) );
  }

  //@{
  /**
   * @brief Get begin iterator for items in this data set.
   *
   *
   * @return Begin iterator.
   */
  datum_map_t::iterator begin();
  datum_map_t::const_iterator begin() const;
  //@}


  //@{
  /**
   * @brief Get ending iterator for items in this data set.
   *
   *
   * @return End iterator.
   */
  datum_map_t::iterator end();
  datum_map_t::const_iterator end() const;
  //@}

  /**
   * @brief Find entry for specific port name.
   *
   * This method returns an iterator pointing at the entry for the
   * specified port. The datum can be accessed through it->second.  If
   * the specified port name is not in the set, the returned iterator
   * is set to end();
   *
   * @param port_t Name of port to locate.
   *
   * @return Iterator pointing at desired entry or end() iterator if element not found.
   */
  datum_map_t::iterator find( sprokit::process::port_t const& port );

protected:
  KWIVER_ADAPTER_NO_EXPORT adapter_data_set( data_set_type type ); // private CTOR - use factory method

private:
  const data_set_type m_set_type;

  datum_map_t m_port_datum_set;

}; // end class adapter_datum

} } // end namespace

#endif // ADAPTER_DATA_SET
