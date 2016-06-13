#ifndef __CLA_H__
#define __CLA_H__

#define CLA_OK 0
#define CLA_NOK 1

#define CLA_RSLT_PEER_CLOSED  100
#define CLA_RSLT_PROCESS_END  101

#define CLA_RSLT_CODE_TIME_OUT 200
#define CLA_RSLT_CODE_SEND_ERROR 201

#define STRING_NUMBER_LEN 11
#define CMD_NAME_LEN 32
#define PKG_NAME_LEN 40
#define CMD_DEST_LEN 18

#define CLA_ERR 1
#define CLA_LOG(_LVL,...){\
	CGlobal *d_inst = NULL;\
	d_inst = CGlobal::GetInstance();\
	d_inst->GetLogP()->ERROR(__VA_ARGS__);\
}

#endif
