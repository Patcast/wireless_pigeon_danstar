#include "config.h"


int star_host_connection (connection_params_t* params);
int connect_with_server(connection_params_t* params);
int start_port_server (connection_params_t* params);
int connect_with_client(connection_params_t* params);
int ShowCerts(SSL * ssl);
int write_to_remote(connection_params_t* params,wireless_data_t msg);
int read_from_remote(connection_params_t* params, wireless_data_t* msg_received);
int close_remote_conn(connection_params_t* params);
int close_host_conn(connection_params_t* params);




