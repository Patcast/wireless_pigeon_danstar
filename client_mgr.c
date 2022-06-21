#include "client_mgr.h"

#define MAXBUF 1024

int start_client (const char* ip_address, const int myport, const char* certificate, const char* priv_key){

    status* current_status = malloc(sizeof(status*));
    MEMORY_ERROR( current_status);
    *current_status = SHUT_DOWN;

    SSL_CTX** pt_ctx_client= malloc(sizeof( SSL_CTX*)); 
    MEMORY_ERROR( pt_ctx_client);
      SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    *pt_ctx_client = SSL_CTX_new(SSLv23_client_method());
    if (*pt_ctx_client == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    SSL_CTX_set_verify(*pt_ctx_client, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    if (SSL_CTX_load_verify_locations(*pt_ctx_client, "keys/ca.crt",NULL)<=0){ 
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (SSL_CTX_use_certificate_file(*pt_ctx_client,certificate, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (SSL_CTX_use_PrivateKey_file(*pt_ctx_client,priv_key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (!SSL_CTX_check_private_key(*pt_ctx_client)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    connections_handler(ip_address, myport, pt_ctx_client, current_status);
    return 0;
}

int connections_handler(const char * ip_address, const int myport,SSL_CTX** pt_ctx_client, status* current_status){

    struct sockaddr_in dest;
    int* pt_client_sockfd =  malloc(sizeof(int)); //stockfd
	MEMORY_ERROR( pt_client_sockfd);


    SSL** pt_ssl_server= malloc(sizeof( SSL*)); 
    MEMORY_ERROR( pt_ssl_server);


    if ((*pt_client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }
    printf("socket created\n");
    memset(&dest, 0,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(myport);
    if (inet_aton(ip_address, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror(ip_address);
        exit(errno);
    }
    printf("address created\n");

    if (connect(*pt_client_sockfd , (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }
    printf("server connected\n");

    *pt_ssl_server = SSL_new(*pt_ctx_client);
    SSL_set_fd(*pt_ssl_server, *pt_client_sockfd );

    if (SSL_connect(*pt_ssl_server) == -1)
        ERR_print_errors_fp(stderr);
    else {
        printf("Connected with %s encryption\n", SSL_get_cipher(*pt_ssl_server));
        ShowCerts(*pt_ssl_server);
        messsage_handler(pt_ctx_client, current_status, pt_ssl_server, pt_client_sockfd);

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


void messsage_handler(SSL_CTX** pt_ctx_client, status* current_status, SSL** pt_ssl_server, int* pt_client_sockfd)
{
  
    char buffer[MAXBUF + 1];
    int bytes;
    int result;
    printf("Type to start\n");
    char keep_connection = TRUE;

    wireless_data_t data;
    data.cmd_status=1;
    data.instruction_code=555;
    data.state_requested=22;

    do{
        memset(&buffer, 0,MAXBUF + 1);
        scanf("%s",(char*) &buffer);
        if(strcmp(buffer, "exit")==0){
            keep_connection = FALSE;
        }
        else{
            bytes = sizeof(data.state_requested);
            if( SSL_write(* pt_ssl_server, (void *) &data.state_requested, bytes)<=0 ){printf("Sending failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));}
            bytes = sizeof(data.cmd_status);
            if( SSL_write(* pt_ssl_server, (void *) &data.cmd_status, bytes)<=0 ){printf("Sending failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));}
            bytes = sizeof(data.instruction_code);
            if( SSL_write(* pt_ssl_server, (void *) &data.instruction_code, bytes)<=0 ){printf("Sending failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));}

            memset(&buffer, 0,MAXBUF + 1);
            bytes = SSL_read(* pt_ssl_server, buffer, MAXBUF);
            if (bytes > 0)
                printf("Message from Server: %s\n", buffer);
            else {
                printf("Message reception failed! The error code is%d，The error message is'%s'\n",errno, strerror(errno));
                close_connection(pt_ssl_server,pt_client_sockfd,pt_ctx_client, current_status);
            }
        }
    }while(keep_connection);

    close_connection(pt_ssl_server,pt_client_sockfd,pt_ctx_client, current_status);    
}

void close_connection(SSL** pt_ssl_server,int* pt_client_sockfd, SSL_CTX** pt_ctx_client, status* current_status){
    SSL_shutdown(* pt_ssl_server);
    SSL_free(* pt_ssl_server);
    close(* pt_client_sockfd);
    SSL_CTX_free(* pt_ctx_client);
    printf("Client was shutted down: Status %d\n", *current_status);

    free( pt_ssl_server);
    free(pt_client_sockfd);
    free(pt_ctx_client);
    free( current_status);

    ERR_remove_state(0);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
}