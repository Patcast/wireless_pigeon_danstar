#include "sever_mgr.h"

enum State currState = NOT_CONNECTED;

enum State FindState() {
    return currState;
}

int connections_handler(int* pt_server_sockfd, SSL_CTX** pt_ctx_server){
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in their_addr;
    char buf[MAXBUF + 1];

    int* pt_client_fd =  malloc(sizeof(int));
	MEMORY_ERROR( pt_client_fd);
    SSL** pt_ssl_client =  malloc(sizeof(SSL*));
	MEMORY_ERROR( pt_ssl_client);	

    while (1) {
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
            close_ssl_client_connection(pt_ssl_client, pt_client_fd); 
        }
        else{
            currState = CONNECTED;
            ShowCerts(pt_ssl_client,pt_client_fd);
            /* Start processing data transfer on each new connection */
            memset(&buf, 0,MAXBUF + 1);
            strcpy(buf, "server -> client: Connection established");
            /* Send message to client */
            len = SSL_write(*pt_ssl_client, buf, strlen(buf));

            if (len <= 0) {
                printf("news'%s'Sending failed! The error code is%d,The error message is'%s'\n", buf, errno, strerror(errno));
                //goto finish;
                close_ssl_client_connection(pt_ssl_client, pt_client_fd);
            } else
                printf("news'%s'Sent successfully, sent in total%d Byte!\n", buf, len);

            memset(&buf, 0,MAXBUF + 1);
            len = SSL_read(*pt_ssl_client, buf, MAXBUF); // Be carful with the maximum BUFFER size 
            if (len > 0)  printf("Message received successfully:'%s |||| size: %d Bytes of data\n", buf, len);
            else printf("Message reception failed! The error code is%d, The error message is'%s'\n", errno, strerror(errno));
            close_ssl_client_connection(pt_ssl_client, pt_client_fd);
        }
    }
}

int start_server (int myport, int lisnum, const char* certificate, const char* priv_key){
    
    
    struct sockaddr_in my_addr;
    SSL_CTX** pt_ctx_server= malloc(sizeof( SSL_CTX*));
    MEMORY_ERROR( pt_ctx_server);

    int* pt_server_sockfd =  malloc(sizeof(int));
	MEMORY_ERROR( pt_server_sockfd);
  
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    *pt_ctx_server = SSL_CTX_new(SSLv23_server_method()); //TODO: Might be better to use 1.2 TLS 
    if (*pt_ctx_server == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    SSL_CTX_set_verify(*pt_ctx_server, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL); //WORKING: verify what are this options doing???
    if (SSL_CTX_load_verify_locations(*pt_ctx_server, "keys/ca.crt",NULL)<=0){
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (SSL_CTX_use_certificate_file(*pt_ctx_server, certificate, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (SSL_CTX_use_PrivateKey_file(*pt_ctx_server, priv_key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    if (!SSL_CTX_check_private_key(*pt_ctx_server)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    if ((*pt_server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    } else
        printf("socket created\n");

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(*pt_server_sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
        perror("error on bind");
        exit(1);
    } else
        printf("binded\n");

    if (listen(*pt_server_sockfd, lisnum) == -1) {
        perror("error on listen");
        exit(1);
    } else
        printf("begin listen\n");

    connections_handler(pt_server_sockfd,pt_ctx_server);

    /* Turn off listening socket */
    close(*pt_server_sockfd); //TODO: Check correct implementation
    /* Release CTX */
    SSL_CTX_free(*pt_ctx_server);
    return 0;
}


void close_ssl_client_connection(SSL** pt_ssl,int* pt_new_fd){
    if (FindState() == CONNECTED) SSL_shutdown(*pt_ssl);
    SSL_free(*pt_ssl);
    close(*pt_new_fd);
    printf("Connection with client is successfully closed.");
}

/* static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
 {
    if(preverify_ok)printf("\nCertificate verified by call back!!\n");
    else printf("\n\nVerification failed  during call back.\nCertificate Missing or invalid.\n\n");
    return preverify_ok;
 } */

//WORKING: handle incoming crtificates better!
void ShowCerts(SSL** pt_ssl, int* pt_new_fd )
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(*pt_ssl); /// 
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
        close_ssl_client_connection(pt_ssl, pt_new_fd);
    }
}

