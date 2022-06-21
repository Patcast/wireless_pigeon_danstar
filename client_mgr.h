#include <resolv.h>
#include "config.h"



typedef enum server_status {SHUT_DOWN =0, NOT_CONNECTED,CONNECTED, ARM, IGNITE} status;

int start_client (const char* ip_address, const int myport, const char* certificate, const char* priv_key);
int connections_handler(const char * ip_address, const int myport,SSL_CTX** pt_ctx_client, status* current_status);
void ShowCerts(SSL * ssl);
void messsage_handler(SSL_CTX** pt_ctx_client, status* current_status, SSL** pt_ssl_server, int* pt_client_sockfd);
void close_connection(SSL** pt_ssl_server,int* pt_client_sockfd, SSL_CTX** pt_ctx_client, status* current_status);




