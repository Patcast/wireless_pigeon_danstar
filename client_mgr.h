#include "conn_mgr.h"
#include "libsoc_gpio.h"
#include "libsoc_debug.h" 

#define ARM_SIGNAL 0xFF
#define IGNITE_SIGNAL 0xFF

#define CONNECT_BTN_GPIO    30
#define ARM_BTN_GPIO        31
#define IGN_BTN_GPIO        3
#define RESET_BTN_GPIO      115
#define SHUT_BTN_GPIO       49

typedef struct gpio_btn {

    gpio* con_gpio_btn;
    gpio* arm_gpio_btn;
    gpio* ign_gpio_btn;
    gpio* reset_gpio_btn;
    gpio* shut_gpio_btn;
}gpio_set_t;

typedef struct callback_params {

    gpio* gpio_btn_c;
    connection_params_t* params_c;
    
}callback_params_t;

typedef enum buttons_cmd{BTN_CONNECT =0,BTN_ZERO, BTN_ARM, BTN_IGNITE,BTN_SHUT_DOWN_SERVER,BTN_SHUT_DOWN_CLIENT} btn_pressed_t; //TODO: Add connect button, also 

int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input);

gpio_set_t * start_gpios(connection_params_t* params);
gpio* start_individual_btn_gpio(connection_params_t* params,int btn_id);

int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code);
int select_state(btn_pressed_t input,connection_params_t* params);
int execute_command(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code);
int command_handshake(connection_params_t* params, wireless_data_t msg_send);
connection_params_t*  start_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input);
int exit_client(connection_params_t* params);