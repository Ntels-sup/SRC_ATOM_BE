#include "CPU.hpp"

//Move to Total CPU
#if 0
bool cpu_sort_function (RESOURCE_ATTR *a_pstSrc, RESOURCE_ATTR *a_pstDst)
{
	CPU_VALUE *pstDataSrc = (CPU_VALUE*)a_pstSrc->pData;
	CPU_VALUE *pstDataDst = (CPU_VALUE*)a_pstDst->pData;
	
	return pstDataSrc->vecCurValue[IDX_CPU_USAGE] > pstDataDst->vecCurValue[IDX_CPU_USAGE];
}
#endif

CPU::CPU()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_unRunCount = 1;
	m_pclsMain = NULL;
	m_pclsDB = NULL;

	m_mapArgs.clear();
//Move to Total CPU
#if 0
	m_bPrintTop = false;
	m_tNextPrint = 0;
	m_nPrintCount = 0;
#endif
}


CPU::~CPU()
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
			delete (CPU_VALUE*)pstAttr->pData;
	}

	m_mapArgs.clear();

}

int CPU::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	CPU_VALUE *pstCpuData = NULL;	

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
			continue;
		}
		
		if(pstAttr->pData == NULL)
		{
			//Init Stat Data
			it->second->pData = (void*)new CPU_VALUE;
			pstCpuData = (CPU_VALUE*)it->second->pData;
			pstCpuData->vecOldValue.assign(MAX_CPU_IDX, 0);
			pstCpuData->vecSumValue.assign(MAX_CPU_IDX, 0);
			pstCpuData->vecAvgValue.assign(MAX_CPU_IDX, 0);
			pstCpuData->vecCurValue.assign(MAX_CPU_IDX, 0);
		}

		m_mapArgs.insert(std::pair<std::string, RESOURCE_ATTR*> (pstAttr->szArgs, pstAttr));

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_CPU_IDX);

	}

//Move to Total CPU
#if 0
	if( NULL != m_pclsMain->GetConfig()->GetConfigValue("RSA", "THRESHOLD_VALUE") )
	{
		m_dThreshold = atof(m_pclsMain->GetConfig()->GetConfigValue("RSA", "THRESHOLD_VALUE"));
	}
#endif


	return 0;
}

int CPU::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	CPU_VALUE *pstData = NULL;

	//TEST Source
	rabbit::object o_root;
	rabbit::object o_rscAttr;

	try{
		
		o_root["NAME"] = m_pGroupRsc->szGroupName;

		rabbit::array a_rscList = o_root["LIST"];

		m_pGroupRsc->strFullJson.clear();
	
		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (CPU_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

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

			pstData->vecCurValue.assign(MAX_CPU_IDX, 0);
		} 

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("CPU MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("CPU MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("CPU MakeJson Parsing Error");
        return -1;
    }	


	return 0;
}

int CPU::MakeStatJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	CPU_VALUE *pstData = NULL;

	CStmResRspApi encApi;
	CStmResRspApi decApi;
	std::string strData;

	encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);
	
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (CPU_VALUE*)it->second->pData;
		for(int i = 0; i < MAX_CPU_IDX ; i++)
		{
			sprintf(szBuff, "%d", i+1);
			encApi[pstRsc->szID][szBuff].Init(a_tCur, m_pclsMain->GetNodeID(), Rounding(pstData->vecAvgValue[i], 2));
		}

		pstData->vecAvgValue.assign(MAX_CPU_IDX, 0);
		pstData->vecSumValue.assign(MAX_CPU_IDX, 0);
		pstData->vecCurValue.assign(MAX_CPU_IDX, 0);
		m_unRunCount = 1;
	}

	encApi.EncodeMessage(strData);

	m_pGroupRsc->strStatJson.assign(strData);

	return 0;

}

//Move to Total CPU
#if 0
int CPU::InsertTopData()
{
	int ret = 0;
	char szBuffer[DEF_MEM_BUF_1024];
	CPU_VALUE *pstData = NULL;

	snprintf(szBuffer, sizeof(szBuffer), "DELETE FROM TAT_RSC_TOP WHERE NODE_NO = '%d' AND RSC_GRP_ID = '%s'"
							, m_pclsMain->GetNodeID()
							, m_pGroupRsc->szGroupID
			);

	if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0 )
	{
		m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}	

	std::sort(vecTop.begin(), vecTop.end(), cpu_sort_function);
	int i = 0;
	for(std::vector<RESOURCE_ATTR*>::iterator it=vecTop.begin(); it != vecTop.end() && i < DEF_MAX_TOP_COUNT; ++it, i++)
	{
		pstData = (CPU_VALUE*)(*it)->pData;

		snprintf(szBuffer, sizeof(szBuffer), "INSERT INTO TAT_RSC_TOP VALUES ('%s', '%s', '%d', sysdate(), '%.2f')"
												, m_pGroupRsc->szGroupID
												, (*it)->szID
												, m_pclsMain->GetNodeID()
												, pstData->vecAvgValue[IDX_CPU_USAGE]
				);		

		if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) <= 0 )
		{
			m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
			return -1;
		}	
	}
	return 0;
}

int CPU::PrintTopData(time_t a_tCur)
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
	
	m_pclsLog->INFO(" [ Start to Print out Top Data ] ========================");
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
	return 0;
}
#endif

int CPU::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char 		szName[DEF_MEM_BUF_64];
	uint64_t	unCPU;
	uint64_t	unTotal;
	uint64_t	unUser;
	uint64_t	unNice;
	uint64_t	unSys;
	uint64_t	unIdle;
	uint64_t	unWait;
	uint64_t	unHardIRQ;
	uint64_t	unSoftIRQ;

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	CPU_VALUE *pstData = NULL;
	vector<uint64_t> vecDuration ;
	vecDuration.assign(MAX_CPU_IDX, 0);

//Move to Total CPU
#if 0
	time_t tCur;


	if(m_bPrintTop)
	{
		time(&tCur);
		PrintTopData(tCur);
	}	
#endif
	vecTop.clear();

	if( (fp = fopen("/proc/stat", "r")) == NULL )
	{
		m_pclsLog->ERROR("popen() error (%s)", "/proc/stat");
		return -1;
	}

	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		memset(szName, 0x00, sizeof(szName));
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
		pstData = (CPU_VALUE*)it->second->pData;
		
		unCPU = atoi(&szName[3]);			
		unSys += unHardIRQ + unSoftIRQ;
				

		m_pclsLog->DEBUG("CPU NO %u, User %u, Nice %u, Sys %u, Idle %u, Wait %u, unHardIRQ %u, unSoftIRQ %u",
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
			m_pclsLog->ERROR("Total Duration time of CPU[%d] is zero", unCPU);
			continue;
		}


//			pstData->vecAvgValue.assign(MAX_CPU_IDX, 0);

		pstData->vecAvgValue[IDX_CPU_NO] = unCPU;	
		pstData->vecCurValue[IDX_CPU_NO] = unCPU;	

		if(vecDuration[IDX_CPU_USER] > 0)
		{
			pstData->vecCurValue[IDX_CPU_USER] = (double) (vecDuration[IDX_CPU_USER] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_USER] += pstData->vecCurValue[IDX_CPU_USER];
			pstData->vecAvgValue[IDX_CPU_USER] 	= pstData->vecSumValue[IDX_CPU_USER] / m_unRunCount;	
		}

		if(vecDuration[IDX_CPU_SYS] > 0)
		{
			pstData->vecCurValue[IDX_CPU_SYS] = (double) (vecDuration[IDX_CPU_SYS] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_SYS] += pstData->vecCurValue[IDX_CPU_SYS];
			pstData->vecAvgValue[IDX_CPU_SYS] 	= pstData->vecSumValue[IDX_CPU_SYS] / m_unRunCount;	
		}

		if(vecDuration[IDX_CPU_NICE] > 0)
		{
			pstData->vecCurValue[IDX_CPU_NICE] = (double) (vecDuration[IDX_CPU_NICE] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_NICE] += pstData->vecCurValue[IDX_CPU_NICE];
			pstData->vecAvgValue[IDX_CPU_NICE]	= pstData->vecSumValue[IDX_CPU_NICE] / m_unRunCount;	
		}

		if(vecDuration[IDX_CPU_IDLE] > 0)
		{
			pstData->vecCurValue[IDX_CPU_IDLE] = (double) (vecDuration[IDX_CPU_IDLE] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_IDLE] += pstData->vecCurValue[IDX_CPU_IDLE];
			pstData->vecAvgValue[IDX_CPU_IDLE] 	= pstData->vecSumValue[IDX_CPU_IDLE] / m_unRunCount;	
		}

		if(vecDuration[IDX_CPU_WAIT] > 0)
		{
			pstData->vecCurValue[IDX_CPU_WAIT] = (double) (vecDuration[IDX_CPU_WAIT] * 100 / (double) unTotal);
			pstData->vecSumValue[IDX_CPU_WAIT] += pstData->vecCurValue[IDX_CPU_WAIT];
			pstData->vecAvgValue[IDX_CPU_WAIT]	= pstData->vecSumValue[IDX_CPU_WAIT] / m_unRunCount;	
		}
			
		pstData->vecCurValue[IDX_CPU_USAGE] = (double) (100 - pstData->vecCurValue[IDX_CPU_IDLE]);
		pstData->vecSumValue[IDX_CPU_USAGE] += pstData->vecCurValue[IDX_CPU_USAGE];
		pstData->vecAvgValue[IDX_CPU_USAGE]	= pstData->vecSumValue[IDX_CPU_USAGE] / m_unRunCount;

//Move to Total CPU
#if 0
			if( (pstData->vecAvgValue[IDX_CPU_USAGE] >= m_dThreshold) && (m_dThreshold != 0) )
			{
				time(&tCur);
				m_bPrintTop = true;
				PrintTopData(tCur);
			}
#endif
        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_CPU_OVER,                                 
                                pstRsc->szName, 
                                Rounding(pstData->vecAvgValue[IDX_CPU_USAGE], 2), 
                                NULL, NULL
                             );


		m_pclsLog->DEBUG("NEW CPU NO %u/%u, Usage %.2f/%u, User %.2f/%u, Nice %.2f/%u, Sys %.2f/%u, Idle %.2f/%u, Wait %.2f/%u",
						unCPU,
						pstData->vecOldValue[IDX_CPU_NO],
						pstData->vecCurValue[IDX_CPU_USAGE]	,
						pstData->vecOldValue[IDX_CPU_USAGE],
						pstData->vecCurValue[IDX_CPU_USER]	,
						pstData->vecOldValue[IDX_CPU_USER],
						pstData->vecCurValue[IDX_CPU_NICE]	,
						pstData->vecOldValue[IDX_CPU_NICE],
						pstData->vecCurValue[IDX_CPU_SYS]	,
						pstData->vecOldValue[IDX_CPU_SYS],
						pstData->vecCurValue[IDX_CPU_IDLE]	,
						pstData->vecOldValue[IDX_CPU_IDLE],
						pstData->vecCurValue[IDX_CPU_WAIT]	,
						pstData->vecOldValue[IDX_CPU_WAIT]
						);
	
		vecTop.push_back(pstRsc);
	}

	fclose(fp);

//Move to Total CPU
#if 0
	InsertTopData();
#endif

	//Plus Run Count
	m_unRunCount++;

	return 0;
}

extern "C"
{
	RSABase *PLUG0001_CPU()
	{
		return new CPU;
	}
}
