#include <resolv.h>
#include "config.h"


int star_host_connection (connection_params_t* params);
int connect_with_server(const char * ip_address, int myport,connection_params_t* params);
void ShowCerts(SSL * ssl);
int write_to_remote(connection_params_t* params,wireless_data_t msg);
int read_from_remote(connection_params_t* params, wireless_data_t* msg_received);
int close_connection(connection_params_t* params);




