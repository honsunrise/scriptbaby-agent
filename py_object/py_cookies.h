//
// Created by zhsyourai on 1/10/17.
//

#ifndef PONGO_AGENT_PY_COOKIE_H
#define PONGO_AGENT_PY_COOKIE_H

#include <string>

class py_cookies {
public:
    py_cookies();

    virtual ~py_cookies();

    std::string load_cookie(std::string cookie_key);

    void save_cookie(std::string cookie_key, std::string cookie);
};

void export_py_cookies();

#endif //PONGO_AGENT_PY_COOKIE_H
