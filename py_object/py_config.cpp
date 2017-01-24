//
// Created by zhsyourai on 1/10/17.
//

#include <boost/python.hpp>
#include "py_config.h"

py_config::~py_config() {

}

py_config::py_config() {

}

boost::python::dict py_config::get_config() {
    boost::python::dict d;
    d["a"] = 'b';
    d["b"] = 1;
    return d;
}


void export_py_config() {
    boost::python::class_<py_config>("Config")
            .def("get_config", &py_config::get_config);
}
