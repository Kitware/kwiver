#include <sprokit/processes/adapters/embedded_pipeline.h>
#include <sprokit/pipeline_util/load_pipe_exception.h>

#include <python/kwiver/vital/util/pybind11.h>

#include <memory>
#include <fstream>
using namespace pybind11;

// Publicist class to access protected methods
class wrap_embedded_pipeline
  : public kwiver::embedded_pipeline
{
public:
  using embedded_pipeline::connect_input_adapter;
  using embedded_pipeline::connect_output_adapter;
  using embedded_pipeline::update_config;
};



// Trampoline class to allow us to use virtual methods
class embedded_pipeline_trampoline
  : public kwiver::embedded_pipeline
{
public:
  using embedded_pipeline::embedded_pipeline;

  bool connect_input_adapter() override;
  bool connect_output_adapter() override;
  void update_config(kwiver::vital::config_block_sptr config) override;
};


void build_pipeline(kwiver::embedded_pipeline& self, kwiver::vital::path_t const& desc_file);


PYBIND11_MODULE(embedded_pipeline, m)
{

class_< kwiver::embedded_pipeline,
        std::shared_ptr<kwiver::embedded_pipeline>,
        embedded_pipeline_trampoline>(m, "EmbeddedPipeline")
  .def(init<>())
  .def("build_pipeline", &build_pipeline)
  .def("send", &kwiver::embedded_pipeline::send)
  .def("send_end_of_input", &kwiver::embedded_pipeline::send_end_of_input)
  .def("receive", &kwiver::embedded_pipeline::receive)
  .def("full", &kwiver::embedded_pipeline::full)
  .def("empty", &kwiver::embedded_pipeline::empty)
  .def("at_end", &kwiver::embedded_pipeline::at_end)
  .def("start", &kwiver::embedded_pipeline::start)
  .def("wait", &kwiver::embedded_pipeline::wait)
  .def("stop", &kwiver::embedded_pipeline::stop)
  .def("input_port_names", &kwiver::embedded_pipeline::input_port_names)
  .def("output_port_names", &kwiver::embedded_pipeline::output_port_names)
  .def("input_adapter_connected", &wrap_embedded_pipeline::input_adapter_connected)
  .def("output_adapter_connected", &wrap_embedded_pipeline::output_adapter_connected)
  .def("connect_input_adapter", static_cast<bool (kwiver::embedded_pipeline::*)()>(&wrap_embedded_pipeline::connect_input_adapter))
  .def("connect_output_adapter", static_cast<bool (kwiver::embedded_pipeline::*)()>(&wrap_embedded_pipeline::connect_output_adapter))
  .def("update_config", static_cast<void (kwiver::embedded_pipeline::*)(kwiver::vital::config_block_sptr)>(&wrap_embedded_pipeline::update_config))
  ;
}

bool
embedded_pipeline_trampoline
::connect_input_adapter()
{
  VITAL_PYBIND11_OVERLOAD(
    bool,
    embedded_pipeline,
    connect_input_adapter,
  );
}

bool
embedded_pipeline_trampoline
::connect_output_adapter()
{
  VITAL_PYBIND11_OVERLOAD(
    bool,
    embedded_pipeline,
    connect_output_adapter,
  );
}

void
embedded_pipeline_trampoline
::update_config(kwiver::vital::config_block_sptr config)
{
  VITAL_PYBIND11_OVERLOAD(
    void,
    embedded_pipeline,
    update_config,
    config
  );
}

void build_pipeline(kwiver::embedded_pipeline& self, kwiver::vital::path_t const& desc_file)
{
  std::ifstream desc_stream(desc_file);
  if (! desc_stream )
  {
    throw sprokit::file_no_exist_exception(desc_file);
  }
  self.build_pipeline(desc_stream);
}
