//
// Created by zhsyourai on 1/10/17.
//
#include <boost/python.hpp>
#include <boost/filesystem.hpp>
#include <ostream>
#include "python_container.h"
#include "py_object/py_config.h"
#include "py_object/py_cookies.h"
#include "py_object/py_logs.h"

using namespace boost::python;

python_container::~python_container() {
}

python_container::python_container(std::string &&path) : path(std::move(path)) {
    process = nullptr;
}


python_container::python_container(const std::string &path) : path(path) {
    process = nullptr;
}

// Python module define
using namespace boost::python;

BOOST_PYTHON_MODULE (pongo) {
    export_py_config();
    export_py_logs();
    export_py_cookies();
}

void python_container::run() {
    process = new subprocess::sub_process([&]() {
        try {
            Py_SetProgramName("pongo_worker");
            Py_InitializeEx(0);
            initpongo();
            object main_module = import("__main__");
            object main_namespace = main_module.attr("__dict__");
            main_namespace["pongo"] = import("pongo");
            object ignored = exec_file(path.c_str(), main_namespace);
            Py_Finalize();
        } catch (...) {
            PyErr_Print();
            PyErr_Clear();
        }
        exit(EXIT_SUCCESS);
    }, subprocess::defer_run{true}, subprocess::environment{
            {
                    {"PYTHONPATH", "."}
            }
    }, subprocess::cwd{boost::filesystem::path{path}.parent_path().generic_string()});
    process->start_process();
}
