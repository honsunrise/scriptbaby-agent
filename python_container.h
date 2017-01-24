//
// Created by zhsyourai on 1/10/17.
//

#ifndef PONGO_AGENT_PYTHON_CONTAINER_H
#define PONGO_AGENT_PYTHON_CONTAINER_H

#include <list>
#include "utils/sub_process.h"

class python_container {
public:
    python_container(std::string &&path);

    python_container(const std::string &path);

    virtual ~python_container();

    void run();
private:
    std::string path;
    subprocess::sub_process *process;
};


#endif //PONGO_AGENT_PYTHON_CONTAINER_H
