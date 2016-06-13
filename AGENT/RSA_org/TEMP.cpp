#include "TEMP.hpp"

TEMP::TEMP()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_unRunCount = 1;
	m_pclsMain = NULL;

	printf("TEMPis Starting\n");
}


TEMP::~TEMP()
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
			delete (TEMP_VALUE*)pstAttr->pData;
	}

}

int TEMP::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	TEMP_VALUE *pstTempData = NULL;	

	if(a_pclsLog == NULL)
		return -1;

	m_pclsLog = a_pclsLog;
	m_pGroupRsc = a_pRsc;
	m_pmapRsc = &(a_pRsc->mapRsc);	
	m_pclsMain = (RSARoot*)a_pclsMain;

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
			it->second->pData = (void*)new TEMP_VALUE;
			pstTempData = (TEMP_VALUE*)it->second->pData;
			pstTempData->vecSumValue.assign(MAX_TEMP_IDX, 0);
			pstTempData->vecAvgValue.assign(MAX_TEMP_IDX, 0);
			pstTempData->vecCurValue.assign(MAX_TEMP_IDX, 0);
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_TEMP_IDX);

	}	
	return 0;
}

int TEMP::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];   
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    TEMP_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];
    
	    m_pGroupRsc->strRootTrapJson.clear();

    	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	    {
        	pstRsc = it->second;
    	    pstData = (TEMP_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

	        o_rscAttr = o_root[pstRsc->szName];

        	o_rscAttr["CODE"] = DEF_ALM_CODE_CPU_TEMP_OVER;
    	    o_rscAttr["TARGET"] = pstRsc->szName;

			snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[IDX_TEMP_AVG]);
        	o_rscAttr["VALUE"] = szBuff;

    	}

	    m_pGroupRsc->strRootTrapJson.assign(o_root.str());

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("TEMP MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("TEMP MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("TEMP MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}


int TEMP::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];	
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	TEMP_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (TEMP_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_TEMP_IDX ; i++)
			{
//				sprintf(szBuff, "%.2f", pstData->vecAvgValue[i]);
				sprintf(szBuff, "%.2f", pstData->vecCurValue[i]);
				o_rscAttr[TEMP_COLUMN[i]] = szBuff;

			}
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("ProcessCLICmd, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("ProcessCLICmd, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("ProcessCLICmd Parsing Error");
        return -1;
    }

    MakeTrapJson();

	return 0;
}

int TEMP::MakeStatJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	TEMP_VALUE *pstData = NULL;

	CStmResRspApi encApi;
	CStmResRspApi decApi;
	std::string strData;

	encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (TEMP_VALUE*)it->second->pData;
		for(int i = 0; i < MAX_TEMP_IDX ; i++)
		{
			sprintf(szBuff, "%d", i+1);
			encApi[pstRsc->szID][szBuff].Init(a_tCur, m_pclsMain->GetNodeID(), Rounding(pstData->vecAvgValue[i], 2));
		}

		pstData->vecAvgValue.assign(MAX_TEMP_IDX, 0);
		pstData->vecSumValue.assign(MAX_TEMP_IDX, 0);
		pstData->vecCurValue.assign(MAX_TEMP_IDX, 0);
		m_unRunCount = 1;

	}

	encApi.EncodeMessage(strData);
	m_pGroupRsc->strStatJson.assign(strData);

	return 0;

}

int TEMP::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	uint64_t	unSensor = 0;
	uint64_t	unCPU = 0;
	uint64_t	unTempC = 0;
	uint64_t	unTempF = 0;
	char		szThreshold[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	TEMP_VALUE *pstData = NULL;

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (TEMP_VALUE*)it->second->pData;

		sprintf(szBuffer, "/sbin/hpasmcli -s 'show temp' | grep \\^# | grep %s | tr -d '#'", pstRsc->szArgs);
		
		if( (fp = popen(szBuffer, "r")) == NULL )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			return -1;
		}

		while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
		{
			printf("szBuffer %s\n", szBuffer);
//			sscanf(szBuffer, "%[0-9] CPU%[0-9] %[0-9.]C/%[0-9.]F %s",
			sscanf(szBuffer, "%lu CPU%lu %luC/%luF %s",
							&unSensor, &unCPU, &unTempC, &unTempF, szThreshold);
		}


		m_pclsLog->DEBUG("TEMP CPU %u, Celsius %u",
						unCPU, unTempC);

		
		pstData->vecAvgValue[IDX_TEMP_CPU_NO] = unCPU;
		pstData->vecCurValue[IDX_TEMP_CPU_NO] = unCPU;

		pstData->vecCurValue[IDX_TEMP_AVG] = unTempC;		
		pstData->vecSumValue[IDX_TEMP_AVG] += pstData->vecCurValue[IDX_TEMP_AVG];
		pstData->vecAvgValue[IDX_TEMP_AVG] = (double)pstData->vecSumValue[IDX_TEMP_AVG] / m_unRunCount;		

		if(pstData->vecCurValue[IDX_TEMP_MAX] < unTempC)
		{
			pstData->vecCurValue[IDX_TEMP_MAX] = unTempC;
			pstData->vecAvgValue[IDX_TEMP_MAX] = unTempC;
		}

		if( 0 == pstData->vecCurValue[IDX_TEMP_MIN] )
		{
			pstData->vecCurValue[IDX_TEMP_MIN] = unTempC;			
			pstData->vecAvgValue[IDX_TEMP_MIN] = unTempC;			
		}
			
		if(pstData->vecCurValue[IDX_TEMP_MIN] > unTempC)
		{
			pstData->vecCurValue[IDX_TEMP_MIN] = unTempC;			
			pstData->vecAvgValue[IDX_TEMP_MIN] = unTempC;			
		}

		m_pclsLog->DEBUG("TEMP CPU %.2f, Avg %.2f, Max %.2f, Min %.2f",
						pstData->vecCurValue[IDX_TEMP_CPU_NO], pstData->vecCurValue[IDX_TEMP_AVG], 
						pstData->vecCurValue[IDX_TEMP_MAX],  pstData->vecCurValue[IDX_TEMP_MIN]);

		pclose(fp);
	}

	m_unRunCount++;
	return 0;
}

extern "C"
{
	RSABase *PLUG0003_TEMP()
	{
		return new TEMP;
	}
}
