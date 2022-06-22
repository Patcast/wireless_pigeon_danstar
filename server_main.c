#include "server_mgr.h"


int main(int argc, char **argv) { /// SET UP SERVER 
   unsigned int myport, res;

    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;

    res = run_server(myport,argv[2],argv[3]);
    return res;
}