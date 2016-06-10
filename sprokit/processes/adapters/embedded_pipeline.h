
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


#ifndef ARROWS_PROCESSES_EMBEDDED_PIPELINE_H
#define ARROWS_PROCESSES_EMBEDDED_PIPELINE_H

#include <sprokit/processes/adapters/kwiver_adapter_export.h>

#include "adapter_data_set.h"

#include <istream>
#include <string>

namespace kwiver {

// -----------------------------------------------------------------
/**
 * @brief Embedded callable pipeline.
 *
 * This class implements a sprokit pipeline that can be instantiated
 * within a program.
 *
 * Inputs to the pipeline are passed to the input adapter through the
 * send() method. Outputs from the pipeline are retrieved using the
 * receive() method.
 *
 * Pipeline description must contain only one input adapter (process
 * type "input_adapter") and only one output adapter (process type
 * "output_adapter"). The actual process names are up to you.

 * Connecting the adapters. port names.
 *
 * Example:
\code
  #include <sprokit/tools/literal_pipeline.h>

  // Use SPROKIT macros to create pipeline description
  std::stringstream pipeline_desc;
  pipeline_desc << SPROKIT_PROCESS( "input_adapter",  "ia" )
                << SPROKIT_PROCESS( "output_adapter", "oa" )

                << SPROKIT_CONNECT( "ia", "port1",    "oa", "port1" )
                << SPROKIT_CONNECT( "ia", "port2",    "oa", "port3" )
                << SPROKIT_CONNECT( "ia", "port3",    "oa", "port2" )
    ;

  // create embedded pipeline
  kwiver::embedded_pipeline ep( pipeline_desc );

  // Query adapters for ports
  auto input_list = ep.input_port_names();
  auto output_list = ep.output_port_names();

  // Verify ports are as expected
  // ...

  // Start pipeline
  ep.start();

  for ( int i = 0; i < 10; ++i)
  {
    // Create dataset for input
    auto ds = kwiver::adapter::adapter_data_set::create();

    ds.add_value( "counter", i );
    ep.send( ds ); // push into pipeline
  }

  ep.send_end_of_input(); // indicate end of input

\endcode
 */
class KWIVER_ADAPTER_EXPORT embedded_pipeline
{
public:
  /**
   * @brief Create embedded pipeline from description in stream.
   *
   * @param istr Input stream containing the pipeline description.
   */
  embedded_pipeline( std::istream& istr );
  virtual ~embedded_pipeline();

  /**
   * @brief Send data set to input adapter.
   *
   * This method sends a data set object to the input adapter. The
   * adapter data set must contain a datum for each port on the input
   * adapter process.
   *
   * If the pipeline is full and can not accept the data set, this
   * method will block until the pipeline can accept the input.
   *
   * The end-of-data item is sent to the pipeline after the last data
   * item to indicate that there are no more data ant the pipeline
   * should start an orderly termination. Passing more data after the
   * end-of-data set has been sent is not a good idea.
   *
   * @param ads Data set to send
   */
  void send( kwiver::adapter::adapter_data_set_t ads );

  /**
   * @brief Send end of input into pipeline.
   *
   * This method indicates that there will be no more input into the
   * pipeline. The pipeline starts to shutdown after this method is
   * called. Calling send() after this method is called is not a good
   * idea.
   */
  void send_end_of_input();

  /**
   * @brief Get pipeline output data.
   *
   * This method returns a data set produced by the pipeline. It will
   * contain one entry for each port on the output adapter process.
   *
   * If the is no output data set immediately available, this call
   * will block until one is available.
   *
   * The last data set from the pipeline will be marked as end of data
   * (is_end_of_data() returns true). Calling this method after the
   * end of data item has been returned is not a good idea.
   *
   * @return Data set from the pipeline.
   */
  kwiver::adapter::adapter_data_set_t receive();

  /**
   * @brief Can pipeline accept more input?
   *
   * This method checks to see if the input adapter process can accept
   * more data.
   *
   * @return \b true if interface queue is full and a send() call would wait.
   */
  bool full() const;

  /**
   * @brief Is any pipeline output ready?
   *
   * This method checks to see if there is a pipeline output data set ready.
   *
   * @return \b true if interface queue is full and thread would wait for receive().
   */
  bool empty() const;

  /**
   * @brief Is pipeline terminated.
   *
   * This method returns true if the end of input marker has been
   * retrieved from the pipeline, indicating that the pipeline has
   * processed all the data and terminated.
   *
   * @return \b true if all data has been processed and pipeline has terminated.
   */
  bool at_end() const;

  /**
   * @brief Start the pipeline
   *
   * This method starts the pipeline processing. After this call, the
   * pipeline is ready to accept input data sets.
   */
  void start();


  /**
   * @brief Wait for pipeline to complete.
   *
   * This method waits until the pipeline scheduler terminates. This
   * is useful when terminating an embedded pipeline to make sure that
   * all threads have terminated.
   *
   * Calling this before sending an end-of-input has been sent to the
   * pipeline will block the caller until the pipeline terminates.
   */
  void wait();

  /**
   * @brief Get list of input ports.
   *
   * This method returns the list of all active data ports on the
   * input adapter. This list is used to drive the adapter_data_set
   * creation so that here is a datum of the correct type for each
   * port.
   *
   * The actual port names are specified in the pipeline
   * configuration.
   *
   * @return List of input port names
   */
  sprokit::process::ports_t input_port_names() const;

  /**
   * @brief Get list of output ports.
   *
   * This method returns the list of all active data ports on the
   * output adapter. This list is used to process the
   * adapter_data_set.  There will be a datum for each output port in
   * the returned data set.
   *
   * The actual port names are specified in the pipeline
   * configuration.
   *
   * @return List of output port names
   */
  sprokit::process::ports_t output_port_names() const;

private:
  class priv;
  std::unique_ptr< priv > m_priv;

}; // end class embedded_pipeline

} // end namespace

#endif /* ARROWS_PROCESSES_EMBEDDED_PIPELINE_H */
