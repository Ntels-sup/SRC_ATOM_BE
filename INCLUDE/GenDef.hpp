#include <stdio.h>
#include <time.h>

#ifdef VOLATILE
#undef VOLATILE
#define VOLATILE        volatile
#else
#define VOLATILE        volatile
#endif /* end of VOLATILE */

/* basic type of value  definition */
typedef void			VOID;
typedef unsigned int    UINT;
typedef signed int      SINT;
typedef unsigned short  USHORT;
typedef signed short    SSHORT;
typedef unsigned long   ULONG;
typedef signed long     SLONG;
typedef unsigned char   UCHAR;
typedef signed char     SCHAR;
typedef char            CHAR;
typedef double          DOUBLE;
typedef float           FLOT;
typedef size_t          SIZET;
typedef time_t          TIMET;

typedef unsigned char 	U_8;
typedef signed char   	S_8;
typedef unsigned short  U_16;
typedef signed short    S_16;
typedef unsigned int    U_32;
typedef signed int      S_32;
typedef double          S_64;

#define RC_OK 1
#define RC_NOK 0
#define RC_TRUE 1
#define RC_FALSE 0
#define RC_USED 1
#define RC_NOT_USED 0

#define RT_RESULT    signed int
