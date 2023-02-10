#include "conn_mgr.h"

#define ARM_SIGNAL 0x1B
#define IGNITE_SIGNAL 0x35

#define CONNECT_BTN_GPIO    30  // P9_11
#define ARM_BTN_GPIO        31  // P9_13
#define IGN_BTN_GPIO        48  // P9_15
#define RESET_BTN_GPIO      3   // P9_21
#define SHUT_BTN_GPIO       49  // P9_23

#define LED_CON     86
#define LED_ARM     87 
#define LED_IGN     10 
#define LED_RESET   9

typedef struct gpio_btn {
    gpio* con_gpio_btn;
    gpio* arm_gpio_btn;
    gpio* ign_gpio_btn;
    gpio* reset_gpio_btn;
    gpio* shut_gpio_btn;

    gpio* con_gpio_led;
    gpio* arm_gpio_led;
    gpio* ign_gpio_led;
    gpio* reset_gpio_led;
}gpio_set_t;


typedef struct individual_callback_params {
    int gpio_id;
    connection_params_t* connection_params;
    gpio_set_t * pt_gpio_set;

}individual_callback_params_t;

typedef struct callback_params {
   individual_callback_params_t con_callback;
   individual_callback_params_t arm_callback;
   individual_callback_params_t ign_callback;
   individual_callback_params_t reset_callback;
   individual_callback_params_t shut_callback;
}callback_params_t;



 

typedef enum buttons_cmd{BTN_CONNECT =0,BTN_ZERO, BTN_ARM, BTN_IGNITE,BTN_SHUT_DOWN_SERVER,BTN_SHUT_DOWN_CLIENT} btn_pressed_t; //TODO: Add connect button, also 

int run_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input, const char* ca_cert);

gpio_set_t * start_gpios(connection_params_t* pt_connection_params, callback_params_t* pt_call_params);
gpio* start_individual_btn_gpio(individual_callback_params_t* );
gpio* start__led_gpio(int led_id);
int connect_with_rocket(connection_params_t* params,commands_t command,status_t cmd_status,u_int8_t  instruction_code);
int select_state(btn_pressed_t input,individual_callback_params_t *  c_params);
int execute_command(individual_callback_params_t *  c_params,commands_t command,status_t cmd_status,u_int8_t  instruction_code);
int command_handshake(connection_params_t* params, wireless_data_t msg_send);
connection_params_t*  start_client(const char* ip_address_input, const int myport_input, const char* certificate_input, const char* priv_key_input, const char* ca_cert);
int close_client(connection_params_t* params);
int close_gpio(gpio_set_t *  btn_gpios);
