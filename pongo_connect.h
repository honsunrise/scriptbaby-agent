//
// Created by zhsyourai on 1/11/17.
//

#ifndef PONGO_AGENT_PONGO_CONNECT_H
#define PONGO_AGENT_PONGO_CONNECT_H


#include <string>
#include <client.h>
#include <filter/bytes_filter.h>
#include <rpc/caller.h>
#include "utils/thread_pool.h"
#include "pongo_protocol.h"

class pongo_connect {
public:
    pongo_connect(pongo_protocol* protocol);

    virtual ~pongo_connect();

    void connect_to_manager();

    void main_loop();

    static void connect_callback(pongo_connect *pthis, zRPC_caller_instance *caller_instance, zRPC_call *call,
                                 zRPC_call_result *result);
    static void control_tasks(pongo_connect *pthis, zRPC_caller_instance *caller_instance, zRPC_call *call,
                              zRPC_call_result *result);
    static void push_tasks(pongo_connect *pthis, zRPC_caller_instance *caller_instance, zRPC_call *call,
                           zRPC_call_result *result);

private:
    zRPC_context *context;
    zRPC_client *client;
    zRPC_caller *caller;
    thread_pool pool;
    pongo_protocol *protocol;
};


#endif //PONGO_AGENT_PONGO_CONNECT_H
