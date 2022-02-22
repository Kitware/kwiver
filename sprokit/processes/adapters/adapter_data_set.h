// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Implementation for adapter_data_set interface
 */

#ifndef PROCESS_ADAPTER_DATA_SET_H
#define PROCESS_ADAPTER_DATA_SET_H

#include "adapter_types.h"
#include <vital/vital_config.h>

#include <sprokit/processes/adapters/kwiver_adapter_export.h>

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
 * This class represents a set of data to be used as input or contains
 * output from a sprokit pipeline. This set consists of a set of
 * elements containing data to/from the pipeline. A data element
 * consists of the port name and a data element.
 *
 * When creating a adapter_data_set for input to the pipeline, use the
 * add_value() method to add a data value for the named port. The
 * names of the ports to the input process are specified in the
 * pipeline configuration file.
 *
 * When a adapter_data_set is returned from the output of a pipeline,
 * it contains one element from each connection to the output
 * process. Each element in the set is labeled with the port name as
 * specified in the pipeline configuration file.
 */
class KWIVER_ADAPTER_EXPORT adapter_data_set
{
public:
  typedef std::map< sprokit::process::port_t, sprokit::datum_t > datum_map_t;

  /**
   * @brief Type of data set.
   *
   * These are used to specify the payload in this data set. Usually
   * it contains data for the ports, but at the end it is marked with
   * end_of_input.
   *
   * Usually, sending an end_of_input element is not needed. Call the
   * embedded_pipeline::send_end_of_input() method to signal end of
   * input and terminate the pipeline processing.  In any event, no
   * data can be sent to an adapter after the end_of_input element has
   * been sent.
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
   * smart pointer. A factory method is used to enforce shared pointer
   * memory management for these objects. Allocating one of these
   * objects on the stack will not work.
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
  data_set_type type() const;

  /**
   * @brief Test if this object has end of data marker.
   *
   * This method is a convenient way to check if the type is the end
   * marker.
   *
   * @return \b true if this is end of data element.
   */
  bool is_end_of_data() const;

  /**
   * @brief Add datum to this data set.
   *
   * This method adds the specified port name and the datum to be
   * placed on that port to the data_set. If there is already a datum
   * in the set for the specified port, the data is overwritten with
   * the new value.
   *
   * @param port Name of the port where data is sent.
   * @param datum Sprokit datum object to be pushed to port.
   */
  void add_datum( sprokit::process::port_t const& port, sprokit::datum_t const& datum );

  /**
   * @brief Add typed value to data set.
   *
   * This method adds the specified value to the adapter data set. The
   * value is copied into the data set. This will overwrite the value
   * at the port
   *
   * @param port Name of the port where data is sent.
   * @param val Value to be wrapped in datum for port.
   */
  template <typename T>
  void add_value( ::sprokit::process::port_t const& port, T const& val );

  /**
   * @brief Query if data set is empty.
   *
   * This method tests if the data set is empty.
   *
   * @return \c true if the data set is empty (contains no values), otherwise
   * \c false.
   */
  bool empty() const;

  //@{
  /**
   * @brief Get begin iterator for items in this data set.
   *
   * An iterator can be used to inspect the elements of the data set.
   *
   * @return Begin iterator.
   */
  datum_map_t::iterator begin();
  datum_map_t::const_iterator begin() const;
  datum_map_t::const_iterator cbegin() const;
  //@}

  //@{
  /**
   * @brief Get ending iterator for items in this data set.
   *
   * An iterator can be used to inspect the elements of the data set.
   *
   * @return End iterator.
   */
  datum_map_t::iterator end();
  datum_map_t::const_iterator end() const;
  datum_map_t::const_iterator cend() const;
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
   * @return Iterator pointing at desired entry or end() iterator if
   * element not found.
   */
  datum_map_t::const_iterator find( sprokit::process::port_t const& port ) const;

  /**
   * @brief Get data value for specific port.
   *
   * This method returns the data value for the specified port.
   *
   * @param port Name of port
   *
   * @return Data value corresponding to the port.
   *
   * @throws std::runtime_error if the specified port name is not in this set.
   *
   * @throws sprokit::bad_datum_cast_exception if the requested data type does
   *         not match the actual type of the data from the port.
   */
  template<typename T>
  T value( ::sprokit::process::port_t const& port );


  /**
   * @brief Get data value for specific port, or default value if not found.
   *
   * This method returns the data value for the specified port.
   *
   * @param port Name of port
   *
   * @return Data value corresponding to the port, or \p value_if_missing if
   *         no such element is found.
   *
   * @throws sprokit::bad_datum_cast_exception if the requested data type does
   *         not match the actual type of the data from the port.
   */
  template< typename T >
  T value_or( ::sprokit::process::port_t const& port,
              T const& value_if_missing = {} );

  /**
   * @copydoc value
   *
   * @deprecated This method exists for historic reasons. Use #value instead.
   */
  template< typename T >
  [[ deprecated( "use value() instead" ) ]]
  T get_port_data( ::sprokit::process::port_t const& port )
  { return this->value< T >( port ); }

  /**
   * @brief Return the number of elements in the adapter_data_set.
   *
   * This method returns the number of elements stored in the adapter_data_set.
   * Similar to std::map::size()
   *
   * @return Number of elements in the adapter_data_set.
   */
  size_t size() const;

protected:
  KWIVER_ADAPTER_NO_EXPORT adapter_data_set( data_set_type type ); // private CTOR - use factory method

private:
  const data_set_type m_set_type;

  datum_map_t m_port_datum_set;

}; // end class adapter_datum

} } // end namespace

#endif // ADAPTER_DATA_SET
