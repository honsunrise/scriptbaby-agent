//
// Created by zhsyourai on 1/16/17.
//

#include <stdexcept>
#include <boost/scope_exit.hpp>
#include "archive_decompress.h"

archive_decompress::archive_decompress(const std::string &archive_file_name)
        : _archive_file_name(archive_file_name), _archive(archive_read_new()), _open(true) {
    struct stat file_stat;
    if (stat(archive_file_name.c_str(), &file_stat) < 0) {
        switch (errno) {
            case ENOENT:
                throw std::runtime_error("Archive file not found.");
            default:
                throw std::runtime_error("Archive file is not valid.");
        }
    }

    init();
    check_error(archive_read_open_filename(_archive, archive_file_name.c_str(), 10240), true);
}

archive_decompress::archive_decompress(unsigned char *in_buffer, const size_t size)
        : _archive_file_name(""), _archive(archive_read_new()), _open(true) {
    init();
    check_error(archive_read_open_memory(_archive, in_buffer, size), true);
}

archive_decompress::archive_decompress(std::vector<unsigned char> &&in_buffer)
        : _archive_file_name(""), _in_buffer(std::move(in_buffer)), _archive(archive_read_new()), _open(true) {
    init();
    check_error(archive_read_open_memory(_archive, &*in_buffer.begin(), in_buffer.size()), true);
}

void archive_decompress::init() {
    check_error(archive_read_support_format_all(_archive), true);
    check_error(archive_read_support_compression_all(_archive), true);
}

archive_decompress::~archive_decompress() {
    close();
}

la_ssize_t copy_data(struct archive *ar, struct archive *aw) {
    la_ssize_t r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK)
            return (r);
    }
}

bool archive_decompress::extract_next(const std::string &root_path) {
    int flags;

    /* Select which attributes we want to restore. */
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    struct archive *a = archive_write_disk_new();
    BOOST_SCOPE_EXIT_ALL(&a) {
                                 if (a != NULL) {
                                     archive_write_finish_entry(a);
                                     archive_write_close(a);
                                     archive_write_free(a);
                                 }
                             };
    archive_write_disk_set_options(a, flags);
    archive_write_disk_set_standard_lookup(a);

    struct archive_entry *entry;
    auto r = archive_read_next_header(_archive, &entry);

    if (r == ARCHIVE_EOF)
        return false;
    else
        check_error(r);

    archive_entry_set_pathname(entry,
                               (root_path + "/" + archive_entry_pathname(entry)).c_str());
    check_error(archive_write_header(a, entry));
    if (archive_entry_size(entry) > 0)
        check_error(copy_data(_archive, a));
    return true;
}

void archive_decompress::extract_to_directory(const std::string &root_path) {
    while (extract_next(root_path)) {}
}

std::pair<std::string, std::vector<unsigned char>> archive_decompress::extract_next() {
    auto result = std::make_pair(std::string(""), std::vector<unsigned char>());

    struct archive_entry *entry;
    auto r = archive_read_next_header(_archive, &entry);
    if (r == ARCHIVE_EOF)
        return result;
    else
        check_error(r);

    result.first = archive_entry_pathname(entry);
    auto entry_size = archive_entry_size(entry);
    if (entry_size > 0) {
        la_ssize_t r_d;
        size_t read_index = 0;
        result.second.resize((unsigned long) entry_size);
        for (;;) {
            r_d = archive_read_data(_archive, &result.second[read_index], result.second.size() - read_index);
            if (r_d == 0)
                break;
            if (r_d < ARCHIVE_OK)
                check_error(r_d);

            read_index += r_d;
            if (read_index == entry_size)
                break;
        }
    }

    return result;
}

template<typename T>
void archive_decompress::check_error(T err_code, const bool close_before_throw) {
    std::string error_str;
    if (err_code != ARCHIVE_OK && err_code != ARCHIVE_WARN)
        error_str = archive_error_string(_archive);
    if (close_before_throw && err_code == ARCHIVE_FATAL)
        close();
    if (err_code != ARCHIVE_OK && err_code != ARCHIVE_WARN)
        throw std::runtime_error(error_str);
}

void archive_decompress::close() {
    if (_open) {
        if (_archive != NULL) {
            archive_read_close(_archive);
            archive_read_free(_archive);
        }

        _open = false;
    }
}
