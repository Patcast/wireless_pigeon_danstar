#include "server_mgr.h"


int main(int argc, char **argv) { 
   unsigned int myport, res;

    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;

    res = run_server(myport,argv[2],argv[3],argv[4]);
    return res;
}