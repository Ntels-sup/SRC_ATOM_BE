#ifndef __DEF_RSA_GLOBAL__
#define __DEF_RSA_GLOBAL__ 1

//Define Memory Buffer Size
#define DEF_MEM_BUF_16 		16
#define DEF_MEM_BUF_32 		32
#define DEF_MEM_BUF_64 		64
#define DEF_MEM_BUF_128 	128
#define DEF_MEM_BUF_256 	256
#define DEF_MEM_BUF_512 	512
#define DEF_MEM_BUF_1024 	1024
#define DEF_MEM_BUF_2048 	2048
#define DEF_MEM_BUF_4096 	4096
#define DEF_MEM_BUF_8192 	8192
#define DEF_MAX_TOP_COUNT 5

#define DEF_MAX_PRINT_TOP_COUNT 5
#define DEF_PRINT_TOP_PERIOD	60


#define DEF_EXEC_DEFAULT 		0x00000000
#define DEF_EXEC_SET_FULL 		0x000000FF
#define DEF_EXEC_UNSET_FULL 	0xFFFFFF00
#define DEF_EXEC_SET_SUMMARY 	0x0000FF00
#define DEF_EXEC_UNSET_SUMMARY 	0xFFFF00FF

#define DEF_ALM_CODE_CPU_OVER           "90300101"
#define DEF_ALM_CODE_MEMORY_OVER        "90300201"
#define DEF_ALM_CODE_CPU_TEMP_OVER      "90300301"
#define DEF_ALM_CODE_TOTAL_CPU_OVER     "90300401"
#define DEF_ALM_CODE_DISK_OVER          "90300501"
#define DEF_ALM_CODE_TABLESPACE_OVER    "90300601"
#define DEF_ALM_CODE_FAN_HALT           "90300701"
#define DEF_ALM_CODE_DIMM_HALT          "90300801"
#define DEF_ALM_CODE_POWER_HALT         "90300901"
#define DEF_ALM_CODE_PROCESSOR_HALT     "90301001"
#define DEF_ALM_CODE_ETHERNET_DOWN      "90301101"
#define DEF_ALM_CODE_BOND_DOWN          "90301201"
#define DEF_ALM_CODE_PHY_DRIVE_HALT     "90301301"
#define DEF_ALM_CODE_EX_DISK_HALT       "90301401"
#define DEF_ALM_CODE_EXTERNAL_TRAP      "90301501"
#define DEF_ALM_CODE_SYS_LOG            "90301601"
#define DEF_ALM_CODE_INTERNAL_DISK_HALT "90301701"
#define DEF_ALM_CODE_QUEUE_OVER         "90301801"


#define DEF_GRP_ID_CPU			"RSC_GRP_01"
#define DEF_GRP_ID_MEMORY		"RSC_GRP_02"
#define DEF_GRP_ID_TEMP			"RSC_GRP_03"
#define DEF_GRP_ID_TOT_CPU		"RSC_GRP_04"
#define DEF_GRP_ID_DISK			"RSC_GRP_05"
#define DEF_GRP_ID_TABLESPACE	"RSC_GRP_06"
#define DEF_GRP_ID_FAN			"RSC_GRP_07"
#define DEF_GRP_ID_DIMM			"RSC_GRP_08"
#define DEF_GRP_ID_POWER		"RSC_GRP_09"
#define DEF_GRP_ID_PROCESSOR	"RSC_GRP_10"
#define DEF_GRP_ID_NIC			"RSC_GRP_11"
#define DEF_GRP_ID_BOND			"RSC_GRP_12"
#define DEF_GRP_ID_PHY_DRIVE	"RSC_GRP_13"
#define DEF_GRP_ID_EX_DISK		"RSC_GRP_14"
#define DEF_GRP_ID_INVEN		"RSC_GRP_15"
#define DEF_GRP_ID_QUEUE		"RSC_GRP_16"
#define DEF_GRP_ID_PING			"RSC_GRP_17"
#define DEF_GRP_ID_SNMP			"RSC_GRP_18"
#define DEF_GRP_ID_SYSLOG		"RSC_GRP_19"


using namespace std;


enum
{
	IDX_DB_CONN_INFO_HOST = 0,
	IDX_DB_CONN_INFO_PORT,
	IDX_DB_CONN_INFO_USER,
	IDX_DB_CONN_INFO_PASS,
	IDX_DB_CONN_INFO_DB,
	MAX_DB_CONN_INFO_IDX
};

typedef struct _st_monitor_dst
{
	int nNode;
	int nProc;
	int nPeriod;
	int nSeq;
	time_t tTimeOut;
	time_t tNextCheck;
}DST_INFO;

typedef struct _st_resource_attr
{
	char szID 	[DEF_MEM_BUF_64]; 
	char szName [DEF_MEM_BUF_64];
	char szArgs [DEF_MEM_BUF_64];
	void *pData;

}RESOURCE_ATTR;

typedef struct _st_resource
{
	char szGroupName 	[DEF_MEM_BUF_64];
	char szGroupID 		[DEF_MEM_BUF_64];
	char szPlugName 	[DEF_MEM_BUF_64];
	int  nPeriod;
	time_t  tNextCheck;
	bool bRoot;
	bool bVNFM;
	bool bStat;
	uint32_t unExec;
	string strFullJson;
	string strSummaryJson;
	string strStatJson;
	string strRootTrapJson;
	void *pclsRSA;
	void *pHandle;
	map<string, RESOURCE_ATTR *> mapRsc;
}RESOURCE;

#endif
