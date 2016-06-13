#ifndef __BJ_DefinesHpp__
#define __BJ_DefinesHpp__

#include <string>

#define SPACE_REMOVE_BUF_SIZE   256

#define TRM_ACTION_START        "START"
#define TRM_ACTION_STOP         "STOP"
#define TRM_ACTION_INIT         "INIT"
#define TRM_ACTION_LOGLEVEL     "LOGLEVEL"

#define TRM_NOK 					0
#define TRM_OK 						1
#define TRM_RECV_TRACE_ON			2
#define TRM_RECV_TRACE_OFF			3
#define TRM_RECV_TRACE_DATA			4
#define TRM_RECV_INIT				5
#define TRM_RECV_REGIST				6
#define TRM_RECV_ERROR				7
#define TRM_RECV_PROC_CTRL          7
#define TRM_RECV_PROC_START         8
#define TRM_RECV_PROC_STOP          9
#define TRM_RECV_REGIST_ROUTE		10

#define TRM_MAX_PROCESS		       1024
#define TRM_MAX_TRACE_ROUTE	       1024
#define TRM_MAX_TRACE_HIST	       1024
#define TRM_MAX_REQUEST		       1024
#define TRM_MAX_RESPONSE	       1024

//20160328 kjjy7 TAT_TRC_ROUTE
#define TRM_GROUP_GROUP_NAME_SIZE     40
#define TRM_GROUP_PKG_NAME_SIZE		  40
#define TRM_GROUP_CREATE_DATE_SIZE    20
#define TRM_GROUP_START_DATE_SIZE     20
#define TRM_GROUP_SCHEDULE_CYCLE_TYPE_SIZE  2
#define TRM_GROUP_EXPIRE_DATE_SIZE    20
#define TRM_GROUP_USER_ID_SIZE        20
#define TRM_GROUP_DESC_SIZE           256


//20160328 kjjy7 TAT_TRC_HIST
#define TRM_JOB_GROUP_NAME_SIZE     40
#define TRM_JOB_JOB_NAME_SIZE       40
#define TRM_JOB_PROC_NAME_SIZE	    40
#define TRM_JOB_PKG_NAME_SIZE	    40
#define TRM_JOB_NODE_TYPE_SIZE	    40
#define TRM_JOB_DESC_SIZE           256

#define TRM_REGIST					"0000000001"
#define TRM_TRACE_ON				"0000100001"
#define TRM_TRACE_OFF				"0000100002"
#define TRM_TRACE_DATA				"0000100003"
#define TRM_TRACE_RECONFIG			"0000100004"
#define TRM_TRACE_REGIST_ROUTE		"0000100005"
#define ATOM_PROC_CTL               "0000000008"

#define PROCID_ATOM_PRA                            0x7FFFFF01
#define PROCID_ATOM_RSA                            0x7FFFFF02

#define PROCID_ATOM_BLUECORE_PRA        0x7FFFFF65
#define PROCID_ATOM_BLUECORE_RSA        0x7FFFFF66
#define PROCID_ATOM_BLUECORE_STA        0x7FFFFF67
#define PROCID_ATOM_BLUECORE_ALA        0x7FFFFF68
#define PROCID_ATOM_BLUECORE_EXA        0x7FFFFF69

#define TRM_HIST_PKG_NAME_SIZE			40
#define TRM_HIST_NODE_NAME_SIZE			40
#define TRM_HIST_USER_ID_SIZE			20
#define TRM_HIST_PROTOCOL_SIZE			20
#define TRM_HIST_START_DATE_SIZE		20
#define TRM_HIST_END_DATE_SIZE			20
#define TRM_HIST_CMD_SIZE				256
#define TRM_HIST_FILENAME_SiZE			256
#define TRM_HIST_STATUS_SIZE			20

#define TRM_ROUTE_PKG_NAME_SIZE			40
#define TRM_ROUTE_CMD_SIZE				256
#define TRM_ROUTE_NODE_NAME_SIZE		40
#define TRM_ROUTE_NODE_TYPE_SIZE		40

// TAT_PROCESS
#define TRM_PROC_PKG_NAME_SIZE      40
#define TRM_PROC_PROC_NAME_SIZE     40
#define TRM_PROC_NODE_TYPE_SIZE     40
#define TRM_PROC_EXEC_BIN_SIZE      512
#define TRM_PROC_EXEC_ARG           256
#define TRM_PROC_EXEC_ENV           256
#define TRM_PROC_CPU_BIND_CORE      512
#define TRM_PROC_MMR_BIND_NODE      256
#define TRM_PROC_NODE_NAME_SIZE     40

#define TRM_RESP_STATUS_SIZE        20
#define TRM_RESP_MSG_SIZE           512

// REQUEST
#define TRM_TRACE_REQ_PKG_NAME_SIZE		20
#define TRM_TRACE_REQ_NODE_TYPE_SIZE    40
#define TRM_TRACE_REQ_NODE_NAME_SIZE	20
#define TRM_TRACE_REQ_SIZE				5
#define TRM_TRACE_REQ_KEYWORD_SIZE		40
#define TRM_TRACE_REQ_USER_ID_SIZE		20
#define TRM_TRACE_REQ_START_DATE_SIZE	20
#define TRM_TRACE_REQ_END_DATE_SIZE		20
#define TRM_TRACE_REQ_CMD_SIZE			256

// // RESPONSE
#define TRM_TRACE_RES_SIZE				5
#define TRM_TRACE_RES_PKG_NAME_SIZE	 	20
#define TRM_TRACE_RES_STATUS_SIZE		40
#define TRM_TRACE_RES_START_DATE_SIZE	20
#define TRM_TRACE_RES_END_DATE_SIZE	20
#define TRM_TRACE_RES_CMD_SIZE			256
#define TRM_TRACE_RES_SUCCESS_SIZE		5	
#define TRM_TRACE_RES_MSG_SIZE			1024

typedef struct
{
	long int oper_no;
	char pkg_name				[TRM_HIST_PKG_NAME_SIZE +1];
	char node_name				[TRM_HIST_NODE_NAME_SIZE +1];		// TAT_NODE 참조
	char user_id				[TRM_HIST_USER_ID_SIZE +1];
	int  protocol;
	int  run_mode;
	int  search_mode;
    char start_date             [TRM_HIST_START_DATE_SIZE  +1];
    char end_date               [TRM_HIST_END_DATE_SIZE +1];
	char cmd					[TRM_HIST_CMD_SIZE +1];
	char filename				[TRM_HIST_FILENAME_SiZE +1];
	int status;
} ST_TRACE_HIST;

typedef struct
{
    int  sequence;
	char pkg_name				[TRM_ROUTE_PKG_NAME_SIZE    +1];
	char cmd					[TRM_ROUTE_CMD_SIZE +1];
    int  node_no;
	char node_name				[TRM_ROUTE_NODE_NAME_SIZE +1];		// TAT_NODE 참조
	int  proc_no;
	char node_type				[TRM_ROUTE_NODE_TYPE_SIZE +1];
} ST_TRACE_ROUTE;

typedef struct
{
    int  proc_no;
    char pkg_name               [TRM_PROC_PKG_NAME_SIZE +1];
    char proc_name              [TRM_PROC_PROC_NAME_SIZE +1];
    char node_type              [TRM_PROC_NODE_TYPE_SIZE +1];
    int  svc_no;
    char exec_bin               [TRM_PROC_EXEC_BIN_SIZE +1];
    char exec_arg               [TRM_PROC_EXEC_ARG +1];
    char exec_env               [TRM_PROC_EXEC_ENV +1];
    char exec_yn                [2];
    int  start_order;
    int  stop_order;
    char use_yn                 [2];
    int  norm_stop_cd;
    int  resta_reset_sec;
    int  resta_cnt;
    char proc_type              [2];
    char node_name              [TRM_PROC_NODE_NAME_SIZE +1];       // TAT_NODE 참조
} ST_PROCESS;

typedef struct
{
    int  sequence;
	long int oper_no;
    char pkg_name               [TRM_TRACE_REQ_PKG_NAME_SIZE    +1];
    int  node_no; 
    char node_type              [TRM_TRACE_REQ_NODE_TYPE_SIZE +1];
    char trace		            [TRM_TRACE_REQ_SIZE + 1];
	int  protocol;
	int  search_mode;
	char keyword				[TRM_TRACE_REQ_KEYWORD_SIZE +1];
	int  run_mode;
	char user_id				[TRM_TRACE_REQ_USER_ID_SIZE +1];
    char start_date             [TRM_TRACE_REQ_START_DATE_SIZE  +1];		
    char end_date             	[TRM_TRACE_REQ_END_DATE_SIZE  +1];		
// WSM Request
    int  proc_no;
	char cmd					[TRM_TRACE_REQ_CMD_SIZE +1];
	bool bTraceOn;
// TRM Request
} ST_TRACE_REQUEST;

typedef struct
{
    int  sequence;
	long int oper_no;
    char pkg_name               [TRM_TRACE_RES_PKG_NAME_SIZE    +1];
    char trace		            [TRM_TRACE_RES_SIZE + 1];
	char status					[TRM_TRACE_RES_STATUS_SIZE +1];
    char start_date             [TRM_TRACE_RES_START_DATE_SIZE  +1];		
    char end_date             	[TRM_TRACE_RES_END_DATE_SIZE  +1];		
	bool success;
	char msg					[TRM_TRACE_RES_MSG_SIZE +1];
	char filename				[TRM_HIST_FILENAME_SiZE +1];
} ST_TRACE_RESPONSE;

typedef struct          
{   
    char pkg_name               [TRM_PROC_PKG_NAME_SIZE +1];
    char node_type              [TRM_PROC_NODE_TYPE_SIZE +1];
    int  na_node_no;
    int  na_proc_no;
    int  trm_node_no;
    int  trm_proc_no;
	int	 app_proc_no;
	int	 app_node_no;
}ST_COWORK_INFO;

#endif
