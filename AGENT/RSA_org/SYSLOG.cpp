#include "SYSLOG.hpp"

SYSLOG::SYSLOG()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

    m_nInitDate = 0;
    m_nInitTime = 0;

}


SYSLOG::~SYSLOG()
{
	RESOURCE_ATTR *pstAttr = NULL;
	map<string, RESOURCE_ATTR *>::iterator it;

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		if(it->second != NULL)
		{
			pstAttr = it->second;
		}
		else
		{
			continue;
		}

		if(pstAttr->pData)
			delete (SYSLOG_VALUE*)pstAttr->pData;
	}

}

int SYSLOG::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{

    char szBuff[DEF_MEM_BUF_256];
    FILE *fp = NULL;
    struct timeb tCur;

    if(a_pclsLog == NULL)
		return -1;

	m_pclsLog = a_pclsLog;
	m_pGroupRsc = a_pRsc;
	m_pmapRsc = &(a_pRsc->mapRsc);	
	m_pclsMain = (RSARoot*)a_pclsMain;
	
	CConfig *pclsConfig = m_pclsMain->GetConfig();

    m_strSysLogPath.assign(pclsConfig->GetConfigValue("RSA", "SYS_LOG_PATH"));
	m_strOldSysLogPath.assign(pclsConfig->GetConfigValue("RSA", "OLD_SYS_LOG_PATH"));

	if( 0 == m_strSysLogPath.size() )
	{
		m_pclsLog->ERROR("SYS LOG PATH is Null");
		return -1;
	}

    if( 0 == m_strOldSysLogPath.size() )
	{
		m_pclsLog->ERROR("Old SYS LOG PATH is Null");
		return -1;
	}

    memset(szBuff, 0x00, sizeof(szBuff));
    snprintf(szBuff, sizeof(szBuff), "cat %s | wc -l", m_strSysLogPath.c_str());
    
    if( !( fp = popen(szBuff, "r") ) )
    {
        m_pclsLog->ERROR("Fail to Open System Log (%s)", m_strSysLogPath.c_str());
        return -1;
    }

    if(!(fgets(szBuff, sizeof(szBuff), fp)))
    {
            m_pclsLog->ERROR("Syslog message read failure");
            pclose(fp);
            return -1 ;
    }
    pclose(fp);

    m_strPrevLogLine.assign(szBuff);

    ftime(&tCur);
    m_nInitDate = atoi(GetAscTime("CCYYMMDD", tCur));
    m_nInitTime = atoi(GetAscTime("hhmmss", tCur));

	RESOURCE_ATTR *pstAttr = NULL;
	SYSLOG_VALUE *pstTempData = NULL;	
	map<string, RESOURCE_ATTR *>::iterator it;
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		if(it->second != NULL)
		{
			pstAttr = it->second;
		}
		else
		{
			break;
		}
		
		if(pstAttr->pData == NULL)
		{
			//Init Stat Data
			it->second->pData = (void*)new SYSLOG_VALUE;
			pstTempData = (SYSLOG_VALUE*)it->second->pData;
			pstTempData->bFind = false;
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_SYSLOG_IDX);

	}	

//    m_pclsMain->SendTrapMsgToRSA( DEF_ALM_CODE_SYS_LOG, "SYSTEM_LOG", "FAULT" );

	return 0;
}

int SYSLOG::MakeTrapJson()
{
	char szBuff[DEF_MEM_BUF_128];	
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	SYSLOG_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];
	
		m_pGroupRsc->strRootTrapJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (SYSLOG_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

        	o_rscAttr["CODE"] = DEF_ALM_CODE_SYS_LOG;
    	    o_rscAttr["TARGET"] = pstRsc->szName;

	        snprintf(szBuff, sizeof(szBuff), "%c", pstData->bFind ? 'Y' : 'N' );
			pstData->bFind = false;
    	    o_rscAttr["VALUE"] = szBuff;

		}

		m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("SYSLOG MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("SYSLOG MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("SYSLOG MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}

int SYSLOG::MakeJson(time_t a_tCur)
{
    MakeTrapJson();

	return 0;
#if 0
	char szBuff[DEF_MEM_BUF_128];	
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	SYSLOG_VALUE *pstData = NULL;

	rabbit::object o_root;
	o_root["NAME"] = m_pGroupRsc->szGroupName;
	rabbit::array a_rscList = o_root["LIST"];

	rabbit::object o_rscAttr;
	
	
	m_pGroupRsc->strFullJson.clear();

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (SYSLOG_VALUE*)it->second->pData;
		a_rscList.push_back(StringRef(pstRsc->szName));

		o_rscAttr = o_root[pstRsc->szName];

		for(int i = 0; i < MAX_SYSLOG_IDX ; i++)
		{
			sprintf(szBuff, "%.2f", pstData->vecCurValue[i]);
			o_rscAttr[SYSLOG_COLUMN[i]] = szBuff;

		}
	}

	m_pGroupRsc->strFullJson.assign(o_root.str());
	m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
#endif
	return 0;
}

char *SYSLOG::GetAscTime(const char *a_szFormat, struct timeb a_tCur)
{
    static char szAscTm[33];

    struct tm  *tBlock ;
    struct tm   tResult ;
    char        szTimeStr[24] ;
    char        szFormat[32] ;
    char       *CC,*YY,*DD,*MM,*hh,*mm,*ss,*XX ;
    char       *pDst ;
    char       *p ;
    char        cValue ;
    int         narrCnt[8] ;
    int         nFormatLen = strlen(a_szFormat) ;

    if ( a_szFormat == NULL || nFormatLen > (int)(sizeof(szAscTm))-1 )
    {
        m_pclsLog->ERROR("Format size too long") ;
        return NULL ;
    }

    strcpy(szFormat, a_szFormat) ;
    p = strstr(szFormat,"YYYY") ;
    if ( p != NULL )
    {
        strncpy(p,"CC",2) ;
    }

    tBlock = localtime_r(&a_tCur.time, &tResult) ;
    sprintf(szTimeStr,"%04d%02d%02d%02d%02d%02d%03d",tBlock->tm_year+1900,
                                                    tBlock->tm_mon+1,
                                                    tBlock->tm_mday,
                                                    tBlock->tm_hour,
                                                    tBlock->tm_min,
                                                    tBlock->tm_sec,
                                                    a_tCur.millitm) ;


    pDst = szAscTm ;

    CC = szTimeStr +  0  ;
    YY = szTimeStr +  2 ;
    MM = szTimeStr +  4 ;
    DD = szTimeStr +  6 ;
    hh = szTimeStr +  8 ;
    mm = szTimeStr + 10 ;
    ss = szTimeStr + 12 ;
    XX = szTimeStr + 14 ;

    for(int i = 0 ; i < 8 ; i++)
        narrCnt[i] = 0 ;

    for(int i = 0 ; i < nFormatLen ; i++)
    {
        switch(szFormat[i])
        {
            case 'C' :            // Century
                if ( narrCnt[0] == 2 )
                    return NULL ;
                cValue = *CC++ ;
                narrCnt[0]++ ;
                break ;
            case 'Y' :
                if ( narrCnt[1] == 2 )
                    return NULL ;
                cValue = *YY++ ;   // Year
                narrCnt[1]++ ;
                break ;
            case 'M' :
                if ( narrCnt[2] == 2 )
                    return NULL;
                cValue = *MM++ ;   // Month
                narrCnt[2]++ ;
                break ;
            case 'D' :
                if ( narrCnt[3] == 2 )
                    return NULL ;
                cValue = *DD++ ;   // Day
                narrCnt[3]++ ;
                break ;
            case 'h' :
                if ( narrCnt[4] == 2 )
                    return NULL;
                cValue = *hh++ ;   // Hour
                narrCnt[4]++ ;
                break ;
            case 'm' :
                if ( narrCnt[5] == 2 )
                    return NULL ;
                cValue = *mm++ ;   // Minute
                narrCnt[5]++ ;
                break ;
            case 's' :
                if ( narrCnt[6] == 2 )
                    return NULL ;
                cValue = *ss++ ;   // Second
                narrCnt[6]++ ;
                break ;
            case 'X' :            // Milli Second 
            case 'c' :
                if ( narrCnt[7] == 3 )
                    return NULL ;
                cValue = *XX++ ;
                narrCnt[7]++ ;
                break ;
            default :
                cValue = szFormat[i] ;
                break ;
        }
        *pDst++ = cValue ;
    }
    *pDst = 0x00 ;

    return szAscTm ;
}

int SYSLOG::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_8192];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	SYSLOG_VALUE *pstData = NULL;

	int ret = 0;

    int nInterval = 0;
    struct stat stFileInfo;

    char szCurLogLine[DEF_MEM_BUF_32];
    char szNowDate[DEF_MEM_BUF_32];
    char szNowTime[DEF_MEM_BUF_32];
    struct timeb tCur;

    char szTempLog[DEF_MEM_BUF_8192];

    char szArrMonth[12][4] =
            { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char *pszMonth, *pszDay, *pszTime, *pszHour, *pszMin, *pszSec;
    char szLogDate[DEF_MEM_BUF_16];
    char szLogTime[DEF_MEM_BUF_16];
    char szLogMonth[DEF_MEM_BUF_16];
    char szLogDay[DEF_MEM_BUF_16];

    int nLogDate = 0;
    int nLogTime = 0;

    ftime(&tCur);
    snprintf(szNowDate, sizeof(szNowDate), "%s", GetAscTime("CCYYMMDD", tCur));
    snprintf(szNowTime, sizeof(szNowTime), "%s", GetAscTime("hhmmsscc", tCur));

	snprintf(szBuffer, sizeof(szBuffer), "cat %s | wc -l", m_strSysLogPath.c_str());
		
	if( (fp = popen(szBuffer, "r")) == NULL )
	{
		m_pclsLog->ERROR("popen() error (%s), %d", szBuffer, errno);
		return -1;
	}

    if( !(fgets(szCurLogLine, sizeof(szCurLogLine), fp)) )
    {
        m_pclsLog->ERROR("Syslog message read failure");
        pclose(fp);
        return -1;
    }

    pclose(fp);
   
     
	snprintf(szBuffer, sizeof(szBuffer), "%s", m_strOldSysLogPath.c_str());
    if( (ret = stat(szBuffer, &stFileInfo)) != 0 )
    {
        nInterval = atoi(szCurLogLine) - atoi(m_strPrevLogLine.c_str());
    }
    else
    {
        if( (stFileInfo.st_mtime >= tPrevTime) && (stFileInfo.st_mtime < tCur.time) )
        {
            nInterval = atoi(szCurLogLine);
        }
        else
        {
            nInterval = atoi(szCurLogLine) - atoi(m_strPrevLogLine.c_str());
        }
    }
    
    memset(szBuffer, 0x00, sizeof(szBuffer));
    snprintf(szBuffer, sizeof(szBuffer), "tail -%d %s", nInterval, m_strSysLogPath.c_str());
    
    if( (fp = popen(szBuffer, "r")) == NULL )
    {
		m_pclsLog->ERROR("popen() error (%s), %d", szBuffer, errno);
		return -1;
    }


	while( !feof(fp) )
	{

        if( fgets(szBuffer, sizeof(szBuffer), fp) == NULL )
            break;

		m_pclsLog->DEBUG("szBuffer %s\n", szBuffer);
        
        memset(szTempLog, 0x00, sizeof(szTempLog));
        strcpy(szTempLog, szBuffer);
        pszMonth = strtok(szTempLog, " ");
        pszDay   = strtok(NULL, " ");
        pszTime  = strtok(NULL, " ");

        for(int nMidx = 0; nMidx < 12 ; nMidx++)
        {
            if( strncmp( szArrMonth[nMidx], pszMonth, 3) == 0)
            {
                snprintf(szLogMonth, sizeof(szLogMonth), "%02d", nMidx+1);
                break;
            }
            else
            {
                snprintf(szLogMonth, sizeof(szLogMonth), "00");
            }
        }

        if( !strncmp(szLogMonth, "00", 2) ) break;

        if( atoi(pszDay) < 10 ) 
        {
            snprintf(szLogDay, sizeof(szLogDay), "%02d", atoi(pszDay));
        }
        else
        {
            snprintf(szLogDay, sizeof(szLogDay), "%s", pszDay);
        }
    
        snprintf(szLogDate, sizeof(szLogDate), "%s%s%s", GetAscTime("CCYY", tCur), szLogMonth, szLogDay);

        pszHour = strtok(pszTime, ":");
        pszMin  = strtok(NULL, ":");
        pszSec  = strtok(NULL, ":");
        snprintf(szLogTime, sizeof(szLogTime), "%s%s%s", pszHour, pszMin, pszSec); 

        nLogDate = atoi(szLogDate);
        nLogTime = atoi(szLogTime);

        m_pclsLog->DEBUG("szLogDate %s, szLogTime %s", szLogDate, szLogTime);

        for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (SYSLOG_VALUE*)it->second->pData;
	
			if (strstr(szBuffer, pstRsc->szArgs) == NULL)
				continue;

		    m_pclsLog->WARNING("SysLog Occur : %s", szBuffer);
            pstData->bFind = true;
			break;
		}

            //ALA Send Trap Msg
//            m_pclsMain->SendTrapMsgToRSA(
  //                                      DEF_ALM_CODE_SYS_LOG, "SYSTEM_LOG", "FAULT"
    //                                );

	}

	pclose(fp);

    m_strPrevLogLine.assign(szCurLogLine);
	return 0;
}

extern "C"
{
	RSABase *PLUG0019_SYSLOG()
	{
		return new SYSLOG;
	}
}
