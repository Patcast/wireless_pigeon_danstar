#include "sever_mgr.h"


int main(int argc, char **argv) { /// SET UP SERVER 
   unsigned int myport, lisnum, res;

    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;

    if (argv[2])
        lisnum = atoi(argv[2]);
    else
        lisnum = 2; //TODO:Will it be better to only handle one connection at the time and only queue one connection.  
    
    res = start_server(myport,lisnum,argv[3],argv[4]);
    return res;
}