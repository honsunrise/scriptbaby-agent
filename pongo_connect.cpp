//
// Created by zhsyourai on 1/11/17.
//
#include "pongo_connect.h"

pongo_connect::pongo_connect(pongo_protocol* protocol): protocol(protocol) {}

pongo_connect::~pongo_connect() {

}

void pongo_connect::connect_to_manager() {
    /*Init context*/
    context = zRPC_context_create();

    /*Init caller*/
    zRPC_caller_create(&caller);

    zRPC_function_table_item function_table_callee[] = {
            {"connect_callback", this,     (void (*)(void *, zRPC_caller_instance *, zRPC_call *,
                                                     zRPC_call_result *)) connect_callback},
            {"control_tasks", this,      (void (*)(void *, zRPC_caller_instance *, zRPC_call *,
                                                     zRPC_call_result *)) control_tasks},
            {"control_all_tasks", this, (void (*)(void *, zRPC_caller_instance *, zRPC_call *,
                                                     zRPC_call_result *)) push_tasks}
    };

    zRPC_caller_set_function_callback_table(caller, function_table_callee,
                                            sizeof(function_table_callee) / sizeof(*function_table_callee));

    /*Init pipe*/
    zRPC_pipe *client_pipe;
    zRPC_pipe_create(&client_pipe);
    zRPC_filter **filters_client;
    int filter_count_client;
    zRPC_caller_get_filters(caller, &filters_client, &filter_count_client);
    for (int i = 0; i < filter_count_client; ++i) {
        zRPC_pipe_add_filter(client_pipe, filters_client[i]);
    }
    client = zRPC_client_create(context, "manager.fifatrade.online:5966", client_pipe);
}

void pongo_connect::main_loop() {
    /* Client start */
    zRPC_client_start(client);
    /* enter main loop*/
    zRPC_context_dispatch(context);
}

void pongo_connect::connect_callback(pongo_connect *pthis, zRPC_caller_instance *caller_instance, zRPC_call *call, zRPC_call_result *result) {
    pthis->pool.run_task(std::bind(&pongo_protocol::main_loop, pthis->protocol, pthis->caller));
}

void pongo_connect::control_tasks(pongo_connect *pthis, zRPC_caller_instance *caller_instance, zRPC_call *call,
                                  zRPC_call_result *result) {
    pthis->protocol->control_tasks("");
}

void
pongo_connect::push_tasks(pongo_connect *pthis, zRPC_caller_instance *caller_instance, zRPC_call *call,
                          zRPC_call_result *result) {
    pthis->protocol->push_tasks("");
}
