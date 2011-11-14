/*
 * util_header.h
 *
 *  Created on: Nov 9, 2011
 *      Author: gassa
 */

#ifndef UTIL_HEADER_H_
#define UTIL_HEADER_H_


/*
 * -----------------------------------------------------------------------------
 * standard C programming headers, basic Unix system programming headers
 * -----------------------------------------------------------------------------
 */
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<time.h>		/* for generating random id */
#include	<netinet/in.h>
#include	<unistd.h>      /* basic Unix system programming          */
#include	<sys/types.h>	/* basic system data types                */
#include	<sys/time.h>	/* timeval{} for select()                 */
#include	<sys/wait.h>    /* 'wait' and 'waitpid'                   */
#include    <sys/select.h>  /* IO multiplexing                        */
#include 	<netdb.h>       /* name and address conversion            */
#include 	<arpa/inet.h>   /* inet_pton/ntop                         */
#include 	<unistd.h>      /* read, write, close, exec, etc.         */
#include	<errno.h>       /* error handling                         */
#include	<signal.h>      /* signal handling                        */

/* global variable */
#define	MAXLINE	     4096     /* max text line length : 4K         */
#define	BUF_SIZE     8192     /* buffer size for reads and writes  */
#define TRUE         1
#define FALSE        0

#ifndef NULL
#define NULL (void *) 0     /* just a NULL pointer               */
#endif

/*log config*/
#define FATAL_ERROR 5
#define ERROR 4
#define WARNING 3
#define INFO 2
#define NOTE 1
#define DEBUG 0

typedef void Sigfunc(int); /* for signal handlers               */

#define	min(a,b)     ((a) < (b) ? (a) : (b))
#define	max(a,b)     ((a) > (b) ? (a) : (b))

int max_citizen_number;

#define GREETING_MSG "\n\
   **************************************************************\n\
           \n\
			Yin Yan, project 2 for Mordern Network Concept\n\
           \n\
   **************************************************************\n"

#endif /* UTIL_HEADER_H_ */
