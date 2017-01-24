//
// Created by zhsyourai on 1/10/17.
//

#ifndef PONGO_AGENT_PYTHONINTERPRETER_H
#define PONGO_AGENT_PYTHONINTERPRETER_H

#include <Python.h>
#include <string>
#include "python_container.h"
#include "utils/archive_decompress.h"

class python_interpreter {
public:
    python_interpreter();

    virtual ~python_interpreter();

    void run_on_path(const std::string &path);

private:

    void _help_decompression(const std::string &path, const std::string &dir);

    std::list<python_container*> tasks;
};


#endif //PONGO_AGENT_PYTHONINTERPRETER_H
