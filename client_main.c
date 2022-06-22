#include "client_mgr.h"

int main(int argc, char **argv){

    if (argc != 5) {
    printf("Parameter format error! The correct usage is as follows:\n\t\t%s IP Address port\n\t such as:\t%s 127.0.0.1 80\n This program is used to"
             "IP The server with the address receives the most from one port MAXBUF Bytes of messages",
             argv[0], argv[0]);
    exit(0);
    }


    run_client(argv[1], atoi(argv[2]),argv[3], argv[4]);

    return 0;
}

