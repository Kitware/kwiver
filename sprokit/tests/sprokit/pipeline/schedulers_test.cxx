// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <sprokit/pipeline/scheduler.h>
#include <sprokit/pipeline/scheduler_factory.h>

#include "schedulers_test_export.h"

#include <memory>

using namespace sprokit;

class SCHEDULERS_TEST_NO_EXPORT test_scheduler
  : public sprokit::scheduler
{
  public:
    test_scheduler(pipeline_t const& pipe, kwiver::vital::config_block_sptr const& config);
    ~test_scheduler();
  protected:
    void _start();
    void _wait();
    void _pause();
    void _resume();
    void _stop();
};

test_scheduler
::test_scheduler(pipeline_t const& pipe, kwiver::vital::config_block_sptr const& config)
  : scheduler(pipe, config)
{
}

test_scheduler
::~test_scheduler()
{
}

void
test_scheduler
::_start()
{
}

void
test_scheduler
::_wait()
{
}

void
test_scheduler
::_pause()
{
}

void
test_scheduler
::_resume()
{
}

void
test_scheduler
::_stop()
{
}

extern "C"
SCHEDULERS_TEST_EXPORT
void
register_factories( kwiver::vital::plugin_loader& vpm )
{
  sprokit::scheduler_registrar reg( vpm, "test_schedulers" );

  if ( reg.is_module_loaded() )
  {
    return;
  }

  reg.register_scheduler< test_scheduler >(
    "test",
    "A test scheduler" );

  reg.mark_module_as_loaded();
}
