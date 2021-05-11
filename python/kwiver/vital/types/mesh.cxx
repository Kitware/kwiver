// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <vital/io/mesh_io.h>
#include <vital/types/mesh.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

namespace py = pybind11;

PYBIND11_MODULE(mesh, m)
{
  py::class_<kwiver::vital::mesh,
             std::shared_ptr<kwiver::vital::mesh> >(m, "Mesh")
    .def(py::init<>())
    .def("is_init",   &kwiver::vital::mesh::is_init)
    .def("num_verts", [](kwiver::vital::mesh& self)
    {
      if(self.is_init())
      {
        return self.num_verts();
      }
      return (unsigned int)0;
    })
    .def("num_faces", [](kwiver::vital::mesh& self)
    {
      if(self.is_init())
      {
        return self.num_faces();
      }
      return (unsigned int)0;
    })
    .def("num_edges", [](kwiver::vital::mesh& self)
    {
      if(self.is_init())
      {
        return self.num_edges();
      }
      return (unsigned int)0;
    })
    .def("has_tex_coords", &kwiver::vital::mesh::has_tex_coords)
    .def("tex_coords",   &kwiver::vital::mesh::tex_coords)
    .def("set_tex_source",   &kwiver::vital::mesh::set_tex_source)
    .def("compute_vertex_normals", &kwiver::vital::mesh::compute_vertex_normals)
    .def("compute_vertex_normals_from_faces",
         &kwiver::vital::mesh::compute_vertex_normals_from_faces)
    .def("compute_face_normals",
         &kwiver::vital::mesh::compute_face_normals, py::arg("norm") = true)
    .def("face_normals", [](kwiver::vital::mesh& self)
    {
        return self.faces().normals();
    })
    .def_static("from_ply_file", [](std::string path)
    {
      return kwiver::vital::read_ply(path);
    })
    .def_static("from_ply2_file", [](std::string path)
    {
      return kwiver::vital::read_ply2(path);
    })
    .def_static("from_obj_file", [](std::string path)
    {
      return kwiver::vital::read_obj(path);
    })
    .def_static("from_file", [](std::string path)
    {
      return kwiver::vital::read_mesh(path);
    })
    .def_static("to_ply2_file", [](std::string path, kwiver::vital::mesh& mesh)
    {
      return kwiver::vital::write_ply2(path, mesh);
    })
    .def_static("to_obj_file", [](std::string path, kwiver::vital::mesh& mesh)
    {
      return kwiver::vital::write_obj(path, mesh);
    })
    .def_static("to_kml_file", [](std::string path, kwiver::vital::mesh& mesh)
    {
      return kwiver::vital::write_kml(path, mesh);
    })
    .def_static("to_kml_collada_file", [](std::string path, kwiver::vital::mesh& mesh)
    {
      return kwiver::vital::write_kml_collada(path, mesh);
    })
    .def_static("to_vrml_file", [](std::string path, kwiver::vital::mesh& mesh)
    {
      return kwiver::vital::write_vrml(path, mesh);
    });

  py::enum_<kwiver::vital::mesh::tex_coord_type>(m, "tex_coord_type")
    .value("TEX_COORD_NONE", kwiver::vital::mesh::tex_coord_type::TEX_COORD_NONE)
    .value("TEX_COORD_ON_VERT", kwiver::vital::mesh::tex_coord_type::TEX_COORD_ON_VERT)
    .value("TEX_COORD_ON_CORNER", kwiver::vital::mesh::tex_coord_type::TEX_COORD_ON_CORNER);
}
