#include "client_mgr.h"



int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = IDLE;
    params->remote_ip_addr =ip_address_input;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    btn_pressed_t btn;

    do{
        printf("\nPress A Button:\nBTN_CONNECT=0\nBTN_ARM=1\nBTN_IGNITE=2\nBTN_RESET=3\nSHUT_DOWN=4\n");
        scanf("%d",(int*) &btn);
        select_state(btn,params);

    }while(btn != BTN_SHUT_DOWN);
    printf("\nprogram is shutting down..\n");
}

int select_state(btn_pressed_t input,connection_params_t* params){
    if (input == BTN_CONNECT)run_connect(params);
    else if(input == BTN_SHUT_DOWN){
        if(params->remote_ssl != NULL)close_connection(params);
        free( params);
    }
}

int start_client (connection_params_t* params){
    
    if (star_host_connection(params))return 1;
    if (connect_with_server(params->remote_ip_addr,params->host_port,params))return 1;
    ShowCerts(params->remote_ssl);
    return 0;
}

int run_connect(connection_params_t* params){
    if(start_client (params)) printf("Error in the connection\n");
    else{
        params->host_state=NOT_CONNECTED;
        wireless_data_t msg_send;
        wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
        msg_send.command = CONNECT_CO;
        msg_send.cmd_status = REQUESTED;
        msg_send.instruction_code = 0;
        if (write_to_remote(params,msg_send))printf("Sending failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));
        if (read_from_remote(params,msg_received)) printf("Message reception failed!Error code= %d. Error message is:'%s'\n", errno, strerror(errno));
        if (msg_received->command==CONNECT_CO&&msg_received->cmd_status==ACK){
            params->host_state=CONNECTED;  
            printf("Client has changed to state = %d\n", params->host_state);
        }else("ERROR: during not connected status\n");
        free(msg_received);
    } 
}