#include "config.h"

#define CONNECTIONS_QUEUE 1

typedef enum server_status {SHUT_DOWN =0, NOT_CONNECTED,CONNECTED, ARM, IGNITE} status_server_t;

typedef struct{
    SSL_CTX* pt_ctx_server;
    status_server_t current_server_status;
    SSL* pt_ssl_server;
    int pt_server_sockfd;
    const char* ip_address;
    int myport;
    const char* certificate;
    const char* priv_key;

} connection_params_t;

int start_server ( connection_params_t* params);
int connections_handler(int* pt_server_sockfd, SSL_CTX** pt_ctx_server, status_server_t* current_status);
void ShowCerts(SSL** pt_ssl, int* pt_new_fd ,status_server_t* current_state);
void messsage_handler (SSL** pt_ssl_client, int* pt_client_fd,status_server_t* current_status);
void close_ssl_client_connection(SSL** pt_ssl,int* pt_new_fd,status_server_t* current_status);
void shut_down_server( SSL_CTX** pt_ctx_server, int* pt_server_sockfd,status_server_t* current_status);
