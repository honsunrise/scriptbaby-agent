//
// Created by zhsyourai on 1/11/17.
//

#include <rpc/caller.h>
#include "pongo_protocol.h"

pongo_protocol::pongo_protocol() : singleton() {}

void pongo_protocol::set_key(const std::string &key) {
    pongo_protocol::key = key;
}

void pongo_protocol::main_loop(zRPC_caller *caller, zRPC_client *client) {
    this->caller = caller;
    this->client = client;
    session = auth(key);
    while (1) {
        crow system_config;
        pongo_system_config system_status;
        system_config.collection(system_status.mem , system_status.cpu, system_status.disk);
        report_system_status(system_status);
        std::list<std::string> pending_work = get_pending_tasks();
        std::for_each(pending_work.begin(), pending_work.end(), [](std::string &work){

        });
        sleep(10);
    }
}

std::string call_auth(zRPC_caller *caller, zRPC_client *client, std::string key) {
    zRPC_call_param *params = (zRPC_call_param *) malloc(sizeof(zRPC_call_param) * 1);
    params[0].name = "key";
    params[0].value.type = STR;
    params[0].value.str_value.len = key.length();
    char *str = (char *) malloc(key.length() + 1);
    strcpy(str, key.c_str());
    params[0].value.str_value.str = str;
    zRPC_caller_do_call(caller, client, "auth", params, 1);
    zRPC_call_result *result;
    zRPC_caller_wait_result(caller, &result);
    zRPC_value value;
    zRPC_call_result_get_param(result, "function_ret", &value);
    zRPC_caller_destroy_result(caller, result);
    free(params);
    return value.str_value.str;
}

std::string pongo_protocol::auth(const std::string &key) {
    return call_auth(caller, client, key);
}

void pongo_protocol::disconnect() {

}

std::string pongo_protocol::get_config(const std::string &account_id) {
    return "";
}

std::list<pongo_account> pongo_protocol::get_account_list() {
    return std::list<pongo_account>();
}

void pongo_protocol::report_logs(  const std::string &account_id,const std::list<pongo_logs> logs) {

}

void pongo_protocol::report_statistics(  const std::string &account_id, const pongo_statistics &statistics) {

}

void pongo_protocol::control_tasks() {

}

void pongo_protocol::push_tasks() {

}

void pongo_protocol::set_do_schduler(const std::string cron, const std::string &script) {

}


std::list<std::string> call_get_pending_tasks(zRPC_caller *caller, zRPC_client *client, const std::string &session) {
    zRPC_call_param *params = (zRPC_call_param *) malloc(sizeof(zRPC_call_param) * 1);
    params[0].name = "session";
    params[0].value.type = INT64;
    params[0].value.str_value.len = session.length();
    char *str = (char *) malloc(session.length() + 1);
    bzero(str, session.length() + 1);
    strcpy(str, session.c_str());
    params[0].value.str_value.str = str;
    zRPC_caller_do_call(caller, client, "get_pending_tasks", params, 1);
    zRPC_call_result *result;
    zRPC_caller_wait_result(caller, &result);
    zRPC_value value;
    zRPC_call_result_get_param(result, "function_ret", &value);
    zRPC_caller_destroy_result(caller, result);
    free(params);
    std::list<std::string> ret_list;
    ret_list.push_back(std::string(value.str_value.str));
    return ret_list;
}

std::list<std::string> pongo_protocol::get_pending_tasks() {
    return call_get_pending_tasks(caller, client, session);
}

std::string call_report_system_status(zRPC_caller *caller, zRPC_client *client, const std::string &session) {
    zRPC_call_param *params = (zRPC_call_param *) malloc(sizeof(zRPC_call_param) * 1);
    params[0].name = "session";
    params[0].value.type = INT64;
    params[0].value.str_value.len = session.length();
    char *str = (char *) malloc(session.length() + 1);
    bzero(str, session.length() + 1);
    strcpy(str, session.c_str());
    params[0].value.str_value.str = str;
    zRPC_caller_do_call(caller, client, "report_system_status", params, 1);
    zRPC_call_result *result;
    zRPC_caller_wait_result(caller, &result);
    zRPC_value value;
    zRPC_call_result_get_param(result, "function_ret", &value);
    zRPC_caller_destroy_result(caller, result);
    free(params);
    return value.str_value.str;
}

void pongo_protocol::report_system_status(const pongo_system_config &system_config) {
    call_report_system_status(caller, client, session);
}
