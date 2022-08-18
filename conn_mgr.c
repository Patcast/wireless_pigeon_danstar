#include "conn_mgr.h"




int star_host_connection (connection_params_t* params){
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    params->host_ctx = (params->host_is_client == TRUE)?SSL_CTX_new(SSLv23_client_method()):SSL_CTX_new(SSLv23_server_method());

    if (params->host_ctx == NULL) {
        ERR_print_errors_fp(stdout);
        return 1;
    }

    SSL_CTX_set_verify(params->host_ctx, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    if (SSL_CTX_load_verify_locations(params->host_ctx, "/home/debian/wireless_rocket_server/keys/ca.crt",NULL)<=0){ 
        printf("error on root");
        ERR_print_errors_fp(stdout);
        return 1;
    }
    // TODO: Add else if to remove returns 
    if (SSL_CTX_use_certificate_file(params->host_ctx,params->host_certificate, SSL_FILETYPE_PEM) <= 0) {
        printf("error on client cert");
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
        perror("Error creating socket");
        return 1;
    }
    printf("socket created\n");
    return 0;
}

int start_port_server (connection_params_t* params){
    
    struct sockaddr_in my_addr;

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(params->host_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    
    if (bind(params->host_fd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
        perror("error on bind");
        exit(1); //TODO: go back to before connect. 
    } else printf("binded\n");

    if (listen(params->host_fd, CONNECTIONS_QUEUE) == -1) {
        perror("error on listen");
        exit(1);
    } else printf("begin listen\n");
    params-> host_state = NOT_CONNECTED;
    return 0;
}

int connect_with_client(connection_params_t* params){
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in their_addr;
    params->host_state = NOT_CONNECTED;

    printf("starting client communication: Status %d\n",params->host_state);
    if ((params->remote_fd = accept(params->host_fd, (struct sockaddr *) &their_addr, &len))== -1) {
            perror("Error accepting socket");
            //exit(errno);
            return 1;
    } else printf("server: got connection from %s, port %d, socket %d\n",inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port),params->remote_fd);

    params->remote_ssl = SSL_new(params->host_ctx);// A new SSL based on ctx 
    SSL_set_fd(params->remote_ssl, params->remote_fd); // Add the socket of the connected user to SSL

    if (SSL_accept(params->remote_ssl) == -1) // Establish SSL connection. it calls verification callback
        {
            perror("\nError establishing the SSL connection during accept"); /// This error will be called if thereis an error with the verification.
            printf("\nVerify that client is using a valid certificate\n");
            return 1;
        }
    return 0;
}

int connect_with_server(connection_params_t* params){

    struct sockaddr_in dest;

    memset(&dest, 0,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(params->host_port);
    if (inet_aton(params->remote_ip_addr, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror(params->remote_ip_addr);
    }
    else if (connect(params->host_fd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        //exit(errno);
        return 1;
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

int ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);

    if(cert!=NULL && SSL_get_verify_result(ssl) == X509_V_OK){
        printf("Certificate verification passed\n");
        printf("Digital certificate information:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
        return 0; 
    } else{
        printf("No certificate information!\n");
        return 1;
    }   
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
    int bytes1 = sizeof(msg_received->command);
    int bytes2 = sizeof(msg_received->cmd_status);
    int bytes3 = sizeof(msg_received->instruction_code);

    printf("\n------------------\n");

    bytes1 = SSL_read(params->remote_ssl, (void *) &(msg_received->command),bytes1);    
    bytes2 = SSL_read(params->remote_ssl, (void *) &(msg_received->cmd_status), bytes2);
    bytes3 =  SSL_read(params->remote_ssl, (void *) &(msg_received->instruction_code), bytes3);


    if((bytes1 <=0)){
         params->host_state = NOT_CONNECTED;
            if(SSL_get_error(params->remote_ssl,bytes1) ==SSL_ERROR_ZERO_RETURN){
                printf("Client closed connection.\n");
                return 1;
            }
            else{
                printf("Error receiving message from client.\n");
                return 1;
            }  
    }
    else {
        printf("\ncommand: %d\ncommand status: %d\ninstruction code: %d\n\n",
        msg_received->command,
        msg_received->cmd_status,
        msg_received->instruction_code);
        return 0;
    }
}
int close_remote_conn(connection_params_t* params){
    printf("Attempting to close client: Status %d\n", params->host_state);
    if (params->host_state == ZERO)SSL_shutdown(params->remote_ssl);
    close(params->remote_fd);//TODO: Check that  this is close properly. And only when needed.
    SSL_free(params->remote_ssl);
    ///params->host_state= NOT_CONNECTED;
    printf("Connection with client closed: Status %d.\n", params->host_state);
    return 0;
}


int close_host_conn(connection_params_t* params){

    if (params->host_is_client == TRUE){ // it should only be done in the case of client. The server does this previously for each client 
        SSL_shutdown(params->remote_ssl);
        SSL_free(params->remote_ssl);
    }
    close(params->host_fd);
    SSL_CTX_free(params->host_ctx);
    printf("device is disconnected: Status %d\n",params->host_state);
    //ERR_remove_state(0); Commented as it casued an error, claming be depreciated. Newer version of openssl, should do it by itself.
    //ERR_remove_thread_state(NULL);
    //ERR_free_strings();
    //EVP_cleanup();
    //CRYPTO_cleanup_all_ex_data();
    //sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
    return 0;
}