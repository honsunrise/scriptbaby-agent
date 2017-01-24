//
// Created by zhsyourai on 1/16/17.
//

#ifndef PONGO_AGENT_ARCHIVE_DECOMPRESS_H
#define PONGO_AGENT_ARCHIVE_DECOMPRESS_H

#include <string>
#include <vector>
#include <archive.h>
#include <archive_entry.h>

class archive_decompress {
public:
    archive_decompress(const std::string &archive_file_name);

    archive_decompress(unsigned char *in_buffer, const size_t size);

    archive_decompress(std::vector<unsigned char> &&in_buffer);

    ~archive_decompress();

    bool extract_next(const std::string &root_path);

    std::pair <std::string, std::vector<unsigned char>> extract_next();

    void extract_to_directory(const std::string &root_path);
private:
    void init();

    template <typename T>
    void check_error(T err_code, const bool close_before_throw = false);

    void close();

    struct archive *_archive;
    bool _open;

    const std::string _archive_file_name;
    std::vector<unsigned char> _in_buffer;
};


#endif //PONGO_AGENT_ARCHIVE_DECOMPRESS_H
