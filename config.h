/**
 * \author Patricio Adolfo Castillo Calderon
 */


#ifndef _CONFIG_H_
#define _CONFIG_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <resolv.h>// Only for client 

#include "gpio_handler.h"
#include "errmacros.h"

//#define MAXBUF 1024
#define CONNECTIONS_QUEUE 1

#define TRUE             1
#define FALSE            0
#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    //force macro-expansion on s before stringify s

typedef enum {ZERO_CO=0,ARM_CO,IGNITE_CO,SHUTDOWN_CO} commands_t; /// TODO: Change connect for sero state and delete reset  
typedef enum {REQUESTED=0,ACK} status_t;

typedef struct {
    commands_t command;         
    status_t cmd_status;   
    u_int8_t  instruction_code;         
} wireless_data_t;

typedef enum host_states {
    //IDLE =0, 
    // SERVER: Server is running but, port is not open, might be unecessary???
    NOT_CONNECTED=0, 
    // SERVER: Port is open, but not client connected 
    // CLIENT: Starting state
    ZERO,  
    // SERVER: Client connected, GPIO set to zero
    ARM_REQ, 
    ARM,  
    // SERVER: Client connected, half GPIO set 
    IGNITE_REQ,
    IGNITE, 
    // SERVER Client connected, all GPIO set 
    RESET_REQ,
    SHUT_DOWN
    } states_t;



typedef struct{
    SSL_CTX* host_ctx;
    states_t host_state;
    SSL* remote_ssl;
    int host_fd;
    int remote_fd;
    const char* remote_ip_addr;
    int host_port;
    const char* host_certificate;
    const char* host_key;
    char host_is_client;
} connection_params_t;


#endif /* _CONFIG_H_ */


