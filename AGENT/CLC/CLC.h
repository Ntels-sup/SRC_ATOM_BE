#ifndef __CLC_H__
#define __CLC_H__

#define CLC_OK    0
#define CLC_NOK   1

#define CLC_RSLT_END   100
#define CLC_RSLT_QUIT  101

#define CLC_SESSION_TYPE_LOGIN 1
#define CLC_SESSION_TYPE_DATA  2

#define CLC_NUMBER_STRING_LEN 11+1
#define CLC_ARG_TYPE_LEN 32+1
#define CLC_ARG_ENUM_TABLE_LEN  16+1
#define CLC_ARG_ENUM_COLUMN_LEN 15+1

#define CLC_BUFFER_LEN 1024+1

#define CLC_USER_ID_LEN  20+1
#define CLC_PASSWD_LEN  64+1

#define CLC_RSLT_CODE_SEND_ERROR 100
#define CLC_RSLT_CODE_TIME_OUT   101

#define TMR_EVNT_CONN_TIMEOUT  1
#define TMR_EVNT_CONN_WAIT     2

#define CLC_ERR   1 

#if 0
#define CLC_LOG(_LVL, ...){\
    fprintf(stderr,__VA_ARGS__);\
}
#else 
#define CLC_LOG(_LVL,...){\
	    CGlobal *inst = NULL;\
	    inst = CGlobal::GetInstance();\
	    inst->GetLogP()->ERROR(__VA_ARGS__);\
}
#endif

#endif
