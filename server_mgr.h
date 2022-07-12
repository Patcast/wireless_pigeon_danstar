//#include "server_pigeon.h" //TODO REMOVE!!
#include "conn_mgr.h"
#include "gpio_handler.h"

#define FLAG_SIGNAL_1 0x01
#define FLAG_SIGNAL_2 0x02
#define FLAG_SIGNAL_3 0x04
#define FLAG_SIGNAL_4 0x08
#define FLAG_SIGNAL_5 0x10
#define FLAG_SIGNAL_6 0x20

#define PIN_ARM_1 86
#define PIN_ARM_2 87
#define PIN_ARM_3 10
#define PIN_ARM_4 9
#define PIN_ARM_5 8
#define PIN_ARM_6 78

#define PIN_IGN_1 88
#define PIN_IGN_2 89
#define PIN_IGN_3 11
#define PIN_IGN_4 81
#define PIN_IGN_5 80
#define PIN_IGN_6 79





int run_server(int myport_input, const char* certificate_input, const char* priv_key_input);
int select_state_server(connection_params_t* params);
int run_zero(connection_params_t* params,wireless_data_t* msg_received, char* error);
int run_arm(connection_params_t* params,wireless_data_t* msg_received);
int run_ignite(connection_params_t* params,wireless_data_t* msg_received);
int run_shut_down_server(connection_params_t* params,wireless_data_t* msg_received);
/* Extracts the each bit of the arm_signal and sets the gpio of the embedded device accordingly.*/
int set_gpio_arm(u_int8_t arm_signal);
/* Extracts the each bit of the ign_signal and sets the gpio of the embedded device accordingly.*/
int set_gpio_ign(u_int8_t ign_signal);
/* Set the pins on the server to be outputs*/
int start_gpio();
int reset_gpio();
int close_gpio();
int close_server(connection_params_t* params);