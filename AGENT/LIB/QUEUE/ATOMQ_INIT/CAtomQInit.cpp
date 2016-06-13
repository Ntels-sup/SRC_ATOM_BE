/*!
 * \file CAtomQInit.cpp
 * \brief CAtomQInit Main Class Source File
 */

// Include Standard Header
#include "CAtomQInit.hpp"
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// Include Definition For DPDK Envirionment Variables and Functions
#include <rte_config.h>
#include <rte_memory.h>
#include <rte_memzone.h>
#include <rte_launch.h>
#include <rte_tailq.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_string_fns.h>
#include <rte_malloc.h>
#include <rte_ring.h>
#include <rte_mbuf.h>
#include <rte_errno.h>

/*! 
 * \class CAtomQInit
 * \brief CMRT Main Class
 */


//! Constructor
/*!
 * \brief Constructor For CAtomQInit Class
 * \details Init Variables 
 * \param a_strLogPath is Log file path if NULL, default stdout
 */
CAtomQInit::CAtomQInit(char *a_strLogPath)
{
	m_pLogPath = NULL;

	//Copy Log path
	memset(m_strLogPath, 0x00, sizeof(m_strLogPath));

	//a_strLogPath Default is NULL
	if(a_strLogPath)
	{
		snprintf(m_strLogPath, sizeof(m_strLogPath), "%s",  a_strLogPath);
		m_pLogPath = m_strLogPath;
		
	}

	m_pstDataMemPool = NULL;
	m_pstCmdMemPool = NULL;

}

//! Destructor
/*!
 * \brief Destructor For CAtomQInit Class
 * \details Init Variables and Delete Memory
 */
CAtomQInit::~CAtomQInit()
{


}


//! Initialize
/*!
 * \brief Init Variables and DPDK Library
 * \param a_pszName is Name of IPC File
 * \param a_nSize is Size of each IPC File
 * \param a_nCount is Count of IPC File
 * \return Succ 0, Fail -1
 */
int CAtomQInit::Initialize(char *a_szCfgFile)
{
	int ret = 0;
	struct rte_ipc_config stIpcConfig;
	uint32_t unMbufCount = 0;
	uint64_t unSize = 0;

	CConfig *pclsConfig = new CConfig();
	if(pclsConfig->Initialize(a_szCfgFile) < 0)
	{
		RTE_LOG (ERR, EAL, "Cannot Init CConfig\n");
		return -1;
	}

	ret = NameToUID(pclsConfig->GetConfigValue("QUEUE", "OWNER"));
	if( ret < 0 )
	{
		printf("Failed to Change UID, User (%s)\n", pclsConfig->GetConfigValue("QUEUE", "OWNER") );
		return -1;
	}

	memset(&stIpcConfig, 0x00, sizeof(struct rte_ipc_config));

	stIpcConfig.ipc_count = atoi(pclsConfig->GetConfigValue("QUEUE","IPC_COUNT"));

	unSize = atoi(pclsConfig->GetConfigValue("QUEUE", "IPC_SIZE"));
	switch(unSize)
	{
		case 1 :
			unSize = RTE_PGSIZE_1G;
			break;
		case 2 :
			unSize = RTE_PGSIZE_2G;
			break;
		case 4 :
			unSize = RTE_PGSIZE_4G;
			break;
		case 8 :
			unSize = RTE_PGSIZE_8G;
			break;
		default :
			break;
	}
	stIpcConfig.ipc_size = unSize;
	sprintf(stIpcConfig.ipc_name, "%s", pclsConfig->GetConfigValue("QUEUE", "IPC_NAME"));

	
	//Set Process Type (primary)
	rte_eal_set_proc_type(RTE_PROC_PRIMARY);
	//Init to DPDK Library	
	ret = rte_eal_init(m_pLogPath, &stIpcConfig);

	//Failed
	if(ret < 0)
	{
		RTE_LOG (ERR, EAL, "Cannot init MRT\n");
		return -1;
	}

	RTE_LOG( INFO, EAL, "IPC Count %u\n", stIpcConfig.ipc_count);
	RTE_LOG( INFO, EAL, "IPC Size %lu\n", stIpcConfig.ipc_size);
	RTE_LOG( INFO, EAL, "IPC Name %s\n", stIpcConfig.ipc_name);

	
	unMbufCount = strtoul( pclsConfig->GetConfigValue("QUEUE", "MBUF_COUNT"), NULL, 16 );
	/* [Parameters]
	 * name, elem Count, elem Size, cache Size, private Data Size,
	 * Memory Pool Init Func, Memory Pool Init func Argument
	 * element Init Func, element Init Func Argument
	 * Memory Socket ID, Flags
	 */
	m_pstDataMemPool = rte_mempool_create(DEF_BASE_MEMORY_POOL_NAME, unMbufCount,
										RTE_MBUF_SIZE, RTE_MBUF_CACHE_SIZE,
										sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init,
										NULL, rte_pktmbuf_init, NULL, SOCKET_ID_ANY, 0); 
	if(m_pstDataMemPool == NULL)
	{
		RTE_LOG (ERR, MEMPOOL, "Cannot Create Memory Pool\n");
		return -1;
	}

	m_pstCmdMemPool = rte_mempool_create(DEF_CMD_MEMORY_POOL_NAME, DEF_MBUF_COUNT,
										RTE_MBUF_SIZE, RTE_MBUF_CACHE_SIZE,
										sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init,
										NULL, rte_pktmbuf_init, NULL, SOCKET_ID_ANY, 0); 
	if(m_pstCmdMemPool == NULL)
	{
		RTE_LOG (ERR, MEMPOOL, "Cannot Create Memory Pool\n");
		return -1;
	}

	struct rte_mbuf *tmp = (struct rte_mbuf*)rte_pktmbuf_alloc(m_pstDataMemPool);
	RTE_LOG( INFO, EAL, "base mbuf data_len %u, buf_len %u, buf_addr %p\n", tmp->data_len, tmp->buf_len, tmp->buf_addr);
	RTE_LOG( INFO, EAL, "mbuf data Buffer %p\n", rte_pktmbuf_mtod(tmp, void*));
	RTE_LOG( INFO, EAL, "DEF_MBUF_COUNT %d\n", unMbufCount);
	RTE_LOG( INFO, EAL, "DEF_MBUF_SIZE %lu\n", RTE_MBUF_SIZE);
	RTE_LOG( INFO, EAL, "DEF_MBUF_CACHE_SIZE %d\n", RTE_MBUF_CACHE_SIZE);
	RTE_LOG( INFO, EAL, "DEF_PRIVATE_ %lu\n", sizeof(struct rte_pktmbuf_pool_private));

	return 0;
}

int CAtomQInit::NameToUID(const char *a_szName)
{
	if( NULL == a_szName )
	{
		printf( "Owner is NULL\n");
		return -1;
	}

	long const lBufLen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if(lBufLen == -1)
	{
		printf( "Failed to Get BufLen\n");
		return -1;
	}

	char szBuffer[lBufLen];

	struct passwd stPwBuf, *pstPwBuf;

	if( 0 != getpwnam_r(a_szName, &stPwBuf, szBuffer, lBufLen, &pstPwBuf) 
			|| !pstPwBuf)
	{
		printf( "Failed to getpwnam_r Func\n");
		return -1;
	}

	if( setgid(pstPwBuf->pw_gid) < 0 )
	{
		printf("Failed to Change GID, errno : %d\n", errno);
		return -1;
	}

	if( setuid(pstPwBuf->pw_uid) < 0 )
	{
		printf("Failed to Change UID, errno :  %d\n", errno);
		return -1;
	}

	return 0;
}

void print_help_msg()
{
	printf("\n\n\n");
	printf("[help] ================================================================\n");
	printf("   -f [Log Path   ] : Log File Path (default:stdout)\n");
	printf("  ex)  ./LQ_INIT -f ./test.log\n");
	printf("=======================================================================\n");
	printf("\n\n\n");
}

//! Main Function
/*!
 * \brief main Function For MRT Process
 * \param argc is Arguments count
 * \param args is String Array of Arguments
 * \return Succ 0, Fail -1
 */
int main(int argc, char *args[])
{
	CAtomQInit *m_pclsCMRT = NULL;

	int param_opt = 0;
	char *pszLog = NULL;
	char *pszConfig = NULL;
	char *pszHome = NULL;

	while( -1 != (param_opt = getopt(argc, args, "hf:c:d:")))
	{
		switch(param_opt)
		{
			case 'h' :
				print_help_msg();
				return DEF_EXIT_CODE_ON_ERR;
			case 'f' :
				pszLog = optarg;
				printf("Log File Path : %s\n", pszLog);
				break;
			case 'c' :
				pszConfig = optarg;
				break;
			case 'd' :
				pszHome = optarg;
				break;
			default :
				break;
		}
	}

	if(NULL == pszConfig)
	{
		printf("Please Insert Config File Path by '-c [config File Path]' option\n");
		return DEF_EXIT_CODE_ON_ERR;
	}

	if(NULL == pszHome)
	{
		printf("Please Insert Atom Home Path by '-d [Atom Home Path]' option\n");
		return DEF_EXIT_CODE_ON_ERR;
	}

	if(setenv("HOME", pszHome, 1) < 0)
	{
		printf("Failed to setenv ATOM_HOME = %s\n", pszHome);
		return DEF_EXIT_CODE_ON_ERR;
	}

	m_pclsCMRT = new CAtomQInit(pszLog);

	//Failed
	if(m_pclsCMRT == NULL)
		return DEF_EXIT_CODE_ON_ERR;


	if(m_pclsCMRT->Initialize(pszConfig) < 0)
		return DEF_EXIT_CODE_ON_ERR;

	delete m_pclsCMRT;

	return 0;
}

