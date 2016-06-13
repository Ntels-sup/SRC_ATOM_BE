#ifndef __STM_H__
#define __STM_H__

#define STM_TABLE_TYPE_STS 1
#define STM_TABLE_TYPE_HIST  2 /* record */
#define STM_TABLE_TYPE_RES 3 /* resource */
#define STM_TABLE_TYPE_ALM 4 /* alarm */

#define TCA_RULE_OVER_CRITICAL 1
#define TCA_RULE_OVER_MAJOR 2
#define TCA_RULE_OVER_MINOR 3
#define TCA_RULE_CLEARED 4
#define TCA_RULE_UNDER_CRITICAL 5
#define TCA_RULE_UNDER_MAJOR 6
#define TCA_RULE_UNDER_MINOR 7

#define DATA_TYPE_LEN 64+1
#define DB_NAME_LEN 128+1
#define DB_IP_LEN 46+1
#define DB_USER_LEN 32+1
#define DB_PASSWORD_LEN 32+1
#define TABLE_NAME_LEN 64+1
#define TABLE_TYPE_LEN 5+1
#define NODE_NAME_LEN 64+1
#define NODE_TYPE_LEN 40+1
#define COLUMN_NAME_LEN 64+1
#define PACKAGE_NAME_LEN 128+1
#define DATA_FORMAT_LEN 16+1
#define SQL_QUERY_LEN 512+1
#define TCA_NAME_LEN 128+1
#define STRING_NUMBER_LEN 11+1
#define BIG_STRING_NUMBER_LEN 20+1
#define YN_FLAG_LEN 2+1
#define SEVENTY_LEN 16+1
#define SIGN_LEN 2+1 
#define RSC_GRP_ID_LEN 20+1
#define RSC_GRP_ID_NAME_LEN 20+1
#define ENUM_VALUE_LEN 64+1

#define MAX_COLUMN_LEN 128

#define MAX_COLLECT_TIME 1200

#define HEARTBEAT_TIMEOUT_TM 3

#define ACTIVE_STATUS 1
#define INACTIVE_STATUS 2

#define STM_OK 1
#define STM_NOK 0
#define STM_CONN_TIMEOUT 100
#define STM_CONN_WAIT 101
#define STM_ERR_RESULT_RECEIVE 102
#define STM_ERR_NODE_INACT 103

#define STM_ERR 1
#if 1
#define STM_LOG(_LVL,...){\
    CGlobal *d_inst = NULL;\
    d_inst = CGlobal::GetInstance();\
    d_inst->GetLogP()->ERROR(__VA_ARGS__);\
}
#else
#define STM_LOG(_LVL,...){\
    fprintf(stderr,__VA_ARGS__);\
}
#endif

#if 1
#define DEBUG_LOG(...){\
    CGlobal *d_inst = NULL;\
    d_inst = CGlobal::GetInstance();\
    d_inst->GetLogP()->DEBUG(__VA_ARGS__);\
}
#else
#define DEBUG_LOG(...){\
    fprintf(stderr,__VA_ARGS__);\
}
#endif

#define MSG_LEN 8192

#endif
