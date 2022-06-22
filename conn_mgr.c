#include "conn_mgr.h"

//#define MAXBUF 1024 


int star_host_connection (connection_params_t* params){
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    params->host_ctx = SSL_CTX_new(SSLv23_client_method());
    if (params->host_ctx == NULL) {
        ERR_print_errors_fp(stdout);
        return 1;
    }
    SSL_CTX_set_verify(params->host_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    if (SSL_CTX_load_verify_locations(params->host_ctx, "keys/ca.crt",NULL)<=0){ 
        ERR_print_errors_fp(stdout);
        return 1;
    }
    if (SSL_CTX_use_certificate_file(params->host_ctx,params->host_certificate, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        return 1;
    }
    if (SSL_CTX_use_PrivateKey_file(params->host_ctx,params->host_key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        return 1;
    }
    if (!SSL_CTX_check_private_key(params->host_ctx)) {
        ERR_print_errors_fp(stdout);
        return 1;
    }
       if ((params->host_fd= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        return 1;
    }
    return 0;
}

int connect_with_server(const char * ip_address, const int myport,connection_params_t* params){

    struct sockaddr_in dest;

    printf("socket created\n");
    memset(&dest, 0,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(myport);
    if (inet_aton(ip_address, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror(ip_address);
        exit(errno); //TODO: handle exit properly.
    }
    printf("address created\n");
    if (connect(params->host_fd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }
    params->remote_ssl = SSL_new(params->host_ctx);
    SSL_set_fd(params->remote_ssl, params->host_fd );

    if (SSL_connect(params->remote_ssl) == -1){
         ERR_print_errors_fp(stderr);
         return 1;
    }
       
    else {
        printf("Server connected with %s encryption\n", SSL_get_cipher(params->remote_ssl));
    }
    return 0;
}

void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    // SSL_get_verify_result()This is the point. SSL_CTX_set_verify()Only when the configuration is enabled or not and the authentication is not executed, can the function be called to verify the authentication
    // If the verification fails, the program throws an exception to terminate the connection
    if(SSL_get_verify_result(ssl) == X509_V_OK){
        printf("Certificate verification passed\n");
    }
    if (cert != NULL) {
        printf("Digital certificate information:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("No certificate information!\n");
}


int write_to_remote(connection_params_t* params,wireless_data_t msg)
{
  
    int bytes;
    char keep_connection = TRUE;
       bytes = sizeof(msg.command);
            if( SSL_write(params->remote_ssl, (void *) &msg.command, bytes)<=0 ){
                return 1;}
            bytes = sizeof(msg.cmd_status);
            if( SSL_write(params->remote_ssl, (void *) &msg.cmd_status, bytes)<=0 ){
                return 1;}  
            bytes = sizeof(msg.instruction_code);
            if( SSL_write(params->remote_ssl, (void *) &msg .instruction_code, bytes)<=0 ){
                return 1;}
    
    return 0;
}

int read_from_remote(connection_params_t* params, wireless_data_t* msg_received){
    int bytes;

    printf("\n------------------\n");
    bytes = sizeof(msg_received->command);
    if( SSL_read(params->remote_ssl, (void *) &(msg_received->command),bytes)<=0)return 1;
       
    bytes = sizeof(msg_received->cmd_status);
    if( SSL_read(params->remote_ssl, (void *) &(msg_received->cmd_status), bytes)<=0 ) return 1;

    bytes = sizeof(msg_received->instruction_code);
    if( SSL_read(params->remote_ssl, (void *) &(msg_received->instruction_code), bytes)<=0 ) return 1;
    else printf("\nmessage received: %d\ncommand status: %d\ninstruction code: %d\n\n",msg_received->command,msg_received->cmd_status,msg_received->instruction_code);
}

int close_connection(connection_params_t* params){
    SSL_shutdown(params->remote_ssl);
    SSL_free(params->remote_ssl);
    close(params->host_fd);
    SSL_CTX_free(params->host_ctx);
    printf("Client was shutted down: Status %d\n",params->host_state);
    ERR_remove_state(0);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
    return 0;
}