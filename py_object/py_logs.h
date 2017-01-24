//
// Created by zhsyourai on 1/10/17.
//

#ifndef PONGO_AGENT_PY_LOGS_H
#define PONGO_AGENT_PY_LOGS_H

#include <string>

class py_logs {
public:
    py_logs();

    virtual ~py_logs();

    void flush();

    void write(std::string str);
};

void export_py_logs();

#endif //PONGO_AGENT_PY_LOGS_H
