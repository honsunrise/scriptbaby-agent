//
// Created by zhsyourai on 1/11/17.
//

#include "pongo_protocol.h"
#include "zRPC/include/support/var_type.h"

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
            std::cout << work << std::endl;
        });
        sleep(10);
    }
}

std::string call_auth(zRPC_caller *caller, zRPC_client *client, std::string key, std::string agent_uid) {
    zRPC_call_param *params = (zRPC_call_param *) malloc(sizeof(zRPC_call_param) * 2);
    params[0].name = "key";
    params[0].value = zRPC_type_var_create_base(zRPC_type_base_create(STR, key.c_str()));

    params[1].name = "agents_uid";
    params[1].value = zRPC_type_var_create_base(zRPC_type_base_create(STR, agent_uid.c_str()));
    zRPC_caller_do_call(caller, client, "auth", params, 2);
    zRPC_call_result *result;
    zRPC_caller_wait_result(caller, &result);
    zRPC_value *value;
    zRPC_call_result_get_param(result, "function_ret", &value);
    zRPC_caller_destroy_result(caller, result);
    free(params);
    return value->base_value->value.str_value.str;
}

std::string pongo_protocol::auth(const std::string &key) {
    crow system_collection;
    return call_auth(caller, client, key, "");
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
    params[0].value = zRPC_type_var_create_base(zRPC_type_base_create(STR, session.c_str()));
    zRPC_caller_do_call(caller, client, "get_pending_tasks", params, 1);
    zRPC_call_result *result;
    zRPC_caller_wait_result(caller, &result);
    zRPC_value *value;
    zRPC_call_result_get_param(result, "function_ret", &value);
    std::list<std::string> ret_list;
    zRPC_call_result_get_param(result, "pending_tasks", &value);
    for(int i = 0; i < value->array_value->len; ++i) {
        ret_list.push_back(std::string(value->array_value->value[i].str_value.str));
    }
    zRPC_caller_destroy_result(caller, result);
    free(params);
    return ret_list;
}

std::list<std::string> pongo_protocol::get_pending_tasks() {
    return call_get_pending_tasks(caller, client, session);
}

void call_report_system_status(zRPC_caller *caller, zRPC_client *client, const std::string &session,
                               const pongo_system_config &system_config) {
    zRPC_call_param *params = (zRPC_call_param *) malloc(sizeof(zRPC_call_param) * 4);
    params[0].name = "session";
    params[0].value = zRPC_type_var_create_base(zRPC_type_base_create(STR, session.c_str()));

    params[1].name = "mem";
    params[1].value = zRPC_type_var_create_map(4);

    params[1].value->map_value->value[0].key = zRPC_type_base_create(STR, "mem_percent");
    params[1].value->map_value->value[0].value = zRPC_type_base_create(FLOAT, &system_config.mem.mem_percent);

    params[1].value->map_value->value[1].key = zRPC_type_base_create(STR, "phys_mem_total");
    params[1].value->map_value->value[1].value = zRPC_type_base_create(FLOAT, &system_config.mem.phys_mem_total);

    params[1].value->map_value->value[2].key = zRPC_type_base_create(STR, "phys_mem_used");
    params[1].value->map_value->value[2].value = zRPC_type_base_create(FLOAT, &system_config.mem.phys_mem_used);

    params[1].value->map_value->value[3].key = zRPC_type_base_create(STR, "phys_mem_free");
    params[1].value->map_value->value[3].value = zRPC_type_base_create(FLOAT, &system_config.mem.phys_mem_free);


    params[2].name = "cpu";
    params[2].value = zRPC_type_var_create_map(4);

    params[2].value->map_value->value[0].key = zRPC_type_base_create(STR, "cpu_percent");
    params[2].value->map_value->value[0].value = zRPC_type_base_create(FLOAT, &system_config.cpu.cpu_percent);

    params[2].value->map_value->value[1].key = zRPC_type_base_create(STR, "cpu_total_user");
    params[2].value->map_value->value[1].value = zRPC_type_base_create(FLOAT, &system_config.cpu.cpu_total_user);

    params[2].value->map_value->value[2].key = zRPC_type_base_create(STR, "cpu_total_sys");
    params[2].value->map_value->value[2].value = zRPC_type_base_create(FLOAT, &system_config.cpu.cpu_total_sys);

    params[2].value->map_value->value[3].key = zRPC_type_base_create(STR, "cpu_total_idle");
    params[2].value->map_value->value[3].value = zRPC_type_base_create(FLOAT, &system_config.cpu.cpu_total_idle);


    params[3].name = "disk";
    params[3].value = zRPC_type_var_create_map(4);

    params[3].value->map_value->value[0].key = zRPC_type_base_create(STR, "disk_percent");
    params[3].value->map_value->value[0].value = zRPC_type_base_create(FLOAT, &system_config.disk.disk_percent);

    params[3].value->map_value->value[1].key = zRPC_type_base_create(STR, "disk_total");
    params[3].value->map_value->value[1].value = zRPC_type_base_create(FLOAT, &system_config.disk.disk_total);

    params[3].value->map_value->value[2].key = zRPC_type_base_create(STR, "disk_used");
    params[3].value->map_value->value[2].value = zRPC_type_base_create(FLOAT, &system_config.disk.disk_used);

    params[3].value->map_value->value[3].key = zRPC_type_base_create(STR, "disk_free");
    params[3].value->map_value->value[3].value = zRPC_type_base_create(FLOAT, &system_config.disk.disk_free);


    zRPC_caller_do_call(caller, client, "report_system_status", params, 4);
    zRPC_call_result *result;
    zRPC_caller_wait_result(caller, &result);
    zRPC_value *value;
    zRPC_call_result_get_param(result, "function_ret", &value);
    zRPC_caller_destroy_result(caller, result);
    free(params);
}

void pongo_protocol::report_system_status(const pongo_system_config &system_config) {
    call_report_system_status(caller, client, session, system_config);
}
