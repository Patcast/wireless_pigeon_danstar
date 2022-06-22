#include "client_mgr.h"



int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = IDLE;
    params->remote_ip_addr =ip_address_input;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = TRUE;
    btn_pressed_t btn;

    do{
        printf("\nPress a Button:\nBTN_CONNECT=0\nBTN_ARM=1\nBTN_IGNITE=2\nBTN_RESET=3\nBTN_SHUT_DOWN_SERVER=4\nBTN_SHUT_DOWN_CLIENT=5\n");
        scanf("%d",(int*) &btn);
        select_state(btn,params);
    }while(params->host_state != SHUT_DOWN);

    exit_client(params);  
}

int select_state(btn_pressed_t input,connection_params_t* params){

    if (input == BTN_CONNECT) connect_with_rocket(params,CONNECT_CO,REQUESTED,0);
    else if(input == BTN_SHUT_DOWN_SERVER) shut_down_server(params,SHUTDOWN_CO,REQUESTED,0);
}
int execute_command_handshake(connection_params_t* params, wireless_data_t msg_send){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);

    if (write_to_remote(params,msg_send)){
        printf("Sending failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));
        return 1;
    }
    if (read_from_remote(params,msg_received)){
        printf("Message reception failed!Error code= %d. Error message is:'%s'\n", errno, strerror(errno));
        return 1;
    }
    
    if ((msg_received->command!=msg_send.command)&&(msg_received->cmd_status!=ACK)){
        printf("Error: wrong ASK packet received: commad: %d\ncmd_status: %d\n",(int)msg_received->command,(int)msg_received->cmd_status);
        return 1;
    }
    free(msg_received);
    return 0;
}


int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int32_t  instruction_code){
    if(start_client (params)) printf("Error in the connection\n");
    else{
         wireless_data_t msg_send;
        msg_send.command = command;
        msg_send.cmd_status = cmd_status;
        msg_send.instruction_code = instruction_code;
        params->host_state=NOT_CONNECTED;
        if(!execute_command_handshake(params,msg_send)){
            params->host_state=CONNECTED;  
            printf("Client has changed to state = %d\n", params->host_state);
        }
    } 
}


int start_client (connection_params_t* params){
    
    if (star_host_connection(params)){
        printf("error starting host\n");
        return 1;
    }
    if (connect_with_server(params)){
        printf("error connecting with server\n");
        return 1;
    }
    ShowCerts(params->remote_ssl);
    return 0;
}

int shut_down_server(connection_params_t* params,commands_t command,status_t cmd_status,u_int32_t  instruction_code){
    wireless_data_t msg_send;
    msg_send.command = command;
    msg_send.cmd_status = cmd_status;
    msg_send.instruction_code = instruction_code;
    if(!execute_command_handshake(params,msg_send)){
            params->host_state=SHUT_DOWN;  
            printf("Client has changed to state = %d\n", params->host_state);
    }
    return 0;
}

int exit_client(connection_params_t* params){
    if(params->host_state != IDLE)close_host_conn(params);
    free( params);
    printf("\nprogram is shutting down..\n");
    return 0;
}


