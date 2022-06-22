#include "server_pigeon.h"


//TODO: ADD LOGS ??

int start_server (connection_params_t* params){
    
    struct sockaddr_in my_addr;

   /*  status* current_status = malloc(sizeof(status*));
    MEMORY_ERROR( current_status);
    *current_status = SHUT_DOWN;

    
    SSL_CTX** pt_ctx_server= malloc(sizeof( SSL_CTX*)); 
    MEMORY_ERROR( pt_ctx_server);

    int* pt_server_sockfd =  malloc(sizeof(int)); 
	MEMORY_ERROR( pt_server_sockfd);
   */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    params->pt_ctx_server= SSL_CTX_new(SSLv23_server_method()); 
    if (params->pt_ctx_server== NULL) {
        ERR_print_errors_fp(stdout);
        exit(1); // TODO: check that the exit calls also free the memory
    }
    SSL_CTX_set_verify(params->pt_ctx_server, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL); 
        if (SSL_CTX_load_verify_locations(params->pt_ctx_server, "keys/ca.crt",NULL)<=0){
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (SSL_CTX_use_certificate_file(params->pt_ctx_server, params->certificate, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (SSL_CTX_use_PrivateKey_file(params->pt_ctx_server,params->priv_key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (!SSL_CTX_check_private_key(params->pt_ctx_server)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    if ((params->pt_server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    } else
        printf("socket created\n");

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(params->myport);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    /// SAME FOR CLIENT AND SERVER
    if (bind(params->pt_server_sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
        perror("error on bind");
        exit(1);
    } else printf("binded\n");

    if (listen(params->pt_server_sockfd, CONNECTIONS_QUEUE) == -1) {
        perror("error on listen");
        exit(1);
    } else printf("begin listen\n");

    params-> current_server_status = NOT_CONNECTED;
    connections_handler(params->pt_server_sockfd,params->pt_ctx_server,params-> current_server_status);
    shut_down_server(params->pt_ctx_server,params->pt_server_sockfd,params-> current_server_status);
  
    return 0;
}

int connections_handler(int* pt_server_sockfd, SSL_CTX** pt_ctx_server, status_server_t* current_status){
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in their_addr;
    

    int* pt_client_fd =  malloc(sizeof(int)); 
	MEMORY_ERROR( pt_client_fd);
    SSL** pt_ssl_client =  malloc(sizeof(SSL*)); 
	MEMORY_ERROR( pt_ssl_client);	

    do {
        printf("starting client communication: Status %d\n",*current_status);
        if ((*pt_client_fd = accept(*pt_server_sockfd, (struct sockaddr *) &their_addr, &len))== -1) {
            perror("Error accepting socket");
            exit(errno);
        } else printf("server: got connection from %s, port %d, socket %d\n",inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port),*pt_client_fd);

        *pt_ssl_client = SSL_new(* pt_ctx_server);// A new SSL based on ctx 
        SSL_set_fd(*pt_ssl_client, *pt_client_fd); // Add the socket of the connected user to SSL

        if (SSL_accept(*pt_ssl_client) == -1) // Establish SSL connection. it calls verification callback
        {
            perror("\nError establishing the SSL connection during accept"); /// This error will be called if thereis an error with the verification.
            printf("\nVerify that client is using a valid certificate\n");
            close_ssl_client_connection(pt_ssl_client, pt_client_fd,current_status); 
        }
        else{
            ShowCerts(pt_ssl_client,pt_client_fd,current_status);
            messsage_handler (pt_ssl_client,pt_client_fd,current_status);
            close_ssl_client_connection(pt_ssl_client, pt_client_fd,current_status);
     
        }
        *current_status = SHUT_DOWN;
    }while (!(*current_status== SHUT_DOWN));
    return 0;
}

void ShowCerts(SSL** pt_ssl, int* pt_new_fd ,status_server_t* current_state){
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(*pt_ssl); 
    if(cert!=NULL && SSL_get_verify_result(*pt_ssl) == X509_V_OK){
        printf("Digital certificate information:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert); 
    }
    else{
        printf("No certificate information!\n");
        close_ssl_client_connection(pt_ssl, pt_new_fd,current_state);
    }
}

void messsage_handler (SSL** pt_ssl_client, int* pt_client_fd,status_server_t* current_status){
    int bytes;
    char keep_connection = TRUE;
    wireless_data_t data;
    
    do{
        *current_status = CONNECTED;
        char buf[MAXBUF + 1];
        printf("\n------------------\n");

        bytes = sizeof(data.command);
        bytes = SSL_read(* pt_ssl_client, (void *) &data.command,bytes);
        if( bytes=0 ) break;
        else if (bytes<0) {printf("Message reception failed!Error code= %d. Error message is:'%s'\n", errno, strerror(errno));}
        
        bytes = sizeof(data.cmd_status);
        bytes = SSL_read(* pt_ssl_client, (void *) &data.cmd_status, bytes);
        if( bytes=0 ) break;
        else if (bytes<0) {printf("Message reception failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));}

        bytes = sizeof(data.instruction_code);
        bytes = SSL_read(* pt_ssl_client, (void *) &data.instruction_code, bytes);
        if( bytes=0 ) break;
        else if (bytes<0){printf("Message reception failed! Error code= %d. Error message is:'%s'\n", errno, strerror(errno));}
        else printf("\nstate_requested: %d\ncommand status: %d\ninstruction code: %d\n\n",data.command,data.cmd_status,data.instruction_code);


        
        memset(&buf, 0,MAXBUF + 1);
        strcpy(buf, "I got your message!\n");
        bytes = SSL_write(*pt_ssl_client, buf, strlen(buf));
        if (bytes <= 0) {
            printf("news'%s'Sending failed! The error code is %d,The error message is'%s'\n", buf, errno, strerror(errno));
            close_ssl_client_connection(pt_ssl_client, pt_client_fd,current_status);
        } 
        else printf("Message sent:%s", buf);
    }while(keep_connection);
}


void close_ssl_client_connection(SSL** pt_ssl,int* pt_new_fd, status_server_t* current_status){
    printf("Attempting to close client: Status %d\n", *current_status);
    if (*current_status == CONNECTED) {
        SSL_shutdown(*pt_ssl);
    }
    close(*pt_new_fd);
    SSL_free(*pt_ssl);
    *current_status = NOT_CONNECTED;
    free(pt_ssl);
    free(pt_new_fd);
    printf("Connection with client closed: Status %d.\n", *current_status);
}
void shut_down_server( SSL_CTX** pt_ctx_server, int* pt_server_sockfd,status_server_t* current_status){
    close(*pt_server_sockfd); 
    SSL_CTX_free(*pt_ctx_server);
    free(pt_server_sockfd);
    free(pt_ctx_server);
    printf("Server was shutted down: Status %d\n", *current_status);
    free(current_status);
    ERR_remove_state(0);
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
}




