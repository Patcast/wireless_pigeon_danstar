//#include "server_pigeon.h" //TODO REMOVE!!
#include "conn_mgr.h"

int run_server(int myport_input, const char* certificate_input, const char* priv_key_input);
int select_state_server(connection_params_t* params);
int run_zero(connection_params_t* params,wireless_data_t* msg_received, char* error);
int run_arm(connection_params_t* params,wireless_data_t* msg_received);
int run_ignite(connection_params_t* params,wireless_data_t* msg_received);
int run_shut_down_server(connection_params_t* params,wireless_data_t* msg_received);
int close_server(connection_params_t* params);