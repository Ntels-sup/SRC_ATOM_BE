#ifndef COMMONCODE_HPP_
#define COMMONCODE_HPP_

#include <string>

// ATOM module Process NO
#define PROCID_ATOM_NM_PRA            0x7FFFFF01
#define PROCID_ATOM_NM_RSA            0x7FFFFF02
#define PROCID_ATOM_NM_ROUTE          0x7FFFFF03	// NM routing thread

#define PROCID_ATOM_NA_PRA      	  0x7FFFFF65
#define PROCID_ATOM_NA_RSA      	  0x7FFFFF66
#define PROCID_ATOM_NA_STA      	  0x7FFFFF67
#define PROCID_ATOM_NA_ALA      	  0x7FFFFF68
#define PROCID_ATOM_NA_EXA            0x7FFFFF69
#define PROCID_ATOM_NA_CLA		      0x7FFFFF6A

// DB PKG column length
#define DB_PKG_NAME_SIZE               (40+1)
#define DB_PKG_VER_SIZE                (10+1)
#define DB_PKG_TYPE_SIZE                40 
#define DB_SVC_NAME_SIZE               (40+1)

// DB PROCESS column length
#define DB_PROC_NAME_SIZE              (40+1)
#define DB_PROC_NAME_SIZE              (40+1)

// DB NODE column length
#define DB_NODE_TYPE_SIZE              (40+1)
#define DB_NODE_NAME_SIZE              (40+1)

// DB ALM Column length
#define DB_ALM_CODE_SIZE                8
#define DB_ALM_ADDITIONAL_TEXT_SIZE     128
#define DB_ALM_LOCATION_SIZE            64
#define DB_ALM_OBJECT_SIZE              32
#define DB_ALM_COMPLEMENT_SIZE          32
#define DB_ALM_VALUE_SIZE               16
#define DB_MSG_ID_SIZE                  16
#define DB_SEQ_ID_SIZE                  DB_MSG_ID_SIZE

#define DB_ALM_ALIAS_CODE_SIZE          16
#define DB_ALM_PROBABLE_CAUSE_SIZE      64
#define DB_ALM_NODE_VERSION_SIZE        32
#define DB_ALM_NODE_TYPE_SIZE           40

// DB Trap Column length
#define DB_TRAP_ENTERPRISE_OID_SIZE 	64
#define DB_TRAP_PROC_SIZE 				256
#define DB_TRAP_VERSION_SIZE 			10
#define DB_TRAP_IP_SIZE 			    45
#define DB_TRAP_AGENT_HOSTNAME_SIZE 	64
#define DB_TRAP_NAME_SIZE 			    50	
#define DB_TRAP_VALUE_OID_SIZE 			50
#define DB_TRAP_VALUE_TYPE_SIZE 		20
#define DB_TRAP_COMMUNITY_SIZE 			64
#define DB_TRAP_HOST_NAME_SIZE 			128

// DB etc column length
#define DB_PATH_NAME_SIZE               (512+1)
#define DB_FILE_NAME_SIZE               (256+1)
#define DB_YN_SIZE                      (1+1)
#define DB_DATETIME_SIZE                32  // 원래는 22Byte 표현되나, ms 단위 포함 null 포함 넉넉히.
#define DB_VALUE_SIZE 					128


struct ST_AtomAddr
{
    int             m_nNo;
    std::string      m_strName;
};

struct ST_Profile
{
    std::string     m_strPkgName;

    int             m_nNodeNo;
    std::string     m_strNodeName;
    std::string     m_strNodeType;
    std::string     m_strNodeVersion;

    int             m_nProcNo;
    std::string     m_strProcName;
};

// ALM

#define     ALRM_TCA_OVER           "90213001"
#define     ALRM_TCA_UNDER          "90213002"

#define     ALRM_APP_CONNECT        "90200102"
#define     ALRM_NODE_STATUS        "90400102"
#define     ALRM_NETWORK_FAIL       "90100101"
#define     ALRM_PROCESS_STATUS     "90200101"

// 2016.06.01 by jhchoi marked
// #define     ALRM_TOTAL_CPU_OVER     "90300105"
// #define     ALRM_MEMORY_OVER        "90300101"
// #define     ALRM_DISK_OVER          "90300102"

#define 	ALRM_DB_EXECUTE_FAIL 	"91500102"


namespace ALM
{
    enum eSEVERITY : int
    {
        eINDETERMINATE  = 0,
        eCRITICAL       = 1,
        eMAJOR          = 2,
        eMINOR          = 3,
        eWARNING        = 4,
        eCLEARED        = 5,
        eSTATUS         = 6,
        eNOTICIE        = 7,
		eTOTAL_CNT      = 8
    };

    enum eTYPE : int
    {
        eUNKNOWN         = 0,
        eALARM          = 1,
        eFAULT          = 2,
        eNOTI           = 3
    };

    enum eNodeStatus : int
    {
        eACTIVE         = 0,
        eDISCONNECT     = 1,
        eSCALE_OUT      = 2,
        eSCALE_IN       = 3,
        eREMOVED        = 4,
        eSTANDBY        = 5,
        eNS_UNKNOWN     = 6
    };

    enum eProcStatus : int
    {
        eSTART         = 0,
        eSTOP          = 1,
        eSUSPEND       = 2
    };

    enum eConnectStatus : int
    {
        eCS_CONNECT    = 0,
        eCS_DISCONNECT = 1
    };

    enum ePingStatus : int
    {
        eSUCCESS        = 0,
        eFAIL           = 1
    };

}


// For VNF
#define 	VNF_LISTEN_PORT 			9000

#define     URI_MAX_SIZE                256
#define     EVENT_TYPE_MSG_SIZE         32
#define     SEVERITY_MSG_SIZE           16
#define     RETRY_ACTION_CNT_SIZE       4
#define     CORESPONSE_ACTION_SIZE      16

#define     PERF_NAME_SIZE              16
#define     PERF_VALUE_SIZE             16
#define     PERF_SUPPLEMENT_SIZE        16

#define     VNF_PERF_MAX_CNT            16
#define     NET_RX_NAME                 "RX"
#define     NET_TX_NAME                 "TX"

#endif // COMMONCODE_HPP_
