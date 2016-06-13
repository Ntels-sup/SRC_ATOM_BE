#include "MEMORY.hpp"

bool memory_sort_function (RESOURCE_ATTR *a_pstSrc, RESOURCE_ATTR *a_pstDst)
{
	MEMORY_VALUE *pstDataSrc = (MEMORY_VALUE*)a_pstSrc->pData;
	MEMORY_VALUE *pstDataDst = (MEMORY_VALUE*)a_pstDst->pData;
	
	return pstDataSrc->vecCurValue[IDX_MEM_USAGE] > pstDataDst->vecCurValue[IDX_MEM_USAGE];
}

MEMORY::MEMORY()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_unRunCount = 1;
	m_pclsMain = NULL;
	m_pclsDB = NULL;

    m_bPrintTop = false;
    m_tNextPrint = 0;
    m_nPrintCount = 0;
    m_dThreshold = 0;
}


MEMORY::~MEMORY()
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
			delete (MEMORY_VALUE*)pstAttr->pData;
	}

}

int MEMORY::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	MEMORY_VALUE *pstMemData = NULL;	

	if(a_pclsLog == NULL)
		return -1;

	m_pclsLog = a_pclsLog;
	m_pGroupRsc = a_pRsc;
	m_pmapRsc = &(a_pRsc->mapRsc);	
	m_pclsMain = (RSAMain*)a_pclsMain;
    m_pclsEvent = m_pclsMain->GetEvent();

	m_pclsDB = m_pclsMain->GetDBConn();
	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("DB Class is NULL");
		return -1;
	}

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
			it->second->pData = (void*)new MEMORY_VALUE;
			pstMemData = (MEMORY_VALUE*)it->second->pData;
			pstMemData->vecSumValue.assign(MAX_MEM_IDX, 0);
			pstMemData->vecAvgValue.assign(MAX_MEM_IDX, 0);
			pstMemData->vecCurValue.assign(MAX_MEM_IDX, 0);
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_MEM_IDX);

	}	

    if( NULL != m_pclsMain->GetConfig()->GetConfigValue("RSA", "THRESHOLD_VALUE") )
    {
        m_dThreshold = atof(m_pclsMain->GetConfig()->GetConfigValue("RSA", "THRESHOLD_VALUE"));
    }

	return 0;
}

int MEMORY::InsertTopData()
{
	int ret = 0;
	char szBuffer[DEF_MEM_BUF_1024];
	MEMORY_VALUE *pstData = NULL;

	snprintf(szBuffer, sizeof(szBuffer), "DELETE FROM TAT_RSC_TOP WHERE NODE_NO = '%d' AND RSC_GRP_ID = '%s'"
							, m_pclsMain->GetNodeID()
							, m_pGroupRsc->szGroupID
			);

	if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0 )
	{
		m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}	

	std::sort(vecTop.begin(), vecTop.end(), memory_sort_function);
	int i = 0;
	for(std::vector<RESOURCE_ATTR*>::iterator it=vecTop.begin(); it != vecTop.end() && i < DEF_MAX_TOP_COUNT; ++it, i++)
	{
		pstData = (MEMORY_VALUE*)(*it)->pData;

		snprintf(szBuffer, sizeof(szBuffer), "INSERT INTO TAT_RSC_TOP VALUES ('%s', '%s', '%d', sysdate(), '%.2f')"
												, m_pGroupRsc->szGroupID
												, (*it)->szID
												, m_pclsMain->GetNodeID()
												, pstData->vecCurValue[IDX_MEM_USAGE]
				);		

		if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0 )
		{
			m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
			return -1;
		}	
	}

	return 0;
}

int MEMORY::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	MEMORY_VALUE *pstData = NULL;

	try {

		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;

		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (MEMORY_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_MEM_IDX ; i++)
			{
				sprintf(szBuff, "%.2f", pstData->vecCurValue[i]);
				o_rscAttr[MEM_COLUMN[i]] = szBuff;			
			}
	
		}
	

    	rabbit::object o_rootSummary;
	    o_rootSummary["NAME"] = m_pGroupRsc->szGroupName;

	    m_pGroupRsc->strSummaryJson.clear();

    	std::vector<RESOURCE_ATTR*>::iterator vecIt = vecTop.begin();

	    pstRsc = (*vecIt);
    	pstData = (MEMORY_VALUE*)pstRsc->pData;

	    o_rootSummary["LIST"] = pstRsc->szName;
//    	snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[IDX_MEM_USAGE]);
    	snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecCurValue[IDX_MEM_USAGE]);
	    o_rootSummary[MEM_COLUMN[IDX_MEM_USAGE]] = szBuff;

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->strSummaryJson.assign(o_rootSummary.str());

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("MEMORY MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("MEMORY MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("MEMORY MakeJson Parsing Error");
        return -1;
    } 


	m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	m_pGroupRsc->unExec |= DEF_EXEC_SET_SUMMARY;

	return 0;
}

int MEMORY::MakeStatJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	MEMORY_VALUE *pstData = NULL;

	CStmResRspApi encApi;
	CStmResRspApi decApi;
	std::string strData;

	encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (MEMORY_VALUE*)it->second->pData;
		for(int i = 0; i < MAX_MEM_IDX ; i++)
		{
			sprintf(szBuff, "%d", i+1);
			encApi[pstRsc->szID][szBuff].Init(a_tCur, m_pclsMain->GetNodeID(), Rounding(pstData->vecAvgValue[i], 2));
		}

		pstData->vecAvgValue.assign(MAX_MEM_IDX, 0);
		pstData->vecSumValue.assign(MAX_MEM_IDX, 0);
		pstData->vecCurValue.assign(MAX_MEM_IDX, 0);
		m_unRunCount = 1;
	
	}

	encApi.EncodeMessage(strData);
	m_pGroupRsc->strStatJson.assign(strData);

	return 0;

}

int MEMORY::PrintTopData(time_t a_tCur)
{
    FILE *fp = NULL;
    char szBuffer[DEF_MEM_BUF_1024];

    char szFullLogBuffer[DEF_MEM_BUF_2048];
    char *p = szFullLogBuffer;
    uint32_t unBuffLen = DEF_MEM_BUF_2048;

    int nLine = 0;

    if(a_tCur < m_tNextPrint)
        return 0;

    if(m_nPrintCount++ > DEF_MAX_PRINT_TOP_COUNT)
    {
        m_bPrintTop = false;
        m_nPrintCount = 0;
        m_tNextPrint = 0;

        return 0;
    }

    snprintf(szBuffer, sizeof(szBuffer), "top -b -n 1 | head -n 30");

    if( (fp = popen(szBuffer, "r")) == NULL )
    {
        m_pclsLog->ERROR("popen() error (%s)", szBuffer);
        return -1;
    }

    m_pclsLog->INFO(" [ Start to Print out Top Data by Memory ] ========================");
    while( fgets(szBuffer, sizeof(szBuffer), fp) != NULL )
    {
        if(unBuffLen < strlen(szBuffer))
        break;

        snprintf(p, unBuffLen, "%s", szBuffer);
        p += strlen(szBuffer);

        unBuffLen -= strlen(szBuffer);

        if(nLine++ >= 29)
            break;
    }
    m_pclsLog->INFO("\n\n%s\n", szFullLogBuffer);
    m_pclsLog->INFO(" ========================================================\n");

    m_tNextPrint = a_tCur + DEF_PRINT_TOP_PERIOD;

    pclose(fp);
    return 0;

}

int MEMORY::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	uint64_t	unTotal = 0;
	uint64_t	unFree = 0;
	uint64_t	unBuffer = 0;
	uint64_t	unCached = 0;
	uint64_t	unUsed = 0;

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	MEMORY_VALUE *pstData = NULL;

    time_t tCur;

    if(m_bPrintTop)
    {
        time(&tCur);
        PrintTopData(tCur);
    }

	vecTop.clear();

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (MEMORY_VALUE*)it->second->pData;
		
		if( (fp = fopen("/proc/meminfo", "r")) == NULL )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			return -1;
		}

		while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
		{
			if (!strncmp(szBuffer, "MemTotal:", 9))\
				/* Read the total amount of memory in kB */
				sscanf(szBuffer + 9, "%lu", &unTotal);
			else if (!strncmp(szBuffer, "MemFree:", 8))
				/* Read the amount of free memory in kB */
				sscanf(szBuffer + 8, "%lu", &unFree);
			else if (!strncmp(szBuffer, "Buffers:", 8))
				/* Read the amount of buffered memory in kB */
				sscanf(szBuffer + 8, "%lu", &unBuffer);
			else if (!strncmp(szBuffer, "Cached:", 7))
				/* Read the amount of cached memory in kB */
				sscanf(szBuffer + 7, "%lu", &unCached);
#if 0
			else if (!strncmp(szBuffer, "SwapTotal:", 10))
				/* Read the total amount of swap memory in kB */
				sscanf(szBuffer + 10, "%lu", &swapTotal);
			else if (!strncmp(szBuffer, "SwapFree:", 9))
				/* Read the amount of free swap memory in kB */
				sscanf(szBuffer + 9, "%lu", &swapFree);
			else if (!strncmp(szBuffer, "VmallocTotal:", 13))
				/* Read the total amount of virtual memory in kB */
				sscanf(szBuffer + 13, "%lu", &vmTotal);
			else if (!strncmp(szBuffer, "VmallocUsed:", 12))
				/* Read the usage amount of virtual memory in kB */
				sscanf(szBuffer + 12, "%lu", &vmUsed);
#endif
		}


		unUsed = unTotal - (unFree + unBuffer + unCached);

		m_pclsLog->DEBUG("MEMORY Total %u, Free %u, Used %u, Buffer %u, Cached %u",
						unTotal, unFree, unUsed, unBuffer, unCached);

		
		pstData->vecCurValue[IDX_MEM_TOTAL] = (double)unTotal / 1024;
		pstData->vecCurValue[IDX_MEM_FREE] 	= (double)(unTotal - unUsed) / 1024;
		pstData->vecCurValue[IDX_MEM_USED] 	= (double)unUsed / 1024;
		pstData->vecCurValue[IDX_MEM_USAGE] = 
				(pstData->vecCurValue[IDX_MEM_USED] / pstData->vecCurValue[IDX_MEM_TOTAL]) * 100;

		pstData->vecSumValue[IDX_MEM_TOTAL] += pstData->vecCurValue[IDX_MEM_TOTAL];
		pstData->vecSumValue[IDX_MEM_FREE] 	+= pstData->vecCurValue[IDX_MEM_FREE];
		pstData->vecSumValue[IDX_MEM_USED] 	+= pstData->vecCurValue[IDX_MEM_USED];
		pstData->vecSumValue[IDX_MEM_USAGE] += pstData->vecCurValue[IDX_MEM_USAGE];

		pstData->vecAvgValue[IDX_MEM_TOTAL] = pstData->vecSumValue[IDX_MEM_TOTAL] / m_unRunCount;
		pstData->vecAvgValue[IDX_MEM_FREE] 	= pstData->vecSumValue[IDX_MEM_FREE] / m_unRunCount;
		pstData->vecAvgValue[IDX_MEM_USED] 	= pstData->vecSumValue[IDX_MEM_USED] / m_unRunCount;
		pstData->vecAvgValue[IDX_MEM_USAGE] = pstData->vecSumValue[IDX_MEM_USAGE] / m_unRunCount;

        if( (pstData->vecAvgValue[IDX_MEM_USAGE] >= m_dThreshold) && (m_dThreshold != 0) )
        {
            time(&tCur);
            m_bPrintTop = true;
            PrintTopData(tCur);
        }

        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_MEMORY_OVER, 
                                pstRsc->szName, 
                                Rounding(pstData->vecAvgValue[IDX_MEM_USAGE], 2), 
                                NULL, NULL
                             );

		m_pclsLog->DEBUG("NEW MEMORY Usage %.2f, Total %.2f, Free %.2f, Used %.2f",
						pstData->vecAvgValue[IDX_MEM_USAGE]	,
						pstData->vecAvgValue[IDX_MEM_TOTAL]	,
						pstData->vecAvgValue[IDX_MEM_FREE]	,
						pstData->vecAvgValue[IDX_MEM_USED]	
						);
		
		
		vecTop.push_back(pstRsc);			

		fclose(fp);
	}

	m_unRunCount++;

	InsertTopData();

	return 0;
}

extern "C"
{
	RSABase *PLUG0002_MEM()
	{
		return new MEMORY;
	}

}
