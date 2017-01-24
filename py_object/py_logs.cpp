//
// Created by zhsyourai on 1/10/17.
//

#include <boost/python.hpp>
#include "py_logs.h"

py_logs::py_logs() {

}

py_logs::~py_logs() {

}

void py_logs::flush() {

}

void py_logs::write(std::string str) {

}


void export_py_logs() {
    boost::python::class_<py_logs>("Logs")
            .def("flush", &py_logs::flush)
            .def("write", &py_logs::write);
}