//
// Created by zhsyourai on 1/11/17.
//

#include <rpc/caller.h>
#include "pongo_protocol.h"

pongo_protocol::pongo_protocol() : singleton() {}

void pongo_protocol::set_key(const std::string &key) {
    pongo_protocol::key = key;
}

void pongo_protocol::main_loop(zRPC_caller *caller) {
    auth(key);
    while (1) {

    }
}

std::string pongo_protocol::auth(const std::string &key) {
    return "";
}

void pongo_protocol::disconnect(const std::string &session) {

}

std::string pongo_protocol::get_config(const std::string &session) {
    return "";
}

std::list<pongo_account> pongo_protocol::get_account_list(const std::string &session) {
    return std::list<pongo_account>();
}

void pongo_protocol::report_logs(const std::string &session, const std::list<pongo_logs> logs) {

}

void pongo_protocol::report_statistics(const std::string &session, const pongo_statistics &statistics) {

}

void pongo_protocol::control_tasks(const std::string &session) {

}

void pongo_protocol::push_tasks(const std::string &session) {

}

void pongo_protocol::set_do_schduler(const std::string &session, const std::string cron, const std::string &script) {

}
