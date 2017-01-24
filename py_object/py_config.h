//
// Created by zhsyourai on 1/10/17.
//

#ifndef PONGO_AGENT_PY_CONFIG_H
#define PONGO_AGENT_PY_CONFIG_H

#include <string>
#include <map>
#include <boost/python.hpp>

class py_config {
public:
    virtual ~py_config();

    py_config();

    boost::python::dict get_config();
};

void export_py_config();

#endif //PONGO_AGENT_PY_CONFIG_H
