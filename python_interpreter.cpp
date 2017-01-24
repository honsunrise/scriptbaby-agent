//
// Created by zhsyourai on 1/10/17.
//

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "python_interpreter.h"
#include "utils/hash_tools.h"

using namespace boost;

python_interpreter::python_interpreter() {
}

python_interpreter::~python_interpreter() {
}

void python_interpreter::run_on_path(const std::string &path) {
    boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path();
    std::string file_hash = hash_tools::file_hash_values(path);
    temp_dir /= file_hash;
    if (!boost::filesystem::exists(temp_dir)) {
        boost::filesystem::create_directories(temp_dir);
    }
    _help_decompression(path, temp_dir.generic_string());
    temp_dir /= "main.py";
    python_container *pc = new python_container(temp_dir.generic_string());
    tasks.push_back(pc);
    pc->run();
}

void python_interpreter::_help_decompression(const std::string &path, const std::string &dir) {
    archive_decompress decompress(path);
    decompress.extract_to_directory(dir);
}
