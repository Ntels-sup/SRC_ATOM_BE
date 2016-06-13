#ifndef __BJ_DefinesHpp__
#define __BJ_DefinesHpp__

#include "CFileLog.hpp"
#include "CConfigBJM.hpp"
#include "MariaDB.hpp"
#include "FetchMaria.hpp"
#include "CommonCode.hpp"
#include "DB.hpp"
#include <string>
#include <vector>


#define BJM_ERR 0

#define BJM_LOG(_LVL,...){\
fprintf(stderr,__VA_ARGS__);\
}

#define SPACE_REMOVE_BUF_SIZE 	256

#define BJM_ACTION_START		"START"
#define BJM_ACTION_STOP			"STOP"
#define BJM_ACTION_INIT			"INIT"
#define BJM_ACTION_LOGLEVEL		"LOGLEVEL"

#define BJM_NOK 					0
#define BJM_OK 						1
#define BJM_RECV_START				2
#define BJM_RECV_RECONFIG			3
#define BJM_RECV_WSM_START			4
#define BJM_RECV_REGIST				5
#define BJM_RECV_ERROR				6
#define BJM_RECV_PROC_CTRL			7
#define BJM_RECV_PROC_START			8
#define BJM_RECV_PROC_STOP			9
#define BJM_RECV_BACKUP				11
#define BJM_RECV_RESTORE			12
#define BJM_RECV_SCALE_IN			13
#define BJM_RECV_SCALE_OUT			14

#define APPLY_DATE_SIZE            	14

#define BJM_MAX_BATCH_GROUP        	256
#define BJM_MAX_BATCH_JOB          	1024
#define BJM_MAX_BATCH_JOB_NEXT     	1024
#define BJM_MAX_BATCH_CMD          	1024
#define BJM_MAX_BATCH_FLOW         	256
#define BJM_MAX_BATCH_HIST         	1024
#define BJM_MAX_PROCESS		       	512
#define BJM_MAX_REQUEST		       	128
#define BJM_MAX_RESPONSE	       	128

//20160328 kjjy7 TAT_BATCH_GROUP
#define BJM_GROUP_GROUP_NAME_SIZE     40
#define BJM_GROUP_PKG_NAME_SIZE		  40
#define BJM_GROUP_CREATE_DATE_SIZE    20
#define BJM_GROUP_START_DATE_SIZE     20
#define BJM_GROUP_SCHEDULE_CYCLE_TYPE_SIZE  2
#define BJM_GROUP_EXPIRE_DATE_SIZE    20
#define BJM_GROUP_USER_ID_SIZE        20
#define BJM_GROUP_DESC_SIZE           256


//20160328 kjjy7 TAT_BATCH_JOB
#define BJM_JOB_GROUP_NAME_SIZE     40
#define BJM_JOB_JOB_NAME_SIZE       40
#define BJM_JOB_PROC_NAME_SIZE	    40
#define BJM_JOB_PKG_NAME_SIZE	    40
#define BJM_JOB_NODE_TYPE_SIZE	    40
#define BJM_JOB_DESC_SIZE           256

// 20160331 kjjy7 TAT_BATCH_FLOW
#define BJM_FLOW_JOB_NAME_SIZE      40

//20160328 kjjy7 TAT_BATCH_HIST
#define BJM_HIST_GROUP_NAME_SIZE    40
#define BJM_HIST_JOB_NAME_SIZE      40
#define BJM_HIST_PRC_DATE_SIZE	    20
#define BJM_HIST_PKG_NAME_SIZE	    40
#define BJM_HIST_END_DATE_SIZE      20
#define BJM_HIST_PROC_NAME_SIZE     40
#define BJM_HIST_NODE_NAME_SIZE     40
#define BJM_HIST_STATUS_SIZE        20
#define BJM_HIST_MSG_SIZE           256

// TAT_PROCESS
#define BJM_PROC_PKG_NAME_SIZE		40
#define BJM_PROC_PROC_NAME_SIZE		40
#define BJM_PROC_NODE_TYPE_SIZE		40
#define BJM_PROC_EXEC_BIN_SIZE		512
#define BJM_PROC_EXEC_ARG			256
#define BJM_PROC_EXEC_ENV			256
#define BJM_PROC_CPU_BIND_CORE		512
#define BJM_PROC_MMR_BIND_NODE		256
#define BJM_PROC_NODE_NAME_SIZE		40

#define BJM_RESP_STATUS_SIZE 		20
#define BJM_RESP_MSG_SIZE 			512

#define NODE_SCALE_IN				"0000000007"
#define NODE_SCALE_OUT				"0000000006"
#define ATOM_PROC_CTL				"0000000008"
#define BJM_REGIST					"0000000001"
#define BJM_BATCH_START				"0000060001"
#define BJM_BATCH_RECONFIG			"0000060002"
#define BJM_BATCH_START_WSM			"0000060003"
#define BJM_BACKUP					"0000110001"
#define BJM_RESTORE					"0000110002"

typedef struct
{
	int  flow_no;
	int  exit_cd;				
	char job_name				[BJM_FLOW_JOB_NAME_SIZE +1];
	char next_job_name			[BJM_FLOW_JOB_NAME_SIZE +1];
} ST_BatchFlow;

typedef struct
{
    char job_name               [BJM_JOB_JOB_NAME_SIZE +1];
    char group_name             [BJM_JOB_GROUP_NAME_SIZE +1];
	char pkg_name				[BJM_JOB_PKG_NAME_SIZE +1];
    char rootjob_yn				[2];
    int  node_no;
	char node_type				[BJM_JOB_NODE_TYPE_SIZE +1];
	int  proc_no;
    char fail_cont_yn			[2];
    char run_yn					[2];
    char description            [BJM_JOB_DESC_SIZE      +1];
	char nextExecDate	 		[APPLY_DATE_SIZE        +1];
} ST_BatchJob;

typedef struct
{
    char group_name             [BJM_GROUP_GROUP_NAME_SIZE  +1];
	char pkg_name				[BJM_GROUP_PKG_NAME_SIZE    +1];
    char create_date            [BJM_GROUP_CREATE_DATE_SIZE +1];
    char start_date             [BJM_GROUP_START_DATE_SIZE  +1];
    char schedule_cycle_type    [BJM_GROUP_SCHEDULE_CYCLE_TYPE_SIZE +1];
    int  nschedule_cycle;
    char use_yn					[2];
    char expire_date            [BJM_GROUP_EXPIRE_DATE_SIZE +1];
	char user_id				[BJM_GROUP_USER_ID_SIZE +1];
    char description            [BJM_GROUP_DESC_SIZE +1];
} ST_BatchGroup;


typedef struct
{
    char group_name             [BJM_HIST_GROUP_NAME_SIZE +1];
    char job_name               [BJM_HIST_JOB_NAME_SIZE +1];
    char prc_date               [BJM_HIST_PRC_DATE_SIZE +1];
    char pkg_name				[BJM_HIST_PKG_NAME_SIZE +1];
    char end_date               [BJM_HIST_END_DATE_SIZE +1];
    char proc_name              [BJM_HIST_PROC_NAME_SIZE +1];
    char node_name              [BJM_HIST_NODE_NAME_SIZE +1];
    int  exit_cd;
    char success_yn				[2];
} ST_BATCHHIST;

typedef struct
{
    int  proc_no;
	char pkg_name				[BJM_PROC_PKG_NAME_SIZE +1];
    char proc_name              [BJM_PROC_PROC_NAME_SIZE +1];
	char node_type				[BJM_PROC_NODE_TYPE_SIZE +1];
	int  svc_no;
    char exec_bin               [BJM_PROC_EXEC_BIN_SIZE +1];
    char exec_arg               [BJM_PROC_EXEC_ARG +1];
    char exec_env               [BJM_PROC_EXEC_ENV +1];
    char exec_yn				[2];
    int  start_order;
    int  stop_order;
    char use_yn					[2];
    int  norm_stop_cd;
    int  resta_reset_sec;
	int	 resta_cnt;
    char proc_type				[2];
	char node_name				[BJM_PROC_NODE_NAME_SIZE +1];		// TAT_NODE 참조
} ST_PROCESS;

// Send Request
typedef struct
{
    int  sequence;	
    char group_name             [BJM_HIST_GROUP_NAME_SIZE +1];
    char job_name               [BJM_HIST_JOB_NAME_SIZE +1];
	int  proc_no;
    char prc_date               [BJM_HIST_PRC_DATE_SIZE +1];
    char pkg_name				[BJM_HIST_PKG_NAME_SIZE +1];
    char proc_name              [BJM_HIST_PROC_NAME_SIZE +1];
    char exec_bin               [BJM_PROC_EXEC_BIN_SIZE +1];
    char exec_arg               [BJM_PROC_EXEC_ARG +1];
    char exec_env               [BJM_PROC_EXEC_ENV +1];
	char wsm_yn					[2];
	int  id_snode;												// Source id
	int  id_sproc;
	int  id_dnode;												// destination id
	int  id_dproc;
}ST_BatchRequest;

// recv Response
typedef struct
{
    int  sequence;
	int  id_snode;												// Source id
	int  id_sproc;
	int  id_dnode;												// destination id
	int  id_dproc;
    char group_name             [BJM_HIST_GROUP_NAME_SIZE +1];
    char job_name               [BJM_HIST_JOB_NAME_SIZE +1];
    char prc_date               [BJM_HIST_PRC_DATE_SIZE +1];
    char end_date               [BJM_HIST_END_DATE_SIZE +1];
	char wsm_yn					[2];
    char status	                [BJM_RESP_STATUS_SIZE +1];
	char msg					[BJM_RESP_MSG_SIZE +1];
    int  exit_cd;												// code
}ST_BatchResponse;

typedef struct
{
    char pkg_name				[BJM_PROC_PKG_NAME_SIZE +1];
	char node_type				[BJM_PROC_NODE_TYPE_SIZE +1];
    int  node_no;
    int  nm_proc_no;
    int  bjm_proc_no;
    int  wsm_proc_no;
}ST_COWORK_INFO;

typedef struct
{
    int  sequence;
	string exec_bin;
	vector<string> vCategory;
	vector<string>::iterator vCategory_it;
}ST_BACKUP_REQUEST;

typedef struct
{
    int  sequence;
	string category;
	string success;
}ST_BACKUP_RESPONSE;

typedef struct
{
    int  sequence;
	string restore_file;
	string restore_path;
}ST_RESTORE_REQUEST;

typedef struct
{
    char group_name             [BJM_HIST_GROUP_NAME_SIZE +1];
	char job_name				[BJM_FLOW_JOB_NAME_SIZE +1];
    char next_job_name          [BJM_FLOW_JOB_NAME_SIZE +1];
	char prev_job_name			[BJM_FLOW_JOB_NAME_SIZE +1];	//for duplicate check
    int  Count;
	int  CurCount;
    int  exit_cd;
}ST_MULTI_JOB_FLOW;
#endif
