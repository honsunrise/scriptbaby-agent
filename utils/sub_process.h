//
// Created by zhsyourai on 1/12/17.
//

#ifndef PONGO_AGENT_SUB_PROCESS_H
#define PONGO_AGENT_SUB_PROCESS_H

#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <future>
#include <vector>
#include <sstream>
#include <memory>
#include <initializer_list>
#include <exception>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
}


namespace subprocess {
    static const size_t SP_MAX_ERR_BUF_SIZ = 1024;
    static const size_t DEFAULT_BUF_CAP_BYTES = 8192;

    class ChildProcessError : public std::runtime_error {
    public:
        ChildProcessError(const std::string &error_msg) :
                std::runtime_error(error_msg) {}
    };


    class OSError : public std::runtime_error {
    public:
        OSError(const std::string &err_msg, int err_code) :
                std::runtime_error(err_msg + " : " + std::strerror(err_code)) {}
    };

    /* -------------------------------
     *     Subprocess Arguments
     * -------------------------------
     */

    enum IO_TYPE {
        STDIN = 0,
        STDOUT,
        STDERR,
        PIPE,
        FILE_PATH
    };

    struct _io_param {
        _io_param(IO_TYPE type) {
            assert((type == STDIN || type == PIPE));
            _io_param::type = type;
        }

        _io_param(std::string &&filename) {
            type = FILE_PATH;
            _io_param::filename = std::move(filename);
        }

        IO_TYPE type;
        std::string filename;
    };

    struct _string_arg {
        _string_arg(const char *arg) : arg_value(arg) {}

        _string_arg(std::string &&arg) : arg_value(std::move(arg)) {}

        _string_arg(std::string arg) : arg_value(std::move(arg)) {}

        std::string arg_value;
    };

    struct _bool_arg {
        _bool_arg(bool b) : arg_value(b) {}

        bool arg_value;
    };

    template<typename T>
    struct _integer_arg {
        _integer_arg(T b) {
            static_assert(std::is_integral<T>::value, "Integer required.");
            arg_value = b;
        }

        T arg_value;
    };


    struct input : _io_param {
        input(std::string &&filename) : _io_param(std::move(filename)) {}

        input(IO_TYPE type) : _io_param(type) {}
    };

    struct output : _io_param {
        output(std::string &&filename) : _io_param(std::move(filename)) {}

        output(IO_TYPE type) : _io_param(type) {}
    };

    struct error : _io_param {
        error(std::string &&filename) : _io_param(std::move(filename)) {}

        error(IO_TYPE type) : _io_param(type) {}
    };

    struct defer_run : _bool_arg {
        defer_run(bool d) : _bool_arg(d) {}
    };

    struct close_fds : _bool_arg {
        close_fds(bool c) : _bool_arg(c) {}
    };

    struct session_leader : _bool_arg {
        session_leader(bool sl) : _bool_arg(sl) {}
    };

    struct shell : _bool_arg {
        shell(bool s) : _bool_arg(s) {}
    };

    struct executable : _string_arg {
        template<typename T>
        executable(T &&arg): _string_arg(std::forward<T>(arg)) {}
    };

    struct cwd : _string_arg {
        template<typename T>
        cwd(T &&arg): _string_arg(std::forward<T>(arg)) {}
    };

    struct environment {
        environment(std::map<std::string, std::string> &&env) :
                env(std::move(env)) {}

        environment(const std::map<std::string, std::string> &env) :
                env(env) {}

        std::map<std::string, std::string> env;
    };

    namespace utils {
        template<typename... T>
        struct param_pack {
        };

        template<typename F, typename T>
        struct has_type {
        };

        template<typename F>
        struct has_type<F, param_pack<>> {
            static constexpr bool value = false;
        };

        template<typename F, typename... T>
        struct has_type<F, param_pack<F, T...>> {
            static constexpr bool value = true;
        };

        template<typename F, typename H, typename... T>
        struct has_type<F, param_pack<H, T...>> {
            static constexpr bool value =
                    std::is_same<F, typename std::decay<H>::type>::value ? true : has_type<F, param_pack<T...>>::value;
        };

        static std::vector<std::string>
        split(const std::string &str, const std::string &delims = " \t") {
            std::vector<std::string> res;
            size_t init = 0;

            while (true) {
                auto pos = str.find_first_of(delims, init);
                if (pos == std::string::npos) {
                    res.emplace_back(str.substr(init, str.length()));
                    break;
                }
                res.emplace_back(str.substr(init, pos - init));
                pos++;
                init = pos;
            }

            return res;
        }

        static
        std::string join(const std::vector<std::string> &vec,
                         const std::string &sep = " ") {
            std::string res;
            for (auto &elem : vec) res.append(elem + sep);
            res.erase(--res.end());
            return res;
        }

        static
        void set_clo_on_exec(int fd, bool set = true) {
            int flags = fcntl(fd, F_GETFD, 0);
            if (set) flags |= FD_CLOEXEC;
            else flags &= ~FD_CLOEXEC;
            //TODO: should check for errors
            fcntl(fd, F_SETFD, flags);
        }

        static
        std::pair<int, int> pipe_cloexec() throw(OSError) {
            int pipe_fds[2];
            int res = pipe(pipe_fds);
            if (res) {
                throw OSError("pipe failure", errno);
            }

            set_clo_on_exec(pipe_fds[0]);
            set_clo_on_exec(pipe_fds[1]);

            return std::make_pair(pipe_fds[0], pipe_fds[1]);
        }

        static
        int write_n(int fd, const char *buf, size_t length) {
            int nwritten = 0;
            while (nwritten < length) {
                int written = write(fd, buf + nwritten, length - nwritten);
                if (written == -1) return -1;
                nwritten += written;
            }
            return nwritten;
        }

        static
        int read_atmost_n(int fd, char *buf, size_t read_upto) {
            int rbytes = 0;
            int eintr_cnter = 0;

            while (1) {
                int read_bytes = read(fd, buf, read_upto);
                if (read_bytes == -1) {
                    if (errno == EINTR) {
                        if (eintr_cnter >= 50) return -1;
                        eintr_cnter++;
                        continue;
                    }
                    return -1;
                }
                if (read_bytes == 0) return rbytes;

                rbytes += read_bytes;
            }
            return rbytes;
        }

        template<typename Buffer>
        // Requires Buffer to be of type class Buffer
        static int read_all(int fd, Buffer &buf) {
            size_t orig_size = buf.size();
            size_t increment = orig_size;
            auto buffer = buf.data();
            int total_bytes_read = 0;

            while (1) {
                int rd_bytes = read_atmost_n(fd, buffer, buf.size());
                if (rd_bytes == increment) {
                    // Resize the buffer to accomodate more
                    orig_size = orig_size * 1.5;
                    increment = orig_size - buf.size();
                    buf.resize(orig_size);
                    buffer += rd_bytes;
                    total_bytes_read += rd_bytes;
                } else if (rd_bytes != -1) {
                    total_bytes_read += rd_bytes;
                    break;
                } else {
                    if (total_bytes_read == 0) return -1;
                    break;
                }
            }
            return total_bytes_read;
        }

        static
        std::pair<int, int> wait_for_child_exit(int pid) {
            int status = 0;
            int ret = -1;
            while (1) {
                ret = waitpid(pid, &status, WNOHANG);
                if (ret == -1) break;
                if (ret == 0) continue;
                return std::make_pair(ret, status);
            }

            return std::make_pair(ret, status);
        }
    }

    class Buffer {
    public:
        Buffer() {}

        Buffer(size_t cap) { buf.resize(cap); }

        void add_cap(size_t cap) { buf.resize(cap); }

    public:
        std::vector<char> buf;
        size_t length = 0;
    };

    using OutBuffer = Buffer;
    using ErrBuffer = Buffer;

    namespace endpoint {
        class BaseEed {
        public:
            BaseEed() {}

            void operator=(const BaseEed &) = delete;

            void close_fds() {
                if (_input_fp != nullptr) {
                    fclose(_input_fp);
                } else if (_input_fd != -1) {
                    close(_input_fd);
                }
                _input_fd = -1;
                _input_fp = nullptr;

                if (_output_fp != nullptr) {
                    fclose(_output_fp);
                } else if (_output_fd != -1) {
                    close(_output_fd);
                }
                _output_fd = -1;
                _output_fp = nullptr;

                if (_error_fp != nullptr) {
                    fclose(_error_fp);
                } else if (_error_fd != -1) {
                    close(_error_fd);
                }
                _error_fd = -1;
                _error_fp = nullptr;
            }

            int input_fd() {
                return _input_fd;
            }

            FILE *input_fp() {
                return _input_fp;
            }

            int output_fd() {
                return _output_fd;
            }

            FILE *output_fp() {
                return _output_fp;
            }

            int error_fd() {
                return _error_fd;
            }

            FILE *error_fp() {
                return _error_fp;
            }

            void input(FILE *fp) {
                if (_input_fd != -1) {
                    close(_input_fd);
                }
                _input_fd = fileno(fp);
            }

            void output(FILE *fp) {
                if (_output_fd != -1) {
                    close(_output_fd);
                }
                _output_fd = fileno(fp);
            }

            void error(FILE *fp) {
                if (_error_fd != -1) {
                    close(_error_fd);
                }
                _error_fd = fileno(fp);
            }

            void input(int fd) {
                if (_input_fp != nullptr) {
                    fclose(_input_fp);
                } else if (_input_fd != -1) {
                    close(_input_fd);
                }
                _input_fp = fdopen(fd, "rb");
                _input_fd = fd;
            }

            void output(int fd) {
                if (_output_fp != nullptr) {
                    fclose(_output_fp);
                } else if (_output_fd != -1) {
                    close(_output_fd);
                }
                _output_fp = fdopen(fd, "wb");
                _output_fd = fd;
            }

            void error(int fd) {
                if (_error_fp != nullptr) {
                    fclose(_error_fp);
                } else if (_error_fd != -1) {
                    close(_error_fd);
                }
                _error_fp = fdopen(fd, "wb");
                _error_fd = fd;
            }

        private:
            int _input_fd = -1;
            int _output_fd = -1;
            int _error_fd = -1;

            FILE *_input_fp = nullptr;
            FILE *_output_fp = nullptr;
            FILE *_error_fp = nullptr;
        };

        class ParentEnd : public BaseEed {
        };

        class ChildEnd : public BaseEed {
        };
    }

    class sub_process {
    public:
        class ArgumentDeductionHelper {
        public:
            ArgumentDeductionHelper(sub_process *p) : sub(p) {}

            void set_option(cwd &&cwdir) {
                sub->cwd = std::move(cwdir.arg_value);
            }

            void set_option(environment &&env) {
                sub->env = std::move(env.env);
            }

            void set_option(defer_run &&defer) {
                sub->is_defer_process_start = std::move(defer.arg_value);
            }

            void set_option(shell &&sh) {
                sub->is_shell = std::move(sh.arg_value);
            }

            void set_option(input &&in) {
                int p_w = -1, c_r = -1, fd;
                switch (in.type) {
                    case FILE_PATH:
                        fd = open(in.filename.c_str(), O_RDONLY);
                        if (fd == -1)
                            throw OSError("File not found: ", errno);
                        utils::set_clo_on_exec(fd);
                        c_r = fd;
                        sub->bridge.second.input(c_r);
                        return;
                    case PIPE:
                        std::tie(p_w, c_r) = utils::pipe_cloexec();
                        sub->bridge.first.output(p_w);
                        sub->bridge.second.input(c_r);
                        return;
                    case STDIN:
                        return;
                    default:
                        throw new std::invalid_argument("input params not support " + in.type);
                }
            }

            void set_option(output &&out) {
                int p_r = -1, c_w = -1, fd;
                switch (out.type) {
                    case FILE_PATH:
                        fd = open(out.filename.c_str(), O_RDONLY);
                        if (fd == -1)
                            throw OSError("File not found: ", errno);
                        utils::set_clo_on_exec(fd);
                        c_w = fd;
                        sub->bridge.second.output(c_w);
                        return;
                    case PIPE:
                        std::tie(p_r, c_w) = utils::pipe_cloexec();
                        sub->bridge.first.input(p_r);
                        sub->bridge.second.output(c_w);
                        return;
                    case STDOUT:
                        return;
                    default:
                        throw new std::invalid_argument("output params not support " + out.type);
                }
            }

            void set_option(error &&err) {
                int p_r = -1, c_w = -1, fd;
                switch (err.type) {
                    case FILE_PATH:
                        fd = open(err.filename.c_str(), O_RDONLY);
                        if (fd == -1)
                            throw OSError("File not found: ", errno);
                        utils::set_clo_on_exec(fd);
                        c_w = fd;
                        sub->bridge.second.error(c_w);
                        return;
                    case PIPE:
                        std::tie(p_r, c_w) = utils::pipe_cloexec();
                        sub->bridge.first.error(p_r);
                        sub->bridge.second.error(c_w);
                        return;
                    case STDERR:
                        return;
                    default:
                        throw new std::invalid_argument("error params not support " + err.type);
                }
            }

            void set_option(close_fds &&cfds) {
                sub->is_close_fds = std::move(cfds.arg_value);
            }

            void set_option(session_leader &&sleader) {
                sub->is_session_leader = std::move(sleader.arg_value);
            }

        private:
            sub_process *sub = nullptr;
        };


        friend class Child;

        template<typename... Args>
        sub_process(const std::string &cmd_args, Args &&...args) {
            vargs = utils::split(cmd_args);
            init_args(std::forward<Args>(args)...);

            if (!is_defer_process_start) execute_process();
        }

        template<typename... Args>
        sub_process(std::initializer_list<const char *> cmd_args, Args &&...args) {
            vargs.insert(vargs.end(), cmd_args.begin(), cmd_args.end());
            init_args(std::forward<Args>(args)...);

            if (!is_defer_process_start) execute_process();
        }

        template<typename... Args>
        sub_process(const std::function<void(void)> &exe_func, Args &&...args) {
            sub_process::exe_func = exe_func;
            init_args(std::forward<Args>(args)...);

            if (!is_defer_process_start) execute_process();
        }

        void start_process() throw(ChildProcessError, OSError);

        int pid() const noexcept { return child_pid; }

        int return_code() const noexcept { return ret_code; }

        int wait() throw(OSError);

        int poll() throw(OSError);

        void kill(int sig_num = 9);

        size_t write(const char *msg, size_t length) {
            return std::fwrite(msg, sizeof(char), length, output());
        }

        size_t write(const std::vector<char> &msg) {
            return write(msg.data(), msg.size());
        }

        size_t read(char *msg, size_t length) {
            return std::fread(msg, sizeof(char), length, input());
        }

        size_t read(std::vector<char> &msg) {
            return read(msg.data(), msg.size());
        }

        size_t err_read(char *msg, size_t length) {
            return std::fread(msg, sizeof(char), length, error());
        }

        size_t err_read(std::vector<char> &msg) {
            return read(msg.data(), msg.size());
        }

        FILE *input() {
            return bridge.first.input_fp();
        }

        FILE *output() {
            return bridge.first.output_fp();
        }

        FILE *error() {
            return bridge.first.error_fp();
        }

    private:
        void init_args() {}

        template<typename F, typename... Args>
        void init_args(F &&f_arg, Args &&... args) {
            sub_process::ArgumentDeductionHelper adh(this);
            adh.set_option(std::forward<F>(f_arg));
            init_args(std::forward<Args>(args)...);
        }

        void execute_process() throw(ChildProcessError, OSError);

    private:
        std::pair<endpoint::ParentEnd, endpoint::ChildEnd> bridge;

        bool is_defer_process_start = false;
        bool is_close_fds = false;
        bool is_shell = false;
        bool is_session_leader = false;

        std::string cwd;
        std::map<std::string, std::string> env;
        std::string exe_name;
        std::function<void(void)> exe_func = nullptr;

        std::vector<std::string> vargs;

        bool is_child_created = false;
        int child_pid = -1;
        int ret_code = -1;
    };

    class Child {
    public:
        Child(bool close_fds, bool session_leader, const endpoint::ChildEnd &end, const std::string &cwd,
              const std::map<std::string, std::string> &env, const std::string &exe_name,
              const std::vector<std::string> &argv)
                : close_fds(close_fds),
                  session_leader(session_leader),
                  end(end),
                  cwd(cwd),
                  env(env),
                  exe_name(exe_name),
                  argv(argv) {}

        Child(bool close_fds, bool session_leader, const endpoint::ChildEnd &end, const std::string &cwd,
              const std::map<std::string, std::string> &env, const std::function<void()> &exe_func)
                : close_fds(close_fds),
                  session_leader(session_leader),
                  end(end),
                  cwd(cwd),
                  env(env),
                  exe_func(exe_func) {}


        void execute_child();

    private:
        bool close_fds;
        bool session_leader;
        endpoint::ChildEnd end;
        std::string cwd;
        std::map<std::string, std::string> env;
        std::string exe_name;
        std::function<void(void)> exe_func;
        std::vector<std::string> argv;
    };
}
#endif //PONGO_AGENT_SUB_PROCESS_H
