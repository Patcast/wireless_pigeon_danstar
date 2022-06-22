#include "conn_mgr.h"

typedef enum buttons_cmd{BTN_CONNECT=0, BTN_ARM, BTN_IGNITE, BTN_RESET,BTN_SHUT_DOWN} btn_pressed_t;

int start_client (connection_params_t* params);
int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input);
int run_connect(connection_params_t* params);
int select_state(btn_pressed_t input,connection_params_t* params);