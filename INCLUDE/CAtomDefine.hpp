#ifndef __ATOM_DEFINE_HPP__
#define __ATOM_DEFINE_HPP__ 1

//include Header File For Command
#include "CommonCode.hpp"
#include "CommandFormat.hpp"

//Command Redefine
#define ATOM_CMD_APP_STATUS 	CMD_APP_STATUS 		//10002
#define ATOM_CMD_APP_START 		CMD_APP_START 		//10003
#define ATOM_CMD_APP_STOP 		CMD_APP_STOP 		//10004
#define ATOM_CMD_APP_RESTART 	CMD_APP_RESTART 	//10005
#define ATOM_CMD_APP_KILL 		CMD_APP_KILL		//10006
#define ATOM_CMD_APP_INIT 		CMD_APP_INIT 		//10007
#define ATOM_CMD_APP_SUSPEND 	CMD_APP_SUSPEND 	//10008
#define ATOM_CMD_APP_RESUME 	CMD_APP_RESUME 		//10009
#define ATOM_CMD_APP_LOGLEVEL 	CMD_APP_LOGLEVEL 	//10010
#define ATOM_CMD_APP_CTL 		CMD_APP_CTL 		//10100
#define ATOM_CMD_APP_TEST 		99990

//Memory Buffer
#define ATOM_DEF_MEM_BUF_64 64
#define ATOM_DEF_MEM_BUF_1024 1024
#define ATOM_DEF_MEM_BUF_1M (ATOM_DEF_MEM_BUF_1024 * ATOM_DEF_MEM_BUF_1024)

//Argument Index
#define ATOM_DEF_IDX_PKG_NO      0
#define ATOM_DEF_IDX_PKG_NAME    1
#define ATOM_DEF_IDX_NODE_NO     2
#define ATOM_DEF_IDX_NODE_NAME   3
#define ATOM_DEF_IDX_PROC_NO     4
#define ATOM_DEF_IDX_PROC_NAME   5

//Log Dup Period
#define ATOM_DEF_LOG_DUP_PERIOD 5

//Error Code
#define E_ATOM_CTL_INIT          9001
#define E_ATOM_CONF_INIT         9002
#define E_ATOM_LOG_ALLOC         9003
#define E_ATOM_LOG_INIT          9004
#define E_ATOM_Q_INIT            9005
#define E_ATOM_RES_SEND_FAIL     9006
#define E_ATOM_CMD_SET_FAIL      9007
#define E_ATOM_CMD_PTR_ERR       9008
#define E_ATOM_CMD_PTR_ERR       9008
#define E_ATOM_CMD_PARSE_ERR     9009
#define E_ATOM_CMD_UNKNOWN       9010
#define E_ATOM_CMD_SEND_FAIL     9011
#define E_ATOM_MEMORY_OVER       9012
#define E_ATOM_RES_RECV_FAIL     9013

//Queue Type
#define ATOM_DEF_QUEUE_TYPE_READ 0
#define ATOM_DEF_QUEUE_TYPE_WRITE 1

//Alarm Code
#define ATOM_DEF_ALM_UNKNOWN_CMD "90701001"

//General
#define ATOM_DEF_CMD_BLOCK_TIME 5

enum trace_idx
{
	ATOM_DEF_IDX_TRACE_IMSI = 0,
	ATOM_DEF_IDX_TRACE_MSISDN,
	ATOM_DEF_IDX_TRACE_CALLER_IP,
	ATOM_DEF_IDX_TRACE_CALLEE_IP,
	ATOM_DEF_MAX_TRACE_IDX
};

typedef struct _trace_info
{
	std::vector<std::string> vecData;
	int narrTraceReqNo[ATOM_DEF_MAX_TRACE_IDX];
}TRACE_INFO;


#endif
