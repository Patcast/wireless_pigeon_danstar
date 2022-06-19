#ifndef __errmacros_h__
#define __errmacros_h__

#include <errno.h>

#define MEMORY_ERROR(ptr) 									\
		do {												\
			if ( (ptr) == NULL )							\
			{												\
 				fprintf(stderr,"\nIn %s - function %s at line %d: There was a syncronization error", __FILE__, __func__, __LINE__);\
				fflush(stderr); \
			}												\
		} while(0)
			
#define ASPRINTF_ERROR(err) 								\
		do {												\
			if ( (err) == EOF )								\
			{												\
				perror("asprintf failed");					\
			}												\
		} while(0)
#define FFLUSH_ERROR(err) 								\
		do {												\
			if ( (err) == EOF )								\
			{												\
				perror("fflush failed");					\
				pthread_exit(NULL);						\
			}												\
		} while(0)
#endif
