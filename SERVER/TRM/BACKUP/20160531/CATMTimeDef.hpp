#ifndef __ATMTimeDefHPP__
#define __ATMTimeDefHPP__

#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define  NL_TRUE   1
#define  NL_FALSE  0

#define  NL_RETRY    1
#define  NL_OK       0
#define  NL_ERROR   -1
#define  NL_TIMEOUT -2

#define NL_DISPLAY_SYS_ERROR(text) do { \
    printf("SYSERR:[%s][%d]  %s %d-%s\n", __FILE__,__LINE__,text,errno,strerror(errno)) ; \
    } while (0) 

#define NL_DISPLAY_SYS_THREAD_ERROR(code,text) do { \
    printf("SYSERR:[%s][%d]  %s %d-%s\n", __FILE__,__LINE__,text,code,strerror(code)) ; \
    } while (0) 
#define NL_DISPLAY_LIB_ERROR(text) do { \
    printf("LIBERR:[%s][%d] : %s\n", __FILE__,__LINE__,text) ; \
    } while (0)

#define NL_DISPLAY_LIB_WARRING(text) do { \
    printf("LIBWAR:[%s][%d] : %s\n", __FILE__,__LINE__,text) ; \
    } while (0) 

#define NL_INVALID_DATE 0x7fffffffl

#define NL_PROCESS_CHECK_NONE 0x00
#define NL_PROCESS_CHECK_USER 0x01
#define NL_PROCESS_CHECK_ARGS 0x10

typedef unsigned char byte ;
typedef int           boolean ;

#endif
