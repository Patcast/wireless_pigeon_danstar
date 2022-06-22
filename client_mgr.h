#include "conn_mgr.h"

typedef enum buttons_cmd{BTN_CONNECT=0, BTN_ARM, BTN_IGNITE, BTN_RESET,BTN_SHUT_DOWN_SERVER,BTN_SHUT_DOWN_CLIENT} btn_pressed_t;

int start_client (connection_params_t* params);
int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input);
int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int32_t  instruction_code);
int execute_command_handshake(connection_params_t* params, wireless_data_t msg_send);
int select_state(btn_pressed_t input,connection_params_t* params);
int shut_down_server(connection_params_t* params,commands_t command,status_t cmd_status,u_int32_t  instruction_code);
int exit_client(connection_params_t* params);