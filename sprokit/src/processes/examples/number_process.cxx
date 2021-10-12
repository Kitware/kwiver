/*ckwg +29
 * Copyright 2011-2018 by Kitware, Inc.
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

#include "number_process.h"

#include <vital/config/config_block.h>
#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/process_exception.h>

#include <stdint.h>

#include <string>

/**
 * \file number_process.cxx
 *
 * \brief Implementation of the number process.
 */

namespace sprokit
{

class number_process::priv
{
  public:
    typedef int32_t number_t;

    priv(number_t s, number_t e);
    ~priv();

    number_t const start;
    number_t const end;

    number_t current;

    static kwiver::vital::config_block_key_t const config_start;
    static kwiver::vital::config_block_key_t const config_end;
    static kwiver::vital::config_block_value_t const default_start;
    static kwiver::vital::config_block_value_t const default_end;
    static port_t const port_output;
};

kwiver::vital::config_block_key_t const number_process::priv::config_start = kwiver::vital::config_block_key_t("start");
kwiver::vital::config_block_key_t const number_process::priv::config_end = kwiver::vital::config_block_key_t("end");
kwiver::vital::config_block_value_t const number_process::priv::default_start = kwiver::vital::config_block_value_t("0");
kwiver::vital::config_block_value_t const number_process::priv::default_end = kwiver::vital::config_block_value_t("100");
process::port_t const number_process::priv::port_output = port_t("number");

number_process
::number_process(kwiver::vital::config_block_sptr const& config)
  : process(config)
  , d()
{
  declare_configuration_key(
    priv::config_start,
    priv::default_start,
    kwiver::vital::config_block_description_t("The value to start counting at."));

  declare_configuration_key(
    priv::config_end,
    priv::default_end,
    kwiver::vital::config_block_description_t("The value to stop counting at."));

  port_flags_t required;

  required.insert(flag_required);

  declare_output_port(
    priv::port_output,
    "integer",
    required,
    port_description_t("Where the numbers will be available."));
}

number_process
::~number_process()
{
}

void
number_process
::_configure()
{
  // Configure the process.
  {
    priv::number_t const start = config_value<priv::number_t>(priv::config_start);
    priv::number_t const end = config_value<priv::number_t>(priv::config_end);

    d.reset(new priv(start, end));
  }

  // Check the configuration.
  if (d->end <= d->start)
  {
    static std::string const reason = "The start value must be greater than the end value";

    VITAL_THROW( invalid_configuration_exception,
                 name(), reason);
  }

  process::_configure();
}

void
number_process
::_step()
{
  datum_t dat;

  if (d->current == d->end)
  {
    mark_process_as_complete();
    dat = datum::complete_datum();
  }
  else
  {
    dat = datum::new_datum(d->current);

    ++d->current;
  }

  push_datum_to_port(priv::port_output, dat);

  process::_step();
}

number_process::priv
::priv(number_t s, number_t e)
  : start(s)
  , end(e)
  , current(s)
{
}

number_process::priv
::~priv()
{
}

}
