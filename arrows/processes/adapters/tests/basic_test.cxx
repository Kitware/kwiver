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

#include <vital/config/config_block.h>
#include <vital/vital_foreach.h>

#include <sprokit/tools/pipeline_builder.h>
#include <sprokit/tools/literal_pipeline.h>
#include <sprokit/pipeline/modules.h>
#include <sprokit/pipeline/pipeline.h>
#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/scheduler.h>
#include <sprokit/pipeline/scheduler_registry.h>

#include <arrows/processes/adapters/input_adapter.h>
#include <arrows/processes/adapters/input_adapter_process.h>

#include <arrows/processes/adapters/output_adapter.h>
#include <arrows/processes/adapters/output_adapter_process.h>

#include <sstream>


static kwiver::vital::config_block_key_t const scheduler_block = kwiver::vital::config_block_key_t("_scheduler");


int main(int argc, char *argv[])
{
  kwiver::input_adapter input_ad;
  kwiver::output_adapter output_ad;

  // load processes
  sprokit::load_known_modules();

  // Use SPROKIT macros to create pipeline description
  std::stringstream pipeline_desc;
  pipeline_desc << SPROKIT_PROCESS( "input_adapter",  "ia" )
                << SPROKIT_PROCESS( "output_adapter", "oa" )

                << SPROKIT_CONNECT( "ia", "port1",    "oa", "port1" )
                << SPROKIT_CONNECT( "ia", "port2",    "oa", "port3" ) // yeah, i know
                << SPROKIT_CONNECT( "ia", "port3",    "oa", "port2" )
    ;

    // create a pipeline
  sprokit::pipeline_builder builder;
  builder.load_pipeline( pipeline_desc );

  // build pipeline
  sprokit::pipeline_t const pipe = builder.pipeline();
  kwiver::vital::config_block_sptr const conf = builder.config();

  if (!pipe)
  {
    std::cerr << "Error: Unable to bake pipeline" << std::endl;
    return EXIT_FAILURE;
  }

  // perform setup operation on pipeline and get it ready to run
  // This throws many exceptions
  try
  {
    pipe->setup_pipeline();
  }
  catch( sprokit::pipeline_exception const& e)
  {
    std::cerr << "Error setting up pipeline: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Connect adapters to their processes.
  input_ad.connect( "ia", pipe );
  output_ad.connect( "oa", pipe );

  // Query adapters for ports
  auto input_list = input_ad.port_list();
  std::cout << "Input adapter ports:\n";
  VITAL_FOREACH( auto port, input_list )
  {
    std::cout << "    " << port << "\n";
  }

  auto output_list = output_ad.port_list();
  std::cout << "\nOutput adapter ports:\n";
  VITAL_FOREACH( auto port, output_list )
  {
    std::cout << "    " << port << "\n";
  }

  sprokit::scheduler_registry::type_t scheduler_type = sprokit::scheduler_registry::default_type;
  kwiver::vital::config_block_sptr const scheduler_config = conf->subblock(scheduler_block +
                                              kwiver::vital::config_block::block_sep + scheduler_type);

  sprokit::scheduler_registry_t reg = sprokit::scheduler_registry::self();
  sprokit::scheduler_t scheduler = reg->create_scheduler(scheduler_type, pipe, scheduler_config);

  if (!scheduler)
  {
    std::cerr << "Error: Unable to create scheduler" << std::endl;
    return EXIT_FAILURE;
  }

  scheduler->start();

  // Feed data to input adapter
  for ( int i = 0; i < 10; ++i)
  {
    auto ds = kwiver::adapter::adapter_data_set::create();
    int val = i;

    VITAL_FOREACH( auto port, input_list )
    {
      ds->add_value( port, (val++) );
    }
    std::cout << "sending set: " << i << "\n";
    input_ad.send( ds );
  }

  std::cout << "Sending end of input element\n";
  auto ds = kwiver::adapter::adapter_data_set::create( kwiver::adapter::adapter_data_set::end_of_input );
  input_ad.send( ds );

  // get output from pipeline
  //+ while ( ! output_ad.empty() )
  //+ for ( int i = 0; i < 10; ++i)
  while( true )
  {
    auto ods = output_ad.receive(); // blocks

    // check for end of data marker
    if (ods->type() == kwiver::adapter::adapter_data_set::end_of_input)
    {
      std::cout << "End of data detected\n";
      break;
    }

    auto ix = ods->begin();
    auto eix = ods->end();

    std::cout << "\nData from pipeline\n";

    for ( ; ix != eix; ++ix )
    {
      std::cout << "   port: " << ix->first << "  value: " << ix->second->get_datum<int>() << "\n";
    }
  }

  scheduler->wait();

  return 0;
}
