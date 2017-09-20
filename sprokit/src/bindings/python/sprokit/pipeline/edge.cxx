/*ckwg +29
 * Copyright 2011-2012 by Kitware, Inc.
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

#include <sprokit/pipeline/datum.h>
#include <sprokit/pipeline/edge.h>
#include <sprokit/pipeline/process.h>
#include <sprokit/pipeline/stamp.h>

#include "PyStamp.cxx"

#include <sprokit/python/util/python_gil.h>

#include <pybind11/stl_bind.h>

/**
 * \file edge.cxx
 *
 * \brief Python bindings for \link sprokit::edge\endlink.
 */

using namespace pybind11;

class PyEdgeDatum : public sprokit::edge_datum_t
{
  public:
    PyEdgeDatum() : sprokit::edge_datum_t() {}
    PyEdgeDatum(sprokit::datum datum, PyStamp stamp)
               : sprokit::edge_datum_t(
                 std::make_shared<sprokit::datum>(datum), stamp.get_stamp()) 
               {}
};

static void push_datum(sprokit::edge& self, PyEdgeDatum const& datum); 
static PyEdgeDatum get_datum(sprokit::edge& self);
static PyEdgeDatum peek_datum(sprokit::edge& self, size_t const& idx);

PYBIND11_MODULE(edge, m)
{

  class_<PyEdgeDatum>(m, "EdgeDatum")
    .def(init<>())
    .def(init<sprokit::datum, PyStamp>())
    .def_readwrite("datum", &sprokit::edge_datum_t::datum)
    .def_readwrite("stamp", &sprokit::edge_datum_t::stamp)
  ;
  bind_vector<std::vector<PyEdgeDatum> >(m, "EdgeData"
    , "A collection of data packets that may be passed through an edge.")
    .def(pybind11::init<>());
  bind_vector<std::vector<sprokit::edge_t> >(m, "Edges"
    , "A collection of edges.");

  class_<sprokit::edge>(m, "Edge"
    , "A communication channel between processes.")
    .def(init<>())
    .def(init<kwiver::vital::config_block_sptr>())
    .def("makes_dependency", &sprokit::edge::makes_dependency
      , "Returns True if the edge implies a dependency from downstream on upstream.")
    .def("has_data", &sprokit::edge::has_data
      , "Returns True if the edge contains data, False otherwise.")
    .def("full_of_data", &sprokit::edge::full_of_data
      , "Returns True if the edge cannot hold anymore data, False otherwise.")
    .def("datum_count", &sprokit::edge::datum_count
      , "Returns the number of data packets within the edge.")
    .def("push_datum", &push_datum
      , (arg("datum"))
      , "Pushes a datum packet into the edge.")
    .def("get_datum", &get_datum
      , "Returns the next datum packet from the edge, removing it in the process.")
    .def("peek_datum", &peek_datum
      , (arg("index") = 0)
      , "Returns the next datum packet from the edge.")
    .def("pop_datum", &sprokit::edge::pop_datum
      , "Remove the next datum packet from the edge.")
    .def("set_upstream_process", &sprokit::edge::set_upstream_process
      , (arg("process"))
      , "Set the process which is feeding data into the edge.")
    .def("set_downstream_process", &sprokit::edge::set_downstream_process
      , (arg("process"))
      , "Set the process which is reading data from the edge.")
    .def("mark_downstream_as_complete", &sprokit::edge::mark_downstream_as_complete
      , "Indicate that the downstream process is complete.")
    .def("is_downstream_complete", &sprokit::edge::is_downstream_complete
      , "Returns True if the downstream process is complete, False otherwise.")
    .def_readonly_static("config_dependency", &sprokit::edge::config_dependency)
    .def_readonly_static("config_capacity", &sprokit::edge::config_capacity)
  ;

}

void
push_datum(sprokit::edge& self, PyEdgeDatum const& datum)
{
  self.push_datum((sprokit::edge_datum_t) datum);
}

PyEdgeDatum
get_datum(sprokit::edge& self)
{
  sprokit::edge_datum_t datum = self.get_datum();
  PyEdgeDatum datum_p(*(datum.datum), PyStamp(datum.stamp));
  return datum_p;
}

PyEdgeDatum
peek_datum(sprokit::edge& self, size_t const& idx)
{
  sprokit::edge_datum_t datum = self.peek_datum(idx);
  PyEdgeDatum datum_p(*(datum.datum), PyStamp(datum.stamp));
  return datum_p;
}
