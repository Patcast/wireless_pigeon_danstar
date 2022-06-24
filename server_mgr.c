#include "server_mgr.h"


int run_server(int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = IDLE;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = FALSE;
   
    ///Stars openssl library and allow server to receive new connections 
    if (star_host_connection(params))return 1;
    if (start_port_server(params))return 1;

    do{
        /// Connects with in coming client
         if (connect_with_client(params))printf("Attempt to connect with client fail");
         else{
            ShowCerts(params->remote_ssl);
            ///TODO: SET GPIO to zero.
            select_state_server(params);
            close_remote_conn(params);
         }
    }while(params->host_state !=SHUT_DOWN);
    close_server(params);
}

int select_state_server(connection_params_t* params){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);

   /*  read_from_remote(params, msg_received); //TODO: Client should send it by default
    if ((msg_received->command == ZERO_CO ))run_zero(params,msg_received,NULL);
    else(printf("ERROR: setting zero state.\n"));
 */
    do{
        printf("Server is reading\n");
        read_from_remote(params, msg_received);
        if (msg_received->command == SHUTDOWN_CO)run_shut_down_server(params,msg_received);
        else if (params->host_state == IGNITE)run_zero(params,msg_received,"ERROR:CMD called after ignite. Setting zero state again");
        else if ((msg_received->command == ZERO_CO ))run_zero(params,msg_received,"Server is connected to Mission Control :)");
        else if ((msg_received->command == ARM_CO ))run_arm(params,msg_received);
        else if ((msg_received->command == IGNITE_CO ))run_ignite(params,msg_received);
        else run_zero(params,msg_received,"ERROR: The CMD received is not valid.");

    }while(params->host_state != SHUT_DOWN);
    free(msg_received);
}

int run_shut_down_server(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = SHUT_DOWN;
    msg_received->cmd_status=ACK;
    write_to_remote( params, *msg_received);
    return 0;
}

int run_zero(connection_params_t* params,wireless_data_t* msg_received, char* error){
    if(error!=NULL)printf("%s", error);
    params->host_state = CONNECTED;
    msg_received->cmd_status=ACK;
    write_to_remote( params, *msg_received);
    return 0;
}

int run_arm(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = ARM;
    msg_received->cmd_status=ACK;
    write_to_remote( params, *msg_received);
    return 0;
}
int run_ignite(connection_params_t* params,wireless_data_t* msg_received){
    if ((params->host_state == ARM )){
        params->host_state = IGNITE;
        msg_received->cmd_status=ACK;
        
        write_to_remote( params, *msg_received);
    }
    else {
        msg_received->instruction_code= 6666;
        run_zero(params,msg_received,"Please send Arm, before igniting");
    }
    return 0;
}

int close_server(connection_params_t* params){
    close_host_conn(params);
    free( params);
    printf("\nServer program is shutting down..\n"); 
}