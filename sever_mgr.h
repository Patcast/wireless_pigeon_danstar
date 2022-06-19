#include "config.h"

enum State {NOT_CONNECTED =0, CONNECTED};

enum State FindState();
int connections_handler(int* pt_server_sockfd, SSL_CTX** pt_ctx_server);
int start_server (int myport, int lisnum, const char* certificate, const char* priv_key);
void close_ssl_client_connection(SSL** pt_ssl,int* pt_new_fd );
void ShowCerts(SSL** pt_ssl, int* pt_new_fd );