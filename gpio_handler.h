
#ifndef GPIO_HANLDER_H_
#define GPIO_HANLDER_H_
#define _GNU_SOURCE
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    //force macro-expansion on s before stringify s


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

//const char * GPIO_DIRECTORY = "/sys/class/gpio";

typedef enum DIRECTION{
	INPUT_PIN=0,
	OUTPUT_PIN=1
}PIN_DIRECTION;

typedef enum VALUE{
	LOW=0,
	HIGH=1
}PIN_VALUE;

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, PIN_DIRECTION out_flag);
int gpio_set_value(unsigned int gpio, PIN_VALUE value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);

#endif /* GPIO_HANLDER_H_ */
