/*!
 * \file CAtomQInit.hpp
 * \brief MRT Main Class Header File
 */
#ifndef _MRT_MAIN_H_
#define _MRT_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pwd.h>
#include <unistd.h>

#include "CConfig.hpp"

//! Define Memory Buffer 128 byte
#define DEF_MEM_BUF_128		128
//! Define Memory Buffer 256 byte
#define DEF_MEM_BUF_256		256

//! Define Default Ring Size, (262,144)
/*!
 * Must be Power of 2
 */
#define DEF_RING_SIZE	0x40000  

//! Define Memory Pool Name
/*!
 * Memory Pool Name For to Use in Application
 */
#define DEF_BASE_MEMORY_POOL_NAME	"BASE_MEMORY_POOL"  

//! Define Command Memory Pool Name
/*!
 * Memory Pool Name For to Use in Application
 */
#define DEF_CMD_MEMORY_POOL_NAME	"COMMAND_MEMORY_POOL"  

//! Define Memory Buffer Elements Count
/*!
 * To Use in Memory Pool Create Function
 * Power of 2 - 1
 */
#define DEF_MBUF_COUNT	( 1 << 18 ) - 1  

//! Define Exit Code on Error
#define DEF_EXIT_CODE_ON_ERR 9

/*!
 * \class CAtomQInit
 * \brief CMRT Main Class
 */
class CAtomQInit
{
	public:
		//! Constructor.
		CAtomQInit(char *a_strLogPath = NULL);
		//! Destructor.
		~CAtomQInit();

		//! Initialize
		int Initialize(char *a_szCfgFile);

	private:
		//! Log Path 
		char m_strLogPath[DEF_MEM_BUF_256];
		//! Log Path Pointer
		char *m_pLogPath;
		//! Memory Pool For Data
		struct rte_mempool *m_pstDataMemPool;
		//! Memory Pool For Command
		struct rte_mempool *m_pstCmdMemPool;
		//! Get UID By User Name for change Root UID -> ATOMQ Owner UID
		int NameToUID(const char *a_szName);
		
};

#endif

