//
// Created by zhsyourai on 1/16/17.
//

#include <openssl/evp.h>
#include <boost/algorithm/hex.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/scope_exit.hpp>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include "hash_tools.h"

using namespace std;

std::string hash_tools::file_hash_values(const std::string &path) {
    try {
        string err_str;
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1) {
            err_str = "Could not open " + path + " (" + strerror(errno) + ")";
            throw runtime_error(err_str);
        }

        BOOST_SCOPE_EXIT_ALL(&) { close(fd); };

        const EVP_MD *md = EVP_sha1();
        if (!md)
            throw logic_error("Couldn't get SHA1 md");
        unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)> md_ctx(EVP_MD_CTX_create(),
                                                              &EVP_MD_CTX_destroy);
        if (!md_ctx)
            throw logic_error("Couldn't create md context");
        int r = EVP_DigestInit_ex(md_ctx.get(), md, 0);
        if (!r)
            throw logic_error("Could not init digest");
        vector<char> v(128 * 1024);
        for (;;) {
            ssize_t n = read(fd, v.data(), v.size());
            if (n < 0) {
                if (errno == EINTR)
                    continue;
                throw runtime_error(string("read error: ") + strerror(errno));
            }
            if (!n)
                break;
            r = EVP_DigestUpdate(md_ctx.get(), v.data(), (size_t) n);
            if (!r)
                throw logic_error("Digest update failed");
        }
        array<unsigned char, EVP_MAX_MD_SIZE> hash;
        unsigned int n = 0;
        r = EVP_DigestFinal_ex(md_ctx.get(), hash.data(), &n);
        if (!r)
            throw logic_error("Could not finalize digest");
        std::stringstream ret_hash;
        boost::algorithm::hex(boost::make_iterator_range(
                reinterpret_cast<const char *>(hash.data()),
                reinterpret_cast<const char *>(hash.data() + n)),
                              std::ostream_iterator<char>(ret_hash));
        return ret_hash.str();
    } catch (const exception &e) {
        return "";
    }
    return "";
}
