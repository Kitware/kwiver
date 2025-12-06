// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Interface file for the subprocess embedded pipeline.
 */

#ifndef ARROWS_PROCESSES_SUBPROCESS_EMBEDDED_PIPELINE_H
#define ARROWS_PROCESSES_SUBPROCESS_EMBEDDED_PIPELINE_H

#include <sprokit/processes/adapters/kwiver_adapter_export.h>

#include "embedded_pipeline.h"
#include "adapter_data_set.h"

#include <vital/logger/logger.h>

#include <istream>
#include <string>
#include <memory>

namespace kwiver {

// -----------------------------------------------------------------
/**
 * @brief Embedded pipeline that runs in a separate process.
 *
 * This class extends embedded_pipeline to run the pipeline in a
 * separate operating system process. This provides process isolation,
 * which is useful for:
 * - Isolating GPU memory usage
 * - Preventing crashes in the pipeline from affecting the main process
 * - Running pipelines that may have conflicting library dependencies
 *
 * The subprocess communication uses pipes (stdin/stdout) with serialized
 * data for cross-platform compatibility (works on both Windows and Linux).
 *
 * Usage is similar to embedded_pipeline:
 * \code
 *   kwiver::subprocess_embedded_pipeline ep;
 *   ep.build_pipeline( pipeline_desc );
 *   ep.start();
 *
 *   // Send data to pipeline
 *   auto ds = kwiver::adapter::adapter_data_set::create();
 *   ds.add_value( "port_name", data );
 *   ep.send( ds );
 *
 *   // Receive output
 *   auto result = ep.receive();
 *
 *   ep.send_end_of_input();
 *   ep.wait();
 * \endcode
 *
 * The subprocess is automatically terminated when the object is destroyed.
 */
class KWIVER_ADAPTER_EXPORT subprocess_embedded_pipeline
  : public embedded_pipeline
{
public:
  /**
   * @brief Create subprocess embedded pipeline.
   *
   * Creates a new subprocess embedded pipeline. The subprocess is not
   * started until start() is called.
   */
  subprocess_embedded_pipeline();

  virtual ~subprocess_embedded_pipeline();

  /**
   * @brief Build the pipeline for subprocess execution.
   *
   * This method prepares the pipeline description but does not start
   * the subprocess. The actual subprocess is spawned when start() is called.
   *
   * @param istr Input stream containing the pipeline description.
   * @param def_dir Directory for resolving relative paths (defaults to cwd).
   *
   * @throws std::runtime_error on configuration errors.
   */
  void build_pipeline( std::istream& istr, std::string const& def_dir = "" ) override;

  /**
   * @brief Send data set to the subprocess pipeline.
   *
   * Serializes and sends the data set to the subprocess via pipe.
   * Blocks if the pipe buffer is full.
   *
   * @param ads Data set to send.
   *
   * @throws std::runtime_error if subprocess is not running or on I/O error.
   */
  void send( kwiver::adapter::adapter_data_set_t ads );

  /**
   * @brief Signal end of input to subprocess.
   *
   * Sends an end-of-input marker to the subprocess, signaling that
   * no more data will be sent.
   */
  void send_end_of_input();

  /**
   * @brief Receive data from subprocess pipeline.
   *
   * Blocks until data is available from the subprocess output.
   *
   * @return Data set from the pipeline output.
   *
   * @throws std::runtime_error if subprocess has terminated unexpectedly.
   */
  kwiver::adapter::adapter_data_set_t receive();

  /**
   * @brief Check if subprocess input buffer is full.
   *
   * @return true if sending would block.
   */
  bool full() const;

  /**
   * @brief Check if subprocess output is available.
   *
   * @return true if no output is currently available.
   */
  bool empty() const;

  /**
   * @brief Check if subprocess has completed.
   *
   * @return true if the subprocess has finished processing all data.
   */
  bool at_end() const;

  /**
   * @brief Start the subprocess.
   *
   * Spawns the worker subprocess and establishes communication channels.
   * The pipeline configuration is sent to the subprocess which then
   * begins execution.
   *
   * @throws std::runtime_error if subprocess cannot be started.
   */
  void start();

  /**
   * @brief Wait for subprocess to complete.
   *
   * Blocks until the subprocess terminates. Should be called after
   * send_end_of_input() to ensure clean shutdown.
   */
  void wait();

  /**
   * @brief Stop the subprocess.
   *
   * Forcefully terminates the subprocess if it is still running.
   */
  void stop();

  /**
   * @brief Get list of input port names.
   *
   * @return List of port names for the input adapter.
   */
  sprokit::process::ports_t input_port_names() const;

  /**
   * @brief Get list of output port names.
   *
   * @return List of port names for the output adapter.
   */
  sprokit::process::ports_t output_port_names() const;

  /**
   * @brief Check if input adapter is connected in the pipeline.
   *
   * @return true if input adapter exists in pipeline configuration.
   */
  bool input_adapter_connected() const;

  /**
   * @brief Check if output adapter is connected in the pipeline.
   *
   * @return true if output adapter exists in pipeline configuration.
   */
  bool output_adapter_connected() const;

  /**
   * @brief Set path to the pipeline worker executable.
   *
   * By default, the system searches for 'kwiver_pipeline_worker' in the
   * PATH and standard installation directories. Use this method to
   * specify an alternate location.
   *
   * @param path Full path to the worker executable.
   */
  void set_worker_path( std::string const& path );

  /**
   * @brief Get the exit code of the subprocess.
   *
   * Valid only after the subprocess has terminated.
   *
   * @return Exit code from the subprocess, or -1 if not available.
   */
  int subprocess_exit_code() const;

  /**
   * @brief Check if subprocess is currently running.
   *
   * @return true if the subprocess is active.
   */
  bool subprocess_running() const;

  class priv;

private:
  std::unique_ptr< priv > m_priv;

}; // end class subprocess_embedded_pipeline

} // end namespace kwiver

#endif /* ARROWS_PROCESSES_SUBPROCESS_EMBEDDED_PIPELINE_H */
