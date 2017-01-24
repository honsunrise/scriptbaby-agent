//
// Created by zhsyourai on 1/12/17.
//

#include "sub_process.h"

using namespace subprocess;

void sub_process::start_process() throw(ChildProcessError, OSError) {
    if (!is_defer_process_start) {
        assert (0);
        return;
    }
    execute_process();
}

int sub_process::wait() throw(OSError) {
    int ret, status;
    std::tie(ret, status) = utils::wait_for_child_exit(pid());
    if (ret == -1) {
        if (errno != ECHILD)
            throw OSError("waitpid failed", errno);
        return 0;
    }
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return WTERMSIG(status);
    else
        return 255;
}

int sub_process::poll() throw(OSError) {
    int status;
    if (!is_child_created)
        return -1;

    int ret = waitpid(child_pid, &status, WNOHANG);
    if (ret == 0)
        return -1;

    if (ret == child_pid) {
        if (WIFSIGNALED(status)) {
            ret_code = WTERMSIG(status);
        } else if (WIFEXITED(status)) {
            ret_code = WEXITSTATUS(status);
        } else {
            ret_code = 255;
        }
        return ret_code;
    }

    if (ret == -1) {
        if (errno == ECHILD) ret_code = 0;
        else throw OSError("waitpid failed", errno);
    } else {
        ret_code = ret;
    }

    return ret_code;
}

void sub_process::kill(int sig_num) {
    if (is_session_leader)
        killpg(child_pid, sig_num);
    else ::kill(child_pid, sig_num);
}


void sub_process::execute_process() throw(ChildProcessError, OSError) {
    if (vargs.empty()) {
        child_pid = fork();

        if (child_pid < 0) {
            throw OSError("fork failed", errno);
        }

        if (child_pid == 0) {
            bridge.first.close_fds();
            Child child(is_close_fds, is_session_leader, bridge.second, cwd, env, exe_func);
            child.execute_child();
        } else {
            bridge.second.close_fds();
            is_child_created = true;
            int sys_ret = -1;
        }
    } else {
        if (is_shell) {
            auto new_cmd = utils::join(vargs);
            vargs.clear();
            vargs.insert(vargs.begin(), {"/bin/sh", "-c"});
            vargs.push_back(new_cmd);
        }
        exe_name = vargs[0];
        child_pid = fork();
        if (child_pid < 0) {
            throw OSError("fork failed", errno);
        }
        if (child_pid == 0) {
            bridge.first.close_fds();
            Child child(is_close_fds, is_session_leader, bridge.second, cwd, env, exe_name, vargs);
            child.execute_child();
        } else {
            bridge.second.close_fds();
            is_child_created = true;
            int sys_ret = -1;
        }
    }
}

void Child::execute_child() {
    int sys_ret = -1;

    try {
        auto _dup2_ = [](int fd, int to_fd) {
            if (fd != -1) {
                int res = dup2(fd, to_fd);
                if (res == -1)
                    throw OSError("dup2 failed", errno);
            }
        };

        _dup2_(end.input_fd(), STDIN_FILENO);
        _dup2_(end.output_fd(), STDOUT_FILENO);
        _dup2_(end.error_fd(), STDERR_FILENO);
        end.close_fds();
        if (close_fds) {
            long max_fd = sysconf(_SC_OPEN_MAX);
            if (max_fd == -1)
                throw OSError("sysconf failed", errno);

            for (int i = 3; i < max_fd; i++) {
                close(i);
            }
        }

        if (cwd.length()) {
            sys_ret = chdir(cwd.c_str());
            if (sys_ret == -1)
                throw OSError("chdir failed", errno);
        }

        if (session_leader) {
            sys_ret = setsid();
            if (sys_ret == -1)
                throw OSError("setsid failed", errno);
        }

        if (env.size()) {
            for (auto &kv : env) {
                setenv(kv.first.c_str(), kv.second.c_str(), 1);
            }
        }
        if (exe_name.empty()) {
            exe_func();
        } else {
            std::vector<char *> c_str;
            std::transform(argv.begin(), argv.end(), std::back_inserter(c_str), [](std::string const &s) -> char * {
                char *pc = new char[s.size() + 1];
                std::strcpy(pc, s.c_str());
                return pc;
            });
            c_str.push_back(nullptr);
            sys_ret = execvp(exe_name.c_str(), c_str.data());
            std::for_each(c_str.begin(), c_str.end(), [](char *p) {
                delete[] p;
            });
            std::cout << argv.data() << std::endl;
            if (sys_ret == -1)
                throw OSError("execve failed", errno);
        }
    } catch (const OSError &exp) {
        std::string err_msg(exp.what());
        throw exp;
    }
    exit(EXIT_FAILURE);
}
