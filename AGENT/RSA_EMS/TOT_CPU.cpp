#include "TOT_CPU.hpp"

TOT_CPU::TOT_CPU()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_unRunCount = 1;
	m_pclsMain = NULL;

	printf("TOT_CPUis Starting\n");
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

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_CPU_IDX);

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
					sprintf(szBuff, "%.0f", pstData->vecAvgValue[i]);
				else
					sprintf(szBuff, "%.2f", pstData->vecAvgValue[i]);
	
				o_rscAttr[CPU_COLUMN[i]] = szBuff;
			}

			//Insert Summary Info
			snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[IDX_CPU_USAGE]);
			o_rootSummary[CPU_COLUMN[IDX_CPU_USAGE]] = szBuff;
		}
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
			sprintf(szBuff, "%d", i);
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


int TOT_CPU::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
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

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (TOT_CPU_VALUE*)it->second->pData;
		sprintf(szBuffer, "cat /proc/stat | grep %s", pstRsc->szArgs);
		
		if( (fp = popen(szBuffer, "r")) == NULL )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			return -1;
		}

		if(fgets(szBuffer, sizeof(szBuffer), fp))
		{
			unCPU = 0;
			unUser = 0;
			unNice = 0;
			unSys = 0;
			unIdle = 0;
			unWait = 0;
			unHardIRQ = 0;
			unSoftIRQ = 0;

			sscanf(szBuffer + 5, "%lu %lu %lu %lu %lu %lu %lu",
					&unUser,
					&unNice,
					&unSys,
					&unIdle,
					&unWait,
					&unHardIRQ,
					&unSoftIRQ
			);
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
				pclose(fp);
				return -1;
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


            m_pclsEvent->SendTrap(
                                    DEF_ALM_CODE_TOTAL_CPU_OVER, 
                                    pstRsc->szName, 
                                    Rounding(pstData->vecAvgValue[IDX_CPU_USAGE], 2), 
                                    NULL, NULL
                                 );

			m_pclsLog->DEBUG("NEW TOTAL_CPU NO %u, Usage %.2f, User %.2f, Nice %.2f, Sys %.2f, Idle %.2f, Wait %.2f",
							unCPU,
							pstData->vecAvgValue[IDX_CPU_USAGE]	,
							pstData->vecAvgValue[IDX_CPU_USER]	,
							pstData->vecAvgValue[IDX_CPU_NICE]	,
							pstData->vecAvgValue[IDX_CPU_SYS]	,
							pstData->vecAvgValue[IDX_CPU_IDLE]	,
							pstData->vecAvgValue[IDX_CPU_WAIT]	
							);
		
		}
		
		pclose(fp);
	}

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
