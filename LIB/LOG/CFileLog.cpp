/*!
 * \file CFileLog.cpp
 * \brief CFileLog Class Source File
 * \details Application 의 Log 를 Level 별로 파일에 작성
 * \author 이현재 (presentlee@ntels.com)
 * \date 2016.03.29
 */

#include <arpa/inet.h>
#include "CFileLog.hpp"

//! Define Log Level and String
stLogLevel g_stLogLevel [] = {
	{ LV_CRITICAL, 	(char*)"CRITICAL" },
	{ LV_ERROR, 	(char*)"ERROR"  },
	{ LV_WARNING, 	(char*)"WARNING" },
	{ LV_INFO, 		(char*)"INFO" },
	{ LV_DEBUG, 	(char*)"DEBUG" }
};  

//! Constructor
/*!
 * \brief CFileLog 의 생성자
 * \param None
 */
CFileLog::CFileLog (void)
{
	m_nLocalDiff = 0;
	m_nLogLevel = 0;
	m_nLogCheckTime = 0;
	m_tLastLog = 0;
	m_nDupCount = 0;

	memset(&m_LOG, 0x00, sizeof(LOGINFO));
	memset(m_szTimeStr, 0x00, sizeof(m_szTimeStr));
	memset(m_szHostName, 0x00, sizeof(m_szHostName));
	memset(m_szServiceName, 0x00, sizeof(m_szServiceName));
	memset(m_szProcessName, 0x00, sizeof(m_szProcessName));
	memset(m_szPrevMsg, 0x00, sizeof(m_szPrevMsg));
	memset(m_szLogBase, 0x00, sizeof(m_szLogBase));

	GetHostname();

	CalcDiffEpochFromLocal() ;

	pthread_mutex_init(&m_tMutex, NULL);
	m_bLockEnable = false;

}

//! Constructor
/*!
 * \brief CFileLog 의 생성자
 * \param a_pnRet is Result of this Constructor
 * \param a_nThreadMode is Thread Safe Mode for to Write Log (default : 0)
 */
CFileLog::CFileLog ( int* a_pnRet, int a_nThreadMode )
{
	int ret = 0;
	m_nLocalDiff = 0;
	m_nLogLevel = 0;
	m_nLogCheckTime = 0;
	m_tLastLog = 0;
	m_nDupCount = 0;

	memset(&m_LOG, 0x00, sizeof(LOGINFO));
	memset(m_szTimeStr, 0x00, sizeof(m_szTimeStr));
	memset(m_szHostName, 0x00, sizeof(m_szHostName));
	memset(m_szServiceName, 0x00, sizeof(m_szServiceName));
	memset(m_szProcessName, 0x00, sizeof(m_szProcessName));
	memset(m_szPrevMsg, 0x00, sizeof(m_szPrevMsg));
	memset(m_szLogBase, 0x00, sizeof(m_szLogBase));

	ret = GetHostname();
	if(ret < 0)
	{
		*a_pnRet = ret;
		return ;
	}

	CalcDiffEpochFromLocal() ;

	pthread_mutex_init(&m_tMutex, NULL);

	if(a_nThreadMode)
	{
		m_bLockEnable = true;
	}
	else
	{
		m_bLockEnable = false;
	}

}

/*!
 * \brief 날자 변경을 알아내기 위해서 현재 하루의 날짜를 구함
 * \param None
 * \return None
 */
void CFileLog::CalcDiffEpochFromLocal()
{
	struct tm	tTime ;
	
	tTime.tm_year	= 1970 - 1900 ;
	tTime.tm_mon	= 1 - 1 ;
	tTime.tm_mday	= 1 ;
	tTime.tm_hour	= 0 ;
	tTime.tm_min	= 0 ;
	tTime.tm_sec	= 0 ;

	m_nLocalDiff = (int)mktime(&tTime) ;
}

//! Initialize
/*!
 * \brief Initialize CFileLog Class
 * \detilas CFile Log Class 를 초기화 한다.
 * 1. Log Header 값에 들어가는 Host Name, Service Name, Process Name 으로 이루어진 문자열 작성
 * 2. Log Level 초기화
 * \param a_pszLogBase is Base Path of Log
 * \param a_pszService is Service Name of Process
 * \param a_pszProc is Name of Process
 * \param a_nLogCheckTime is Dup Check period(sec)
 * \param a_nLogLevel is Log Level that you want to print out
 * \return 0 on Success
 */
int CFileLog::Initialize(const char *a_pszLogBase, const char *a_pszService, const char *a_pszProc, 
															int a_nLogCheckTime, int a_nLogLevel)
{
	char szBuff[DEF_BUFFER_128];
	char szPrevFileInfo[DEF_BUFFER_512];

	memset (szPrevFileInfo	, 0x00, sizeof(szPrevFileInfo));

	sprintf(m_szPrevMsg, "%s", m_szHostName);

	m_nLogCheckTime = a_nLogCheckTime;

	if(a_pszProc == NULL || strlen(a_pszProc) == 0)
	{
		return -1;
	}

	if(a_pszLogBase != NULL && strlen(a_pszLogBase) != 0)
	{
		sprintf(m_szLogBase, "%s", a_pszLogBase);

		if(a_pszService != NULL && strlen(a_pszService) != 0)
		{
			sprintf(m_szServiceName, "%s", a_pszService);
			sprintf(szBuff, ", %s", m_szServiceName);
			strcat(m_szPrevMsg, szBuff);
		}

		sprintf(m_szProcessName, "%s", a_pszProc);
		sprintf(szBuff, ", %s", m_szProcessName);
		strcat(m_szPrevMsg, szBuff);

		if (strlen(m_szLogBase) > 0) {
			strcat(szPrevFileInfo, m_szLogBase);
			strcat(szPrevFileInfo, "/");
		}
		if (strlen(m_szServiceName) > 0) {
			strcat(szPrevFileInfo, m_szServiceName);
			strcat(szPrevFileInfo, "/");
		}
		strcat(szPrevFileInfo, m_szProcessName);
		
		StartLogFile ((const char*)szPrevFileInfo );
	}
	else
	{
		if(a_pszService != NULL && strlen(a_pszService) != 0)
		{
			sprintf(m_szServiceName, "%s", a_pszService);
			sprintf(szBuff, ", %s", m_szServiceName);
			strcat(m_szPrevMsg, szBuff);
		}

		sprintf(m_szProcessName, "%s", a_pszProc);
		sprintf(szBuff, ", %s", m_szProcessName);
		strcat(m_szPrevMsg, szBuff);

		StartLogFile(NULL);
	}

	SetLogLevel ( a_nLogLevel );


	INFO( "CFileLog: Log Dup Check Period 	[%d]", m_nLogCheckTime );
	if(strlen(m_szLogBase) != 0)
	{
		INFO( "CFileLog: Log Main path   		[%s]", m_szLogBase);
		INFO( "CFileLog: Log Full Name   		[%s]", szPrevFileInfo);
	}
	else
	{
		INFO( "CFileLog: Log Main path   		[stdout]");
		INFO( "CFileLog: Log Full Name   		[stdout]");
	}


	INFO( "CFileLog: Log Level       		[%d]", m_nLogLevel );
	INFO( "CFileLog: Hostname        		[%s]", m_szHostName );

	if(strlen(m_szServiceName) != 0)
		INFO( "CFileLog: Servicename     		[%s]", m_szServiceName );

	INFO( "CFileLog: Processame      		[%s]", m_szProcessName );

	return 0;
}

//! Destructor
CFileLog::~CFileLog()
{
	CloseLog ( &m_LOG );
}

//! Close Log File
/*!
 * \brief Close Log File
 * \param pLog is Structure of Log
 * \return None
 */
void CFileLog::CloseLog(LOGINFO* pLog)
{
	if (pLog->fpLog)
	{
		fclose(pLog->fpLog);
		pLog->fpLog = NULL;
	}
}

/*!
 * \biref Start(Open) Log File 
 * \details Open to Log File
 * \param a_pszLogFile is Full Name of Log File
 * \return None
 */
void CFileLog::StartLogFile( const char* a_pszLogFile)
{
	char			szFileName[PATH_MAX];
	char*			szDir;

	time_t		tNow;
	struct tm	rt ;

	if (m_LOG.fpLog)
	{
		fclose(m_LOG.fpLog);
		m_LOG.fpLog = NULL;
	}

	time(&tNow);
	m_LOG.tDate = tNow - ((tNow - m_nLocalDiff) % ONEDAY);

	if (a_pszLogFile != NULL && strlen(a_pszLogFile) != 0)
	{

		memset ( szFileName, INULL, sizeof ( szFileName ));
  
  		sprintf ( szFileName, "%s", a_pszLogFile);
		szDir = strdup(szFileName);
		MakeDirs(dirname(szDir));
		free(szDir);


		strftime(szFileName + strlen(szFileName), sizeof(szFileName) - strlen(szFileName), ".%Y%m%d", localtime_r(&tNow, &rt));


		m_LOG.fpLog = fopen(szFileName, "at+");

		if (!m_LOG.fpLog)
			return;

#ifdef	linux
		setlinebuf(m_LOG.fpLog);
#endif

		strcpy(m_LOG.szLogName, a_pszLogFile);
	}
	else
	{
		m_LOG.fpLog = stdout;
	}


	fprintf(m_LOG.fpLog, "\n\n");
	fprintf(m_LOG.fpLog, "\n-------------------------log open at %s-------------------------\n"
						, time2str(&tNow) );
	fprintf(m_LOG.fpLog, "\n\n");
}

/*!
 * \brief Get to Log Level
 */
int CFileLog::GetLogLevel( void )
{
	return ( m_nLogLevel );
}

/*!
 * \brief Set to Log Level
 */
void CFileLog::SetLogLevel( int a_nLevel)
{
	m_nLogLevel = a_nLevel;
}

/*!
 * \brief Write Log Msg 
 * \details 특정한 형식으로 Log Msg 를 작성
 * Format : Time, Host Name, [Service Name], Process Name, Log Level, Msg
 * \param a_nLevel is Level of This Log
 * \param a_pszFmt is Print Format
 * \return void
 */
void CFileLog::LogMsg( int a_nLevel, const char* a_pszFmt, ...)
{
	va_list ap;
	time_t			tNow;
	static	char	szLogHead[DEF_BUFFER_256];
	static	char	szLogMsg[DEF_BUFFER_4096];

	if ( a_nLevel > m_nLogLevel)
		return;

	if (m_bLockEnable)
	{
		pthread_mutex_lock(&m_tMutex);
	}

	time(&tNow);
	AdjustFileName( tNow);

	sprintf(szLogHead, "%s, %s, %s, "
					, time2str(&tNow)
					, m_szPrevMsg
					, g_stLogLevel[a_nLevel].pLevelString
	);

	va_start(ap, a_pszFmt);

	vsprintf(szLogMsg, a_pszFmt, ap);
	sprintf(szLogMsg + strlen(szLogMsg), "\n");

	//현재의 Log Msg 와 이전에 씌여진 Log Msg 를 비교
	if(DiffLastLog(szLogMsg, m_szLastLogMsg) == 0)
	{
		//지정 된 시간 이내에 중복이 발생할 경우 시간 초기화
		if(((tNow - m_tLastLog) <= m_nLogCheckTime))
		{
			m_nDupCount++;	
			m_tLastLog = tNow;
			if (m_bLockEnable)
			{
				pthread_mutex_unlock(&m_tMutex);
			}
			return ;
		}
		//지정 된 시간 이후에 중복이 발생할 경우 중복이 발생하였어도 출력
		else if(m_nLogCheckTime > 0)
		{
			fprintf(m_LOG.fpLog, "%s Last Message was repeated %d times\n", szLogHead, m_nDupCount);
			m_nDupCount = 0;
		}
	}
	//이전 Log Msg 와 같지 않고, Dup Count 가 존재할 경우 이전 Msg 의 중복 결과 출력 후
	//현재 Msg 출력
	else if(m_nDupCount)
	{
		fprintf(m_LOG.fpLog, "%s Last Message was repeated %d times\n", szLogHead, m_nDupCount);
		m_nDupCount = 0;
	}

	
	fprintf(m_LOG.fpLog, "%s%s", szLogHead, szLogMsg);
#ifndef	linux
	fflush(m_LOG.fpLog);
#endif

	if (m_bLockEnable)
	{
		pthread_mutex_unlock(&m_tMutex);
	}

	sprintf(m_szLastLogMsg, "%s", szLogMsg);
	m_tLastLog = tNow;
	va_end(ap);

}

/*!
 * \brief Debug Log Msg 출력
 * \details Debug Log 의 경우 File 명과 Line 번호를 자동적으로 출력
 * \param a_nLine is Line Number of Source Code
 * \param a_pszFile is Source File Name 
 * \param a_pszFmt is Print Format
 */
void CFileLog::DebugLogMsg(int a_nLine, const char *a_pszFile, const char* a_pszFmt, ...)
{
	static	char	szLogMsg[DEF_BUFFER_4096];
	va_list ap;

	va_start(ap, a_pszFmt);
	vsprintf(szLogMsg, a_pszFmt, ap);

	LogMsg(LV_DEBUG, "[%s:%d], %s", a_pszFile, a_nLine, szLogMsg); 
	va_end(ap);
}

/*!
 * \brief Check Log Duplication
 * \details 현재의 Log 와 이전의 Log 를 비교하여 중복을 판단
 * \param a_pszOld is Last Log Msg
 * \param a_pszNew is New Log Msg
 * \return 
 *  - 0 on Same
 *  - else Diff 
 */
int CFileLog::DiffLastLog(char *a_pszOld, char *a_pszNew)
{
	return strcmp(a_pszOld, a_pszNew);
}

/*!
 * \brief Print Hex Dump
 * \details 지정된 범위의 메모리 Dump 출력
 * \param a_pszBuf is Start Address to Dump
 * \param a_nLen is Length of Memory
 * \return void
 */
void CFileLog::LogHexMsg( const char* a_pszBuf, int a_nLen)
{
	time_t			tNow;

	if (m_bLockEnable)
	{
		pthread_mutex_lock(&m_tMutex);
	}
	
	time(&tNow);
	AdjustFileName( tNow);

	if (a_nLen > 20240 ) a_nLen = 20240;

	fprintf(m_LOG.fpLog, "[%s]\n------------------------------------------------------------------\n", time2str(&tNow));
	HexDump((unsigned char*)a_pszBuf, a_nLen, m_LOG.fpLog);
	fprintf(m_LOG.fpLog, "------------------------------------------------------------------\n");
#ifndef	linux
	fflush(m_LOG.fpLog);
#endif

	if (m_bLockEnable)
	{
		pthread_mutex_unlock(&m_tMutex);
	}

}

/*!
 * \brief convert time to string
 * \param ptime is unixtime
 * \return string of time
 */
char* CFileLog::time2str(time_t* ptime)
{           
	struct tm rt ;
    strftime(m_szTimeStr, sizeof(m_szTimeStr), "%Y-%m-%d %H:%M:%S", localtime_r(ptime, &rt));

    return m_szTimeStr;
}     

int CFileLog::MakeDirs(const char* a_pszPath)
{
    char*           szDirc  = strdup(a_pszPath);
    char*           szBasec = strdup(a_pszPath);
    char*           szDir;
    char*           szBase;

    szDir = dirname(szDirc);
    szBase = basename(szBasec);
    
    if (strcmp(szBase, ".") == 0 || strcmp(szBase, "..") == 0)
        goto bye;

    if (strcmp(szDir, ".") != 0 && strcmp(szDir, "/") != 0)
        MakeDirs(szDir);

    mkdir(a_pszPath, 0777);
    
        
bye:    
    free(szDirc);
    free(szBasec);
    
    return 0;
}

/*!
 * \brief Print Hex Dump
 * \details 지정된 범위의 메모리 Dump 출력
 * \param a_pszBuf is Start Address to Dump
 * \param a_nLen is Length of Memory
 * \param a_fpWrite is File Pointer for Write Log
 * \return void
 */
void CFileLog::HexDump(unsigned char* a_pszData, int a_nSize, FILE* a_fpWrite)
{   
    int             i, j, k;

    for (i = 0; i < a_nSize; i++)
    {
        fprintf(a_fpWrite, "%02x ", a_pszData[i]);
        if (i % 16 == 15 || i == a_nSize - 1)
        {
            for (k = i % 16; k < 15; k++)
                fprintf(a_fpWrite, "   ");                                                                                                 

            fprintf(a_fpWrite, "| ");                                                                                                      
            for (j = i - (i % 16); j <= i; j++)                                                                                          
            {       
                if (a_pszData[j] >= 32 && a_pszData[j] <= 126)                                                                                   
                    fprintf(a_fpWrite, "%c", a_pszData[j]);                                                                                    
                else                                                                                                                     
                    fprintf(a_fpWrite, ".");                                                                                               
            }
                                                                                                                                         
            fprintf(a_fpWrite, "\n");
        }                                                                                                                                
    }   
        
    fprintf(a_fpWrite, "\n");                                                                                                              
}

/*!
 * \brief Get Host Name of this system
 * \return 0 on success
 */
int CFileLog::GetHostname ()
{
	memset (m_szHostName, 0x00, sizeof(m_szHostName));

	if ( gethostname ( m_szHostName, 32 ) < 0 ) 
	{
		return -1;
	}
	
	return 0;
}

/*!
 * \brief 일의 변화를 감지
 * \details 하루가 지난 경우 (자정이 지났을 경우) Log 파일을 새로 Open
 * \return None
 */
void CFileLog::AdjustFileName( time_t tNow )
{
	if( tNow - m_LOG.tDate >= ONEDAY )
		StartLogFile(m_LOG.szLogName);

}

