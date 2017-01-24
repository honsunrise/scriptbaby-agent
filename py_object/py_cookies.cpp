//
// Created by zhsyourai on 1/10/17.
//

#include <boost/python.hpp>
#include "py_cookies.h"

py_cookies::py_cookies() {

}

py_cookies::~py_cookies() {

}

std::string py_cookies::load_cookie(std::string cookie_key) {
    return "";
}

void py_cookies::save_cookie(std::string cookie_key, std::string cookie) {

}


void export_py_cookies() {
    boost::python::class_<py_cookies>("Cookies")
            .def("load_cookie", &py_cookies::load_cookie)
            .def("save_cookie", &py_cookies::save_cookie);
}