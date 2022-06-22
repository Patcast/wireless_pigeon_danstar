#include "server_mgr.h"


int run_server(int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = IDLE;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = FALSE;
   
    if (star_host_connection(params))return 1;
    if (start_port_server(params))return 1;

    do{
         if (connect_with_client(params))printf("Attempt to connect with client fail");
         else{
            ShowCerts(params->remote_ssl);
            select_state_server(params);
            close_remote_conn(params);
         }
    }while(params->host_state !=SHUT_DOWN);
    close_server(params);
}

int select_state_server(connection_params_t* params){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);
    do{
        printf("Server is connected and listennig\n");
        read_from_remote(params, msg_received);
        if ((msg_received->command == CONNECT_CO )&& (params->host_state == NOT_CONNECTED))run_connect_server(params,msg_received);
        if (msg_received->command == SHUTDOWN_CO)run_shut_down_server(params,msg_received);

    }while(params->host_state != SHUT_DOWN);
    free(msg_received);
}

int run_connect_server(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = CONNECTED;
    msg_received->cmd_status=ACK;
    write_to_remote( params, *msg_received);
}
int run_shut_down_server(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = SHUT_DOWN;
    msg_received->cmd_status=ACK;
    write_to_remote( params, *msg_received);
}
int close_server(connection_params_t* params){
    close_host_conn(params);
    free( params);
    printf("\nServer program is shutting down..\n"); 
}