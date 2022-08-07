#include "server_mgr.h"
//TODO: When SD command is received. close down connection. 

//unsigned int PIN_ARM_1 = 86;

// BUG: PRevent cancel IGNITE signal due to debouncing. Redraw diagram. Also remove the function that If ignite is pressed twice it stops. 

int run_server(int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->host_state = NOT_CONNECTED;
    params->host_port= myport_input;
    params->host_certificate = certificate_input;
    params->host_key = priv_key_input;
    params->host_is_client = FALSE;

    #ifdef DEBUG_NO_GPIO 
        printf("No gpio mode use");
    #else 
        printf("GPIO is used");
    #endif
    
    ///Stars openssl library and allow server to receive new connections 
    if ((star_host_connection(params)==0)&&(start_port_server(params)==0)) {//TODO: is the error printed?
        start_gpio();
        do{
            
            if (connect_with_client(params))printf("Attempt to connect with client fail");
            else{
                params->host_state = ZERO;
                ShowCerts(params->remote_ssl);
                ///TODO: SET GPIO to zero.
                select_state_server(params);
                close_remote_conn(params);
            }
        }while(params->host_state !=SHUT_DOWN);
        close_server(params);
        close_gpio();
        return 0;
    }
    return 1;
    
}

int select_state_server(connection_params_t* params){
    wireless_data_t* msg_received = malloc(sizeof(connection_params_t));
    MEMORY_ERROR(msg_received);
    int bytes;

    do{
        //printf("Server is reading\n");
        //TODO: use switch statement.
        if(read_from_remote(params, msg_received))break;
        else{
            if (msg_received->command == SHUTDOWN_CO)run_shut_down_server(params,msg_received);
            else if (params->host_state == IGNITE)run_zero(params,msg_received,"ERROR:CMD called after ignite. Setting zero state again\n");
            else if ((msg_received->command == ZERO_CO ))run_zero(params,msg_received,"Server is connected to Mission Control :)\n");
            else if ((msg_received->command == ARM_CO ))run_arm(params,msg_received);
            else if ((msg_received->command == IGNITE_CO ))run_ignite(params,msg_received);
            else run_zero(params,msg_received,"ERROR: The CMD received is not valid.");
        }
        printf("Server has changed to state = %d\n", params->host_state);
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
    params->host_state = ZERO;
    msg_received->cmd_status=ACK;
    reset_gpio();
    write_to_remote( params, *msg_received);
    return 0;
}

int run_arm(connection_params_t* params,wireless_data_t* msg_received){
    params->host_state = ARM;
    msg_received->cmd_status=ACK;
    set_gpio_arm(msg_received->instruction_code);
    write_to_remote( params, *msg_received);
    return 0;
}
int run_ignite(connection_params_t* params,wireless_data_t* msg_received){
    if ((params->host_state == ARM )){
        params->host_state = IGNITE;
        msg_received->cmd_status=ACK;
        set_gpio_ign(msg_received->instruction_code);
        write_to_remote( params, *msg_received);
    }
    else {
        msg_received->instruction_code= 0xFF;
        run_zero(params,msg_received,"Please send Arm, before igniting");
    }
    return 0;
}

int set_gpio_arm(u_int8_t arm_signal){
    
    if(arm_signal!=0){
        unsigned int bit;

        bit = (arm_signal & FLAG_SIGNAL_6) ? 1 : 0;
        gpio_set_value(PIN_ARM_1, bit);
        bit = (arm_signal & FLAG_SIGNAL_5) ? 1 : 0;
        gpio_set_value(PIN_ARM_2, bit);
        bit = (arm_signal & FLAG_SIGNAL_4) ? 1 : 0;
        gpio_set_value(PIN_ARM_3, bit);
        bit = (arm_signal & FLAG_SIGNAL_3) ? 1 : 0;
        gpio_set_value(PIN_ARM_4, bit);
        bit = (arm_signal & FLAG_SIGNAL_2) ? 1 : 0;
        gpio_set_value(PIN_ARM_5, bit);
        bit = (arm_signal & FLAG_SIGNAL_1) ? 1 : 0;
        gpio_set_value(PIN_ARM_6, bit);
    }
    return 0;
}
int set_gpio_ign(u_int8_t ign_signal){
    if(ign_signal!=0){
        printf("IGN signal:\n");
        unsigned char bit;
        bit = (ign_signal & FLAG_SIGNAL_6) ? 1 : 0;
        gpio_set_value(PIN_IGN_6, bit);
        bit = (ign_signal & FLAG_SIGNAL_5) ? 1 : 0;
        gpio_set_value(PIN_IGN_5, bit);
        bit = (ign_signal & FLAG_SIGNAL_4) ? 1 : 0;
        gpio_set_value(PIN_IGN_4, bit);
        bit = (ign_signal & FLAG_SIGNAL_3) ? 1 : 0;
        gpio_set_value(PIN_IGN_3, bit);
        bit = (ign_signal & FLAG_SIGNAL_2) ? 1 : 0;
        gpio_set_value(PIN_IGN_2, bit);
        bit = (ign_signal & FLAG_SIGNAL_1) ? 1 : 0;
        gpio_set_value(PIN_IGN_1, bit);
        return 0;
    }
    return 1;
}
int start_gpio(){

    // BUG : If gpio files already exist, this will crash. 
    printf("Starting GPIO...\n");
    gpio_export(PIN_ARM_1);  
    gpio_set_dir(PIN_ARM_1, OUTPUT_PIN); 
    gpio_export(PIN_ARM_2);  
    gpio_set_dir(PIN_ARM_2, OUTPUT_PIN);
    gpio_export(PIN_ARM_3);  
    gpio_set_dir(PIN_ARM_3, OUTPUT_PIN);
    gpio_export(PIN_ARM_4);  
    gpio_set_dir(PIN_ARM_4, OUTPUT_PIN);
    gpio_export(PIN_ARM_5);  
    gpio_set_dir(PIN_ARM_5, OUTPUT_PIN);
    gpio_export(PIN_ARM_6);  
    gpio_set_dir(PIN_ARM_6, OUTPUT_PIN);

    gpio_export(PIN_IGN_1);  
    gpio_set_dir(PIN_IGN_1, OUTPUT_PIN); 
    gpio_export(PIN_IGN_2);  
    gpio_set_dir(PIN_IGN_2, OUTPUT_PIN);
    gpio_export(PIN_IGN_3);  
    gpio_set_dir(PIN_IGN_3, OUTPUT_PIN);
    gpio_export(PIN_IGN_4);  
    gpio_set_dir(PIN_IGN_4, OUTPUT_PIN);
    gpio_export(PIN_IGN_5);  
    gpio_set_dir(PIN_IGN_5, OUTPUT_PIN);
    gpio_export(PIN_IGN_6);  
    gpio_set_dir(PIN_IGN_6, OUTPUT_PIN);
    return 0;
}

int reset_gpio(){
    gpio_set_value(PIN_ARM_1, LOW);
    gpio_set_value(PIN_ARM_2, LOW);
    gpio_set_value(PIN_ARM_3, LOW);
    gpio_set_value(PIN_ARM_4, LOW);
    gpio_set_value(PIN_ARM_5, LOW);
    gpio_set_value(PIN_ARM_6, LOW);

    gpio_set_value(PIN_IGN_1, LOW);
    gpio_set_value(PIN_IGN_2, LOW);
    gpio_set_value(PIN_IGN_3, LOW);
    gpio_set_value(PIN_IGN_4, LOW);
    gpio_set_value(PIN_IGN_5, LOW);
    gpio_set_value(PIN_IGN_6, LOW);
}

int close_gpio(){
    gpio_unexport(PIN_ARM_1);
    gpio_unexport(PIN_ARM_2);
    gpio_unexport(PIN_ARM_3);
    gpio_unexport(PIN_ARM_4);
    gpio_unexport(PIN_ARM_5);
    gpio_unexport(PIN_ARM_6);

    gpio_unexport(PIN_IGN_1);
    gpio_unexport(PIN_IGN_2);
    gpio_unexport(PIN_IGN_3);
    gpio_unexport(PIN_IGN_4);
    gpio_unexport(PIN_IGN_5);
    gpio_unexport(PIN_IGN_6);
}

int close_server(connection_params_t* params){
    close_host_conn(params);
    free( params);
    printf("\nServer program is shutting down..\n"); 
}