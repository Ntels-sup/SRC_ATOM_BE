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
int CFileLog::Initialize(const char *a_pszLogBase, const char *a_pszService, 
						const char *a_pszProc, int a_nLogCheckTime, int a_nLogLevel)
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


	INFO( "CFileLog: Log Dup Check Period   [%d]", m_nLogCheckTime );
	if(strlen(m_szLogBase) != 0)
	{
		INFO( "CFileLog: Log Main path          [%s]", m_szLogBase);
		INFO( "CFileLog: Log Full Name          [%s]", szPrevFileInfo);
	}
	else
	{
		INFO( "CFileLog: Log Main path          [stdout]");
		INFO( "CFileLog: Log Full Name          [stdout]");
	}


	INFO( "CFileLog: Log Level              [%d]", m_nLogLevel );
	INFO( "CFileLog: Hostname               [%s]", m_szHostName );

	if(strlen(m_szServiceName) != 0)
		INFO( "CFileLog: Service Name           [%s]", m_szServiceName );

	INFO( "CFileLog: Process Name           [%s]", m_szProcessName );

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
		while(aio_error(&m_LOG.aio_wb) == EINPROGRESS);
		fsync(pLog->fpLog);
		close(pLog->fpLog);
		pLog->fpLog = 0;
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

	struct stat stStat;
	memset(&stStat, 0x00, sizeof(struct stat));

	if (m_LOG.fpLog)
	{
		close(m_LOG.fpLog);
		m_LOG.fpLog = 0;
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

		m_LOG.fpLog = open(szFileName, O_CREAT|O_RDWR, 0666);

		if (m_LOG.fpLog == -1)
		{
			printf("Fail: open(%s)", szFileName);
			return;
		}

		if (fstat(m_LOG.fpLog, &stStat) < 0)
		{
			printf("Fail : [%s] Fstat(errno:%d)", szFileName, errno);
			return;
		}
#ifdef	linux
//		setlinebuf(m_LOG.fpLog);
#endif

		strcpy(m_LOG.szLogName, a_pszLogFile);
	}
	else
	{
		m_LOG.fpLog = dup(1);
	}

	m_LOG.aio_wb.aio_fildes  = m_LOG.fpLog;
	m_LOG.aio_wb.aio_offset  = stStat.st_size;

	char szLogBuff[DEF_BUFFER_1024];
	snprintf(szLogBuff, sizeof(szLogBuff), "\n\n\n-------------------------log open at %s-------------------------\n\n\n", time2str(&tNow));
	
	m_LOG.aio_wb.aio_buf = szLogBuff;
	m_LOG.aio_wb.aio_nbytes = strlen(szLogBuff);
	aio_write(&m_LOG.aio_wb);
			
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
	static char	szLogBuff[DEF_BUFFER_1024 * DEF_BUFFER_1024];
	static char	szDupMsgBuff[DEF_BUFFER_256];
	char	*pszLogHead = szLogBuff;
	char	*pszLogMsg;

	if ( a_nLevel > m_nLogLevel)
		return;

	if (m_bLockEnable)
	{
		pthread_mutex_lock(&m_tMutex);
	}

	time(&tNow);
	AdjustFileName( tNow);

	while(aio_error(&m_LOG.aio_wb) == EINPROGRESS);
	m_LOG.aio_wb.aio_offset += m_LOG.aio_wb.aio_nbytes;

	snprintf(pszLogHead, sizeof(szLogBuff), "%s, %s, %8s, "
					, time2str(&tNow)
					, m_szPrevMsg
					, g_stLogLevel[a_nLevel].pLevelString
	);

	pszLogMsg = pszLogHead + strlen(pszLogHead);

	va_start(ap, a_pszFmt);

	vsprintf(pszLogMsg, a_pszFmt, ap);
	sprintf(pszLogMsg + strlen(pszLogMsg), "\n");

	//현재의 Log Msg 와 이전에 씌여진 Log Msg 를 비교
	if(DiffLastLog(pszLogMsg, m_szLastLogMsg) == 0)
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
			va_end(ap);
			m_LOG.aio_wb.aio_offset -= m_LOG.aio_wb.aio_nbytes;
			return ;
		}
		//지정 된 시간 이후에 중복이 발생할 경우 중복이 발생하였어도 출력
		else if(m_nLogCheckTime > 0)
		{
			snprintf(szDupMsgBuff, sizeof(szDupMsgBuff), "%s, %s, %8s, Last Message was repeated %d times\n"
													, time2str(&tNow)
													, m_szPrevMsg
													, g_stLogLevel[a_nLevel].pLevelString
													, m_nDupCount
					);

			m_LOG.aio_wb.aio_buf = szDupMsgBuff;
			m_LOG.aio_wb.aio_nbytes = strlen(szDupMsgBuff);
			aio_write(&m_LOG.aio_wb);

			while(aio_error(&m_LOG.aio_wb) == EINPROGRESS);
			m_LOG.aio_wb.aio_offset += m_LOG.aio_wb.aio_nbytes;

			m_nDupCount = 0;
		}
	}
	//이전 Log Msg 와 같지 않고, Dup Count 가 존재할 경우 이전 Msg 의 중복 결과 출력 후
	//현재 Msg 출력
	else if(m_nDupCount)
	{
		snprintf(szDupMsgBuff, sizeof(szDupMsgBuff), "%s, %s, %8s, Last Message was repeated %d times\n"
												, time2str(&tNow)
												, m_szPrevMsg
												, g_stLogLevel[a_nLevel].pLevelString
												, m_nDupCount
				);

		m_LOG.aio_wb.aio_buf = szDupMsgBuff;
		m_LOG.aio_wb.aio_nbytes = strlen(szDupMsgBuff);
		aio_write(&m_LOG.aio_wb);

		m_nDupCount = 0;

		while(aio_error(&m_LOG.aio_wb) == EINPROGRESS);
		m_LOG.aio_wb.aio_offset += m_LOG.aio_wb.aio_nbytes;
	}
	
	m_LOG.aio_wb.aio_buf = szLogBuff;
	m_LOG.aio_wb.aio_nbytes = strlen(szLogBuff);
	aio_write(&m_LOG.aio_wb);


	if (m_bLockEnable)
	{
		pthread_mutex_unlock(&m_tMutex);
	}

	sprintf(m_szLastLogMsg, "%s", pszLogMsg);
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
	static	char	szDebugLogMsg[DEF_BUFFER_1024 * DEF_BUFFER_1024];
	va_list ap;

	va_start(ap, a_pszFmt);
	vsprintf(szDebugLogMsg, a_pszFmt, ap);
	va_end(ap);

	LogMsg(LV_DEBUG, "[%s:%d], %s", a_pszFile, a_nLine, szDebugLogMsg); 
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
 * \param a_pszReadBuf is Start Address to Read
 * \param a_nLen is Length of Memory
 * \return void
 */
void CFileLog::LogHexMsg( const char* a_pszReadBuf, int a_nLen)
{
	time_t			tNow;
	static char	szLogHexBuff[DEF_BUFFER_1024 * DEF_BUFFER_1024];

	if( LV_DEBUG > m_nLogLevel)
		return ;
	

	if (m_bLockEnable)
	{
		pthread_mutex_lock(&m_tMutex);
	}
	
	time(&tNow);
	AdjustFileName( tNow);

	while(aio_error(&m_LOG.aio_wb) == EINPROGRESS);
	m_LOG.aio_wb.aio_offset += m_LOG.aio_wb.aio_nbytes;

	HexDump((char*)a_pszReadBuf, (char *)szLogHexBuff, a_nLen, (char *)(szLogHexBuff+sizeof(szLogHexBuff)), tNow);
	m_LOG.aio_wb.aio_buf = szLogHexBuff;
	m_LOG.aio_wb.aio_nbytes = strlen(szLogHexBuff);
	aio_write(&m_LOG.aio_wb);

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
 * \param a_pszReadBuf is Start Address to Read
 * \param a_pszWriteBuf is Start Address to Write
 * \param a_nLen is Length of Memory
 * \param a_fpWrite is File Pointer for Write Log
 * \return void
 */
void CFileLog::HexDump(char* a_pszReadBuf, char *a_pszWriteBuf, int a_nSize, char *a_pEndOfWriteBuf, time_t a_tNow)
{   
    int             i, j, k;
	char *p = a_pszWriteBuf;

	
	snprintf((char*)p, a_pEndOfWriteBuf - p, "\n[%s] [Len : %d] \n-------------------------------------------------------------------\n", time2str(&a_tNow), a_nSize);
	p += strlen(p);
	for (i = 0; i < a_nSize; i++)
	{
		snprintf((char*)p, a_pEndOfWriteBuf - p, "%02x ", a_pszReadBuf[i]);
		p += strlen(p);
		if (i % 16 == 15 || i == a_nSize - 1)
		{
			for (k = i % 16; k < 15; k++)
			{
				snprintf(p, a_pEndOfWriteBuf - p, "   ");
				p += strlen(p);
			}
					
			snprintf(p, a_pEndOfWriteBuf - p, "   ");
			p += strlen(p);

			for (j = i - (i % 16); j <= i; j++)
            {       
				if (a_pszReadBuf[j] >= 32 && a_pszReadBuf[j] <= 126)
				{
					snprintf(p, a_pEndOfWriteBuf - p, "%c", a_pszReadBuf[j]);
					p += strlen(p);
				}
				else 
				{
					snprintf(p, a_pEndOfWriteBuf - p, ".");
					p += strlen(p);
				}
			}
                                                                                                                                         
			snprintf(p, a_pEndOfWriteBuf - p, "\n");
			p ++;
		}
	}   
        
	snprintf(p, a_pEndOfWriteBuf - p, "\n");
	p++;
	snprintf(p, a_pEndOfWriteBuf - p, "-------------------------------------------------------------------\n\n");
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
		snprintf(m_szHostName, sizeof(m_szHostName), "ATOM");
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


/*!
 * \brief OFCS Pkg 의 Log Class Wrapping 추후 삭제 예정
 * \return Succ 0, Fail -1
 */
int CFileLog::HexaLog(const char *a_szComment, const byte *a_szData, size_t a_nLen)
{
	LogHexMsg((const char*)a_szData, (int)a_nLen);
	return 0;
}

/*!
 * \brief OFCS Pkg 의 Log Class Wrapping 추후 삭제 예정
 * \return Succ 0, Fail -1
 */
int CFileLog::MessageLog(const char *a_pszFmt, ...)
{
	static	char	szLogMsg1[DEF_BUFFER_1024 * DEF_BUFFER_1024];
	va_list ap;

	va_start(ap, a_pszFmt);
	vsprintf(szLogMsg1, a_pszFmt, ap);
	va_end(ap);

	LogMsg(LV_INFO, "%s", szLogMsg1); 
	
	return 0;
}



/*!
 * \brief OFCS Pkg 의 Log Class Wrapping 추후 삭제 예정
 * \return Succ 0, Fail -1
 */
int CFileLog::MessageLog(int a_nLevel, const char *a_pszFmt, ...)
{
	static	char	szLogMsg2[DEF_BUFFER_1024 * DEF_BUFFER_1024];
	va_list ap;

	va_start(ap, a_pszFmt);
	vsprintf(szLogMsg2, a_pszFmt, ap);
	va_end(ap);

	LogMsg(a_nLevel, "%s", szLogMsg2); 

	return 0;
}


/*!
 * \brief OFCS Pkg 의 Log Class Wrapping 추후 삭제 예정
 * \return Succ 0, Fail -1
 */
int CFileLog::ChangeLogLevel(int a_nLogLevel)
{
	SetLogLevel(a_nLogLevel);
	return 0;
}
