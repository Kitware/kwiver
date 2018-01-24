/*ckwg +29
 * Copyright 2012 by Kitware, Inc.
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

#include "gate_process.h"

#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/edge.h>
#include <sprokit/processes/kwiver_type_traits.h>

/**
 * \file gate_process.cxx
 *
 * \brief Implementation of the gate process.
 */

namespace sprokit
{

class gate_process::priv
{
  public:

    priv();
    ~priv();

    typedef port_t tag_t;
    typedef int32_t flag_t;

    static port_t const port_input;
    static port_t const port_true;
    static port_t const port_false;
    static tag_t const tag;
};

process::port_t const gate_process::priv::port_input = port_t("input");
process::port_t const gate_process::priv::port_true = port_t("true");
process::port_t const gate_process::priv::port_false = port_t("false");
gate_process::priv::tag_t const gate_process::priv::tag = tag_t("gate");

gate_process
::gate_process(kwiver::vital::config_block_sptr const& config)
  : process(config)
  , d(new priv)
{
  // Special cases are handled by the process.
  set_data_checking_level(check_sync);

  port_flags_t required;

  required.insert(flag_required);

  declare_input_port_using_trait( kwiver_logical, required );

  declare_input_port(
    priv::port_input,
    type_flow_dependent + priv::tag,
    required,
    port_description_t("The datum to route."));

  declare_output_port(
    priv::port_true,
    type_flow_dependent + priv::tag,
    required,
    port_description_t("The passed datum when test is true."));

  declare_output_port(
    priv::port_false,
    type_flow_dependent + priv::tag,
    required,
    port_description_t("The passed datum when test is false."));
}

gate_process
::~gate_process()
{
}

void
gate_process
::_step()
{
  datum_t const dat = grab_datum_from_port(priv::port_input);

  bool const complete = (dat->type() == datum::complete);

  if (complete) {
      // If we're complete, just push the "complete" datum on both ports
      push_datum_to_port(priv::port_true, dat);
      push_datum_to_port(priv::port_false, dat);
      mark_process_as_complete();
  } else {
    bool test = grab_from_port_using_trait( kwiver_logical );

    if (test == 0) {
      push_datum_to_port(priv::port_false, dat);
      push_datum_to_port(priv::port_true, sprokit::datum::empty_datum() );
    } else {
      push_datum_to_port(priv::port_true, dat);
      push_datum_to_port(priv::port_false, sprokit::datum::empty_datum() );
    }
  }

  process::_step();
}

gate_process::priv
::priv()
{
}

gate_process::priv
::~priv()
{
}

}
