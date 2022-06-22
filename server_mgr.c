#include "server_mgr.h"


int run_server(int myport_input, const char* certificate_input, const char* priv_key_input){

    connection_params_t* params = malloc(sizeof(connection_params_t));
    MEMORY_ERROR( params);
    params->current_server_status = SHUT_DOWN;
    params->myport= myport_input;
    params->certificate = certificate_input;
    params->priv_key = priv_key_input;
    //clbtn_pressed_t btn;

    start_server(params);


    /* do{
        printf("\nPress A Button:\nBTN_CONNECT=0\nBTN_ARM=1\nBTN_IGNITE=2\nBTN_RESET=3\nSHUT_DOWN=4\n");
        scanf("%d",(int*) &btn);
        select_state(btn,params);

    }while(btn != BTN_SHUT_DOWN);
    printf("\nprogram is shutting down..\n"); */
}