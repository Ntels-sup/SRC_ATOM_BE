#ifndef __STA_H__
#define __STA_H__

#define STA_NOK 1
#define STA_OK 0

#define MAX_MESSAGE_BUFFER_LEN 8192
#define FILE_LINE_BUFFER_LEN 1024

#define FILE_LOG_EXT ".LOG"
#define FILE_LOG_EXT_LEN 4

#define FILE_NAME_LEN 1024

#define MSG_FLAG_REQUEST 
#define MSG_FLAG_

#define RECEIVE_WAIT_TIME 3

#define STA_ERR 1
#if 1
#define STA_LOG(_LVL,...){\
    CGlobal *inst = NULL;\
    inst = CGlobal::GetInstance();\
    inst->GetLogP()->ERROR(__VA_ARGS__);\
}
#define DEBUG_LOG(...){\
    CGlobal *inst = NULL;\
    inst = CGlobal::GetInstance();\
    inst->GetLogP()->DEBUG(__VA_ARGS__);\
}
#else
#define STA_LOG(_LVL,...){\
    fprintf(stderr,__VA_ARGS__);\
}
#define DEBUG_LOG(...){\
    fprintf(stderr,__VA_ARGS__);\
}
#endif

#endif
