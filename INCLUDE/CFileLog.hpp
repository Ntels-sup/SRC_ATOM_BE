/*!
 * \file CFileLog.hpp
 * \brief CFileLog Class Header File
 * \details class of For Log Write
 * \author lee HyunJae (presentlee@ntels.com)
 * \date 2016.03.29
 */
#ifndef		CFILELOG_HPP_
#define		CFILELOG_HPP_

//Include Header Files
#include 	<stdio.h>
#include 	<unistd.h>
#include	<string.h>
#include 	<stdlib.h>
#include	<fcntl.h>
#include 	<limits.h>
#include	<sys/time.h>
#include	<time.h>
#include 	<stdarg.h>
#include 	<libgen.h>
#include	<pthread.h>
#include	<sys/stat.h>

//!Define one day sec
#define	ONEDAY		(24 * 60 * 60)

#ifndef		INULL
#define		INULL				'\0'
#endif
		
//! Define Log Level
enum eLogLevel {
	LV_CRITICAL = 0,
	LV_ERROR,
	LV_WARNING,
	LV_INFO,
	LV_DEBUG
};

//! Define 128 byte Buffer Size
#define DEF_BUFFER_128  128
//! Define 256 byte Buffer Size
#define DEF_BUFFER_256  256
//! Define 512 byte Buffer Size
#define DEF_BUFFER_512  512
//! Define 1024 byte Buffer Size
#define DEF_BUFFER_1024 1024
//! Define 2048 byte Buffer Size
#define DEF_BUFFER_2048 2048
//! Define 4096 byte Buffer Size
#define DEF_BUFFER_4096 4096

//! Define Macro of DEBUG Level LOG
#define DEBUG(fmt, args...)		DebugLogMsg(__LINE__, __FILE__, fmt,  ## args) 
//! Define Macro of DEBUG Level INFO
#define INFO(fmt, args...)		LogMsg(LV_INFO, fmt, ## args) 
//! Define Macro of DEBUG Level WARNING
#define WARNING(fmt, args...)	LogMsg(LV_WARNING, fmt,  ## args) 
//! Define Macro of DEBUG Level ERROR
#define ERROR(fmt, args...)		LogMsg(LV_ERROR, fmt,  ## args) 
//! Define Macro of DEBUG Level CRITICAL
#define CRITICAL(fmt, args...)	LogMsg(LV_CRITICAL, fmt,  ## args) 

//! Define Structure of Log Level
/*!
 * \struct _stLogLevel
 * \brief Structure for Log Level 
 */
typedef struct _stLogLevel {
    int             nLevelNo;
    char            *pLevelString;
} stLogLevel;


/*!
 * \class CFileLog
 * \brief CFileLog Class for Write Log
 */
class CFileLog{


	private:
		//! Define Structure of Log Info
		/*!
 		 * \struct LOGINFO
 		 * \brief Infomation of Write Log File
 		 */
		struct LOGINFO {
			FILE*		fpLog;
			char		szLogName[PATH_MAX];
			time_t      tDate;
		};

		//! Log Info Structure
		LOGINFO	m_LOG;
		//! For Log File Rewrite per oneday
		int		m_nLocalDiff;
		//! Log Level
		int		m_nLogLevel;
		//! String of Time
		char    m_szTimeStr[PATH_MAX];
		//! Log Base Path
		char	m_szLogBase[DEF_BUFFER_256];
		//! Host Name of This System
		char 	m_szHostName	[DEF_BUFFER_128];
		//! Service Name of Caller
		char 	m_szServiceName	[DEF_BUFFER_128];
		//! Process Name of Caller
		char 	m_szProcessName	[DEF_BUFFER_128];
		//! Log Header (HostName + ServiceName + ProcessName)
		char 	m_szPrevMsg		[DEF_BUFFER_512];
		//! Last Log Msg
		char 	m_szLastLogMsg	[DEF_BUFFER_4096];
		//! Last Log Write time
		time_t	m_tLastLog;
		//! Log Duplication Check Time
		int		m_nLogCheckTime;
		//! Count of Log Duplication
		int		m_nDupCount;
		//! Thread Safe Flag
		bool m_bLockEnable;
		//! Thread Mutex Lock
		pthread_mutex_t	m_tMutex;
		//! Close Log
		void	CloseLog(LOGINFO* pLog);
		//! Compute One Day
		void	CalcDiffEpochFromLocal() ;
		//! Get String of Time
		char*	time2str( time_t* pT );
		//! Open Log Files
		void	StartLogFile( const char* a_pszLogFile);
		//! Make Directory if there is no directory
		int		MakeDirs(const char* a_pszPath);
		//! Print Hex Dump
		void	HexDump(unsigned char* a_pszData, int a_nSize, FILE* a_fpWrite);
		//! Get Host Name
		int 	GetHostname ();
		//! Check Log File Date
		void	AdjustFileName( time_t tNow );	
		//! Diff Log Msg
		int		DiffLastLog(char *a_pszOld, char *a_pszNew);

	public:
		//! Define Structure of Log Level
		/*!
 	 	 * \struct _stLogLevel
 		 * \brief Structure for Log Level 
 		 */
		struct stLogLevel {
    		int             nLevelNo;
    		char            *pLevelString;
		};

		//! Base Constructor
		CFileLog ();
		//! Constructor
		CFileLog ( int* a_pnRet, int a_nThreadMode = 0 );
		//! Destructor
		~CFileLog();

		//! Init Log Class
		int		Initialize(const char *a_pszLogBase, const char *a_pszService, const char *a_pszProc, 
													int a_nLogCheckTime, int a_nLogLevel = LV_INFO);
		//! Get to Log Level
		int		GetLogLevel( void );
		//! Set to Log Level
		void	SetLogLevel( int nLevel);
		//! Set to thread loging lock
		void	SetThreadLock(void) { m_bLockEnable = true; }
		//! Write Hex Dump
		void	LogHexMsg( const char* a_pszBuf, int a_nLen);
		//! Write Log Msg
		void	LogMsg( int a_nLevel, const char* a_pszFmt, ...);
		//! Write Debug Log Msg
		void	DebugLogMsg(int a_nLine, const char *a_pszFile, const char *a_pszFmt, ...);

};

#endif //CFILELOG_HPP_
