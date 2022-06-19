/**
 * \author Patricio Adolfo Castillo Calderon
 */


#ifndef _CONFIG_H_
#define _CONFIG_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "errmacros.h"

#define MAXBUF 1024

#define TRUE             1
#define FALSE            0
#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    //force macro-expansion on s before stringify s



#endif /* _CONFIG_H_ */
