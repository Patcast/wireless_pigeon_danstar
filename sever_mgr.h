#include "config.h"

#define CONNECTIONS_QUEUE 1

typedef enum server_status {SHUT_DOWN =0, NOT_CONNECTED,CONNECTED, ARM, IGNITE} status;

int start_server (int myport, const char* certificate, const char* priv_key);
int connections_handler(int* pt_server_sockfd, SSL_CTX** pt_ctx_server, status* current_status);
void ShowCerts(SSL** pt_ssl, int* pt_new_fd ,status* current_state);
void messsage_handler (SSL** pt_ssl_client, int* pt_client_fd,status* current_status);
void close_ssl_client_connection(SSL** pt_ssl,int* pt_new_fd,status* current_status);
void shut_down_server( SSL_CTX** pt_ctx_server, int* pt_server_sockfd,status* current_status);
