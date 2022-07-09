#include "conn_mgr.h"
#define ARM_SIGNAL 0xFF
#define IGNITE_SIGNAL 0xFF

typedef enum buttons_cmd{BTN_ZERO=0, BTN_ARM, BTN_IGNITE,BTN_SHUT_DOWN_SERVER,BTN_SHUT_DOWN_CLIENT} btn_pressed_t; //TODO: Add connect button, also 

int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input);
int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code);
int select_state(btn_pressed_t input,connection_params_t* params);
int execute_command(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code);
int command_handshake(connection_params_t* params, wireless_data_t msg_send);
int exit_client(connection_params_t* params);