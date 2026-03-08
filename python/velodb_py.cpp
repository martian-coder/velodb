/*
 * VeloDB: High-Performance Asynchronous Storage Engine
 * Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
 * 
 * This software and the associated Nexus Protocol (NXP) are the 
 * intellectual property of Amit Nilajkar.
 * Licensed under the MIT License. See LICENSE file for details.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "velodb/db.h"

namespace py = pybind11;
using namespace velodb;

PYBIND11_MODULE(velodb_py, m) {
    py::class_<DB>(m, "DB")
        .def(py::init<const std::string&, size_t>(),
             py::arg("data_dir"),
             py::arg("hot_buffer_bytes") = 64 << 20)
        .def("put", &DB::put, py::arg("key"), py::arg("value"))
        .def("get", [](DB& db, uint64_t key) {
            uint64_t v;
            if (!db.get(key, v)) throw py::value_error("Key not found");
            return v;
        }, py::arg("key"))
        .def("range", &DB::range,
             py::arg("lo"), py::arg("hi"))
        .def("snapshot", &DB::snapshot)
        .def("backup", &DB::backup, py::arg("path"));
}

