/* vim:ts=4:sw=4
 */
/**
 * \file	CommandMessage.hpp
 * \brief	Atom Internal Message
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef 	CCOMMANDFORMAT_HPP_
#define 	CCOMMANDFORMAT_HPP_

#include    <ctime>

#include 	"CommonCode.hpp"


// Command별 Body 정의-------------------------------

// Request 메세지에 대한 Error 발생시 공통으로 사용하는 Reponse 구조
#define CMD_REGIST					1
#define CMD_REGIST_NODE				2
#define CMD_PING					3
#define CMD_ATOM_PROC_CTL 			8
#define CMD_NODELIST                5

#define CMD_ATOM_PROC_STATUS        9
#define CMD_PRA_RECONFIG			10001
#define CMD_APP_STATUS				10002
#define CMD_APP_START				10003
#define CMD_APP_STOP				10004
#define CMD_APP_RESTART				10005
#define CMD_APP_KILL				10006
#define CMD_APP_INIT				10007
#define CMD_APP_SUSPEND				10008
#define CMD_APP_RESUME				10009
#define CMD_APP_LOGLEVEL			10010
#define CMD_APP_CTL					10100

#define CMD_ALM_EVENT				20000
#define CMD_ALM_TCA_EVENT			(CMD_ALM_EVENT+1)   // 20001
#define CMD_STATUS_NODE_EVENT		(CMD_ALM_EVENT+2)   // 20002
#define CMD_STATUS_PROC_EVENT 		(CMD_ALM_EVENT+3)   // 20003		 
#define CMD_STATUS_CONNECT_EVENT 	(CMD_ALM_EVENT+4)   // 20004
#define CMD_ALM_SUBSCRIBE_EVENT 	(CMD_ALM_EVENT+5)   // 20005
#define CMD_ALM_NOTIFY_EVENT 	    (CMD_ALM_EVENT+6)   // 20006
#define CMD_ALM_USER_CHANGE 		(CMD_ALM_EVENT+7)   // 20007
#define CMD_ALM_LIST_UPDATE 	    (CMD_ALM_EVENT+8)   // 20008
#define CMD_ALM_RETRY_REQ 			(CMD_ALM_EVENT+9)   // 20009
#define CMD_ALM_RETRY_RESP 			(CMD_ALM_EVENT+10)  // 20010
#define CMD_ALM_SYNC_REQ 			(CMD_ALM_EVENT+11) 	// 20011
//#define CMD_ALM_SYNC_RESP 			(CMD_ALM_EVENT+12) 	// 20012
#define CMD_ALM_TEST_REQ 			(CMD_ALM_EVENT+13) 	// 20013
#define CMD_ALM_TEST_RESP 			(CMD_ALM_EVENT+14) 	// 20014
//#define CMD_RSA_PERF_REQ 	        (CMD_ALM_EVENT+15)  // 20015
#define CMD_RSA_PERF_REPORT         (CMD_ALM_EVENT+16)  // 20016
#define CMD_RSA_TRAP_REPORT         (CMD_ALM_EVENT+17)  // 20017
#define CMD_STATUS_PING_EVENT 	    (CMD_ALM_EVENT+18)  // 20018
#define CMD_MANAGE_DUPLICATE 		(CMD_ALM_EVENT+19)  // 20019
#define CMD_REGISTER_PROVIDER 	    (CMD_ALM_EVENT+20) 	// 20020
#define CMD_ALM_NOTIFY_MANUAL 		(CMD_ALM_EVENT+21)  // 20021

#define CMD_STS_APP                 30001				// STA <-> APP (REQ, RSP)
#define CMD_STS_APP_HIST            30002				// STA <-> APP (IND)
#define CMD_STS_RSA                 30003 				// STA <-> RSA (REQ, RSP)
#define CMD_STS_STA                 30004				// STM <-> STA (REQ, RSP)
#define CMD_STS_STA_HIST            30005				// STM <-> STA (IND)
#define CMD_STS_STA_RESOURCE        30006				// STM <-> STA (REQ, RSP)
#define CMD_STS_STA_REALTIME        30007				// STA <-> UI  (IND)
#define CMD_STS_STA_HEARTBEAT       30008				// STM <-> STA (REQ, RSP)

#define CMD_CLI_COMMAND             80001               // CLC <-> CLA (REQ, RSP)
#define CMD_CLI_REG                 80002               // CLC <-> CLA (REQ, RSP)

#define CMD_RSA_ROOT_MSG 					90001				// RSA <-> RSA_ROOT 
#define CMD_RSA_MONITORING_REPORT   		90002				// UI <-> RSA 
#define CMD_RSA_MONITORING_REPORT_STOP   	90003				// UI <-> RSA 
#define CMD_RSA_ROOT_STAT 					90004				// RSA <-> ROOT
#define CMD_RSA_SUMMARY_REPORT				90005				// UI <-> RSA
#define CMD_RSA_SUMMARY_REPORT_STOP			90006				// UI <-> RSA
#define CMD_RSA_ROOT_TRAP			        90007				// RSA <-> RSA_ROOT
#define CMD_RSA_NIC_IP_QUERY		        90008				// RSA <-> LB or APP
#define CMD_RSA_PEER_IP_QUERY		        90009				// RSA <-> LB or APP
#define CMD_RSA_ROOT_INIT			        90010				// RSA <-> RSA root

#define CMD_VNF                     70000
#define CMD_VNF_PRA_READY           (CMD_VNF+111)
#define CMD_PRA_VNF_READY           (CMD_VNF+112)
#define CMD_VNF_PRA_INSTALL         (CMD_VNF+121)
#define CMD_PRA_VNF_INSTALLED       (CMD_VNF+122)
// #define CMD_VNF_PRA_START           CMD_APP_START 
#define CMD_VNF_PRA_START           (CMD_VNF+131) 
#define CMD_PRA_VNF_STARTED         (CMD_VNF+132)
// #define CMD_VNF_PRA_STOP            CMD_APP_STOP 
#define CMD_VNF_PRA_STOP            (CMD_VNF+141) 
#define CMD_PRA_VNF_STOPPED         (CMD_VNF+142)
#define CMD_VNF_SUBSCRIBER          (CMD_VNF+200)
#define CMD_VNF_NOTIFY              (CMD_VNF+201)
#define CMD_VNF_EVENT               (CMD_VNF+300)
#define CMD_VNF_PERF                (CMD_VNF+400)
#define CMD_VNF_PERF_REQ            (CMD_VNF_PERF+1) 
#define CMD_VNF_PERF_TPS            (CMD_VNF_PERF+2)
#define CMD_VNF_PERF_CPU            (CMD_VNF_PERF+3)
#define CMD_VNF_PERF_MEM            (CMD_VNF_PERF+4)
#define CMD_VNF_PERF_DISK           (CMD_VNF_PERF+5)
#define CMD_VNF_PERF_NET            (CMD_VNF_PERF+6)

#define CMD_BATCH_START				60001

#define CMD_TRACE_ON				100001
#define CMD_TRACE_OFF				100002

struct ST_VNF_NOTI_BODY {
        char        m_date[DB_DATETIME_SIZE];
} ;

struct ST_VNF_EVENT_BODY {
    char        m_date[DB_DATETIME_SIZE];
    char        m_eventType[EVENT_TYPE_MSG_SIZE];
    char        m_severityMsg[SEVERITY_MSG_SIZE];
    char        m_probableCause[DB_ALM_PROBABLE_CAUSE_SIZE];
    char        m_additionalText[DB_ALM_ADDITIONAL_TEXT_SIZE];
    char        m_retryCnt[RETRY_ACTION_CNT_SIZE];
    char        m_coresponseAction[CORESPONSE_ACTION_SIZE];
} __attribute__ ((packed));

struct ST_VNF_PERF {
    char        m_name[PERF_NAME_SIZE];
    char        m_value[PERF_VALUE_SIZE];
    char        m_supplement[PERF_SUPPLEMENT_SIZE];
} __attribute__ ((packed));


struct ST_VNF_PERF_BATCH {
    char        m_date[DB_DATETIME_SIZE];
    ST_VNF_PERF m_arr[VNF_PERF_MAX_CNT];

} __attribute__ ((packed));

#endif // CCOMMANDFORMAT_HPP_
