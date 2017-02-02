//
// Created by zhsyourai on 1/11/17.
//

#ifndef PONGO_AGENT_PONGO_PROTOCOL_H
#define PONGO_AGENT_PONGO_PROTOCOL_H

#include <string>
#include <list>
#include "utils/singleton.h"
#include "utils/thread_pool.h"
#include "utils/crow.h"
#include "zRPC/include/rpc/caller.h"
#include "zRPC/include/client.h"

struct pongo_account {
    std::string id;
    std::string username;
    std::string password;
    std::string mail_password;
    std::string code;
    std::string answer;
};

struct card {
    std::string name;
    double sale;
};

struct api {
    std::string name;
    uint64_t request;
    uint64_t success;
    uint64_t fail;
};

struct pongo_statistics {
    std::string account_id;
    double current_coin;
    std::string status;
    std::list<card> sale_cards;
    std::list<card> buy_cards;
    std::list<api> api_status;
    uint64_t timestamp;
};

struct pongo_logs {
    std::string account_id;
    uint64_t timestamp;
    std::string str;
};


struct pongo_system_config {
    mem_status mem;
    cpu_status cpu;
    disk_status disk;
};

class pongo_protocol_account_callback {
public:
    virtual bool control_account_callback() = 0;

    virtual bool control_all_cccounts_callback() = 0;
};

class pongo_protocol : public singleton<pongo_protocol> {
    friend class pongo_connect;

public:
    pongo_protocol();

    void set_key(const std::string &key);

    void main_loop(zRPC_caller *caller, zRPC_client *client);

private:
    std::string auth(const std::string &key);

    void disconnect();

    std::list<std::string> get_pending_tasks();

    std::string get_config(const std::string &account_id);

    std::list<pongo_account> get_account_list();

    void report_logs(const std::string &account_id,const std::list<pongo_logs> logs);

    void report_statistics(const std::string &account_id, const pongo_statistics &statistics);

    void report_system_status(const pongo_system_config &system_config);

    void control_tasks();

    void push_tasks();

    void set_do_schduler(const std::string cron, const std::string &script);

private:
    thread_pool pool;
    std::string key;
    std::string session;
    zRPC_caller *caller;
    zRPC_client *client;
};


#endif //PONGO_AGENT_PONGO_PROTOCOL_H
