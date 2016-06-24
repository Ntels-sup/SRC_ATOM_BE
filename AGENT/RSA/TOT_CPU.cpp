#include "TOT_CPU.hpp"

TOT_CPU::TOT_CPU()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_unRunCount = 1;
	m_pclsMain = NULL;

    m_bPrintTop = false;
    m_tNextPrint = 0;
    m_nPrintCount = 0;
    m_dThreshold = 0;

    m_pclsDB = NULL;

	m_mapArgs.clear();
//	printf("TOT_CPUis Starting\n");
}


TOT_CPU::~TOT_CPU()
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
			delete (TOT_CPU_VALUE*)pstAttr->pData;
	}

	m_mapArgs.clear();

}

int TOT_CPU::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	TOT_CPU_VALUE *pstTotCpuData = NULL;	

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
			it->second->pData = (void*)new TOT_CPU_VALUE;
			pstTotCpuData = (TOT_CPU_VALUE*)it->second->pData;
			pstTotCpuData->vecOldValue.assign(MAX_CPU_IDX, 0);
			pstTotCpuData->vecSumValue.assign(MAX_CPU_IDX, 0);
			pstTotCpuData->vecAvgValue.assign(MAX_CPU_IDX, 0);
			pstTotCpuData->vecCurValue.assign(MAX_CPU_IDX, 0);
		}

		m_mapArgs.insert(std::pair<std::string, RESOURCE_ATTR*> (pstAttr->szArgs, pstAttr));
		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_CPU_IDX);

	}	

    if( NULL != m_pclsMain->GetConfig()->GetConfigValue("RSA", "THRESHOLD_VALUE") )
    {
        m_dThreshold = atof(m_pclsMain->GetConfig()->GetConfigValue("RSA", "THRESHOLD_VALUE"));
    }

	return 0;
}

int TOT_CPU::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	TOT_CPU_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;

		rabbit::array a_rscList = o_root["LIST"];
		rabbit::object o_rscAttr;

		rabbit::object o_rootSummary;
		o_rootSummary["NAME"] = m_pGroupRsc->szGroupName;

		m_pGroupRsc->strSummaryJson.clear();	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (TOT_CPU_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rootSummary["LIST"] = pstRsc->szName;

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_CPU_IDX ; i++)
			{
				if(i == IDX_CPU_NO)
//					sprintf(szBuff, "%.0f", pstData->vecAvgValue[i]);
					sprintf(szBuff, "%.0f", pstData->vecCurValue[i]);
				else
//					sprintf(szBuff, "%.2f", pstData->vecAvgValue[i]);
					sprintf(szBuff, "%.2f", pstData->vecCurValue[i]);
	
				o_rscAttr[CPU_COLUMN[i]] = szBuff;
			}

			//Insert Summary Info
//			snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[IDX_CPU_USAGE]);
			snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecCurValue[IDX_CPU_USAGE]);
			o_rootSummary[CPU_COLUMN[IDX_CPU_USAGE]] = szBuff;
		}

		pstData->vecCurValue.assign(MAX_CPU_IDX, 0);
		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->strSummaryJson.assign(o_rootSummary.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("TOTAL CPU MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("TOTAL CPU MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("TOTAL CPU MakeJson Parsing Error");
        return -1;
    }


	m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	m_pGroupRsc->unExec |= DEF_EXEC_SET_SUMMARY;

	return 0;
}

int TOT_CPU::MakeStatJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	TOT_CPU_VALUE *pstData = NULL;

	CStmResRspApi encApi;
	CStmResRspApi decApi;
	std::string strData;

	encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);
	
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (TOT_CPU_VALUE*)it->second->pData;
		for(int i = 0; i < MAX_CPU_IDX ; i++)
		{
			sprintf(szBuff, "%d", i+1);
			encApi[pstRsc->szID][szBuff].Init(a_tCur, m_pclsMain->GetNodeID(), Rounding(pstData->vecAvgValue[i], 2));
		}

		pstData->vecSumValue.assign(MAX_CPU_IDX, 0);
		pstData->vecAvgValue.assign(MAX_CPU_IDX, 0);
		pstData->vecCurValue.assign(MAX_CPU_IDX, 0);
		m_unRunCount = 1;

	}

	encApi.EncodeMessage(strData);
	m_pGroupRsc->strStatJson.assign(strData);

	return 0;

}

int TOT_CPU::InsertTopData()
{
    int ret = 0;
    char szBuffer[DEF_MEM_BUF_1024];

    map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
    TOT_CPU_VALUE *pstData = NULL;

    snprintf(szBuffer, sizeof(szBuffer), "DELETE FROM TAT_RSC_TOP WHERE NODE_NO = '%d' AND RSC_GRP_ID = '%s'"
                            , m_pclsMain->GetNodeID()
                            , m_pGroupRsc->szGroupID
    );

    if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0 )
    {
        m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
        return -1;
    }

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (TOT_CPU_VALUE*)it->second->pData;

        snprintf(szBuffer, sizeof(szBuffer), "INSERT INTO TAT_RSC_TOP VALUES ('%s', '%s', '%d', sysdate(), '%.2f')"
                                        , m_pGroupRsc->szGroupID
                                        , pstRsc->szID
                                        , m_pclsMain->GetNodeID()
                                        , pstData->vecCurValue[IDX_CPU_USAGE]
        );

        if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) <= 0 )
        {
            m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
            return -1;
        }
    }
    return 0; 

}

int TOT_CPU::PrintTopData(time_t a_tCur)
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

    m_pclsLog->INFO(" [ Start to Print out Top Data by Total CPU ] ========================");
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

int TOT_CPU::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char		szName[DEF_MEM_BUF_64];
	uint64_t	unTotal;
	uint64_t	unCPU;
	uint64_t	unUser;
	uint64_t	unNice;
	uint64_t	unSys;
	uint64_t	unIdle;
	uint64_t	unWait;
	uint64_t	unHardIRQ;
	uint64_t	unSoftIRQ;

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	TOT_CPU_VALUE *pstData = NULL;
	vector<uint64_t> vecDuration ;
	vecDuration.assign(MAX_CPU_IDX, 0);

    time_t tCur;

    if(m_bPrintTop)
    {
        time(&tCur);
        PrintTopData(tCur);
    }

	if( (fp = fopen("/proc/stat", "r")) == NULL )
	{
		m_pclsLog->ERROR("fopen() error (%s)", "/proc/stat");
		return -1;
	}

	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		memset(szName, 0x00, sizeof(szName));
		unCPU = 0;
		unUser = 0;
		unNice = 0;
		unSys = 0;
		unIdle = 0;
		unWait = 0;
		unHardIRQ = 0;
		unSoftIRQ = 0;

		sscanf(szBuffer, "%s %lu %lu %lu %lu %lu %lu %lu",
				szName,
				&unUser,
				&unNice,
				&unSys,
				&unIdle,
				&unWait,
				&unHardIRQ,
				&unSoftIRQ
		);

		it = m_mapArgs.find(szName);

		if(it == m_mapArgs.end())
		{
			continue;
		}

		pstRsc = it->second;
		pstData = (TOT_CPU_VALUE*)it->second->pData;
	
		unSys += unHardIRQ + unSoftIRQ;


		m_pclsLog->DEBUG("TOT CPU NO %u, User %u, Nice %u, Sys %u, Idle %u, Wait %u, unHardIRQ %u, unSoftIRQ %u",
						unCPU, unUser, unNice, unSys, unIdle, unWait, unHardIRQ, unSoftIRQ);

		vecDuration[IDX_CPU_USER] = unUser - pstData->vecOldValue[IDX_CPU_USER];
		vecDuration[IDX_CPU_SYS] = unSys - pstData->vecOldValue[IDX_CPU_SYS];
		vecDuration[IDX_CPU_NICE] = unNice - pstData->vecOldValue[IDX_CPU_NICE];
		vecDuration[IDX_CPU_IDLE] = unIdle - pstData->vecOldValue[IDX_CPU_IDLE];
		vecDuration[IDX_CPU_WAIT] = unWait - pstData->vecOldValue[IDX_CPU_WAIT];


		pstData->vecOldValue[IDX_CPU_NO] = unCPU;
		pstData->vecOldValue[IDX_CPU_USER] = unUser;
		pstData->vecOldValue[IDX_CPU_SYS]  = unSys;
		pstData->vecOldValue[IDX_CPU_NICE] = unNice;
		pstData->vecOldValue[IDX_CPU_IDLE] = unIdle;
		pstData->vecOldValue[IDX_CPU_WAIT] = unWait;


		unTotal = vecDuration[IDX_CPU_USER] + vecDuration[IDX_CPU_SYS] +
					vecDuration[IDX_CPU_NICE] + vecDuration[IDX_CPU_IDLE] + vecDuration[IDX_CPU_WAIT];


		if(unTotal == 0)
		{
			m_pclsLog->ERROR("Total Duration time of CPU is zero");
			continue;
		}

		pstData->vecAvgValue[IDX_CPU_NO] = unCPU;
		pstData->vecCurValue[IDX_CPU_NO] = unCPU;

		if(vecDuration[IDX_CPU_USER] > 0)
		{
			pstData->vecCurValue[IDX_CPU_USER] = (double) (vecDuration[IDX_CPU_USER] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_USER] += pstData->vecCurValue[IDX_CPU_USER];
			pstData->vecAvgValue[IDX_CPU_USER]  = pstData->vecSumValue[IDX_CPU_USER] / m_unRunCount;
		}

		if(vecDuration[IDX_CPU_SYS] > 0)
		{
			pstData->vecCurValue[IDX_CPU_SYS] = (double) (vecDuration[IDX_CPU_SYS] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_SYS] += pstData->vecCurValue[IDX_CPU_SYS];
			pstData->vecAvgValue[IDX_CPU_SYS]  = pstData->vecSumValue[IDX_CPU_SYS] / m_unRunCount;
		}

		if(vecDuration[IDX_CPU_NICE] > 0)
		{
			pstData->vecCurValue[IDX_CPU_NICE] = (double) (vecDuration[IDX_CPU_NICE] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_NICE] += pstData->vecCurValue[IDX_CPU_NICE];
			pstData->vecAvgValue[IDX_CPU_NICE]  = pstData->vecSumValue[IDX_CPU_NICE] / m_unRunCount;
		}

		if(vecDuration[IDX_CPU_IDLE] > 0)
		{
			pstData->vecCurValue[IDX_CPU_IDLE] = (double) (vecDuration[IDX_CPU_IDLE] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_IDLE] += pstData->vecCurValue[IDX_CPU_IDLE];
			pstData->vecAvgValue[IDX_CPU_IDLE]  = pstData->vecSumValue[IDX_CPU_IDLE] / m_unRunCount;
		}

		if(vecDuration[IDX_CPU_WAIT] > 0)
		{
			pstData->vecCurValue[IDX_CPU_WAIT] = (double) (vecDuration[IDX_CPU_WAIT] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_WAIT] += pstData->vecCurValue[IDX_CPU_WAIT];
			pstData->vecAvgValue[IDX_CPU_WAIT]  = pstData->vecSumValue[IDX_CPU_WAIT] / m_unRunCount;
		}

		pstData->vecCurValue[IDX_CPU_USAGE] = (double) (100 - pstData->vecCurValue[IDX_CPU_IDLE]);
		pstData->vecSumValue[IDX_CPU_USAGE] += pstData->vecCurValue[IDX_CPU_USAGE];
		pstData->vecAvgValue[IDX_CPU_USAGE]  = pstData->vecSumValue[IDX_CPU_USAGE] / m_unRunCount;


        if( (pstData->vecAvgValue[IDX_CPU_USAGE] >= m_dThreshold) && (m_dThreshold != 0) )
        {
            time(&tCur);
            m_bPrintTop = true;
            PrintTopData(tCur);
        }

        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_TOTAL_CPU_OVER, 
                                pstRsc->szName, 
                                Rounding(pstData->vecAvgValue[IDX_CPU_USAGE], 2), 
                                NULL, NULL
                             );

		m_pclsLog->DEBUG("NEW TOTAL_CPU NO %u, Usage %.2f/%u, User %.2f/%u, Nice %.2f/%u, Sys %.2f/%u, Idle %.2f/%u, Wait %.2f/%u",
						unCPU,
						pstData->vecCurValue[IDX_CPU_USAGE]	,
						pstData->vecOldValue[IDX_CPU_USAGE]	,
						pstData->vecCurValue[IDX_CPU_USER]	,
						pstData->vecOldValue[IDX_CPU_USER]	,
						pstData->vecCurValue[IDX_CPU_NICE]	,
						pstData->vecOldValue[IDX_CPU_NICE]	,
						pstData->vecCurValue[IDX_CPU_SYS]	,
						pstData->vecOldValue[IDX_CPU_SYS]	,
						pstData->vecCurValue[IDX_CPU_IDLE]	,
						pstData->vecOldValue[IDX_CPU_IDLE]	,
						pstData->vecCurValue[IDX_CPU_WAIT]	,
						pstData->vecOldValue[IDX_CPU_WAIT]	
						);
		
		
	}

	fclose(fp);

    InsertTopData();

	m_unRunCount++;
	return 0;
}

extern "C"
{
	RSABase *PLUG0004_TOT_CPU()
	{
		return new TOT_CPU;
	}
}
