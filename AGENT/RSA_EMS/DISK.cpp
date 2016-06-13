#include "DISK.hpp"

bool disk_sort_function (RESOURCE_ATTR *a_pstSrc, RESOURCE_ATTR *a_pstDst)
{
	DISK_VALUE *pstDataSrc = (DISK_VALUE*)a_pstSrc->pData;
	DISK_VALUE *pstDataDst = (DISK_VALUE*)a_pstDst->pData;
	
	return pstDataSrc->vecAvgValue[IDX_DISK_USAGE] > pstDataDst->vecAvgValue[IDX_DISK_USAGE];
}

DISK::DISK()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_unRunCount = 1;
	m_pclsMain = NULL;
	m_pclsDB = NULL;

}


DISK::~DISK()
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
			delete (DISK_VALUE*)pstAttr->pData;
	}

	if(m_pclsDB)
		delete m_pclsDB;
}

int DISK::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	DISK_VALUE *pstDiskData = NULL;	

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
			it->second->pData = (void*)new DISK_VALUE;
			pstDiskData = (DISK_VALUE*)it->second->pData;
			pstDiskData->vecSumValue.assign(MAX_DISK_IDX, 0);
			pstDiskData->vecAvgValue.assign(MAX_DISK_IDX, 0);
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_DISK_IDX);

	}	

	if(ConnectDB() < 0)
	{
		m_pclsLog->ERROR("Fail to Init CPU Module");
		return -1;
	}

	return 0;
}

int DISK::ConnectDB()
{
	if(m_pclsDB)
	{
		delete m_pclsDB;
	}

	m_pclsDB = new (std::nothrow) MariaDB();
	if(m_pclsDB == NULL)
	{
		m_pclsLog->ERROR("Fail to DB Connect in DB Resource Checker");
		return -1;
	}

	if( m_pclsDB->Connect(
							m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_HOST),
							atoi(m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_PORT)),
							m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_USER),
							m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_PASS),
							m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_DB)
						) != true )
	{
		m_pclsLog->ERROR("Failed to DB Connect / host[%s], port[%s], user[%s], pw[%s], db[%s], [%d:%s]"
			, m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_HOST)
			, m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_PORT)
			, m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_USER)
			, m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_PASS)
			, m_pclsMain->GetDBInfo(IDX_DB_CONN_INFO_DB)
			, m_pclsDB->GetError()
			, m_pclsDB->GetErrorMsg()
		);
		delete m_pclsDB;
		m_pclsDB = NULL;
		return -1;
	}

	return 0;
}

int DISK::InsertTopData()
{
	int ret = 0;
	char szBuffer[DEF_MEM_BUF_1024];
	DISK_VALUE *pstData = NULL;

	if(ConnectDB() < 0)
	{
		m_pclsLog->ERROR("Fail to Init CPU Module");
		return -1;
	}

	snprintf(szBuffer, sizeof(szBuffer), "DELETE FROM TAT_RSC_TOP WHERE NODE_NO = '%d' AND RSC_GRP_ID = '%s'"
							, m_pclsMain->GetNodeID()
							, m_pGroupRsc->szGroupID
			);

	if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0 )
	{
		m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, m_pclsDB->GetError(), m_pclsDB->GetErrorMsg());
		delete m_pclsDB;
		m_pclsDB = NULL;
		return -1;
	}	

	std::sort(vecTop.begin(), vecTop.end(), disk_sort_function);
	int i = 0;
	for(std::vector<RESOURCE_ATTR*>::iterator it=vecTop.begin(); it != vecTop.end() && i < DEF_MAX_TOP_COUNT; ++it, i++)
	{
		pstData = (DISK_VALUE*)(*it)->pData;

		snprintf(szBuffer, sizeof(szBuffer), "INSERT INTO TAT_RSC_TOP VALUES ('%s', '%s', '%d', sysdate(), '%.2f')"
												, m_pGroupRsc->szGroupID
												, (*it)->szID
												, m_pclsMain->GetNodeID()
												, pstData->vecAvgValue[IDX_DISK_USAGE]
				);		

		if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) <= 0 )
		{
			m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, m_pclsDB->GetError(), m_pclsDB->GetErrorMsg());
			delete m_pclsDB;
			m_pclsDB = NULL;
			return -1;
		}	
	}
	return 0;
}

int DISK::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DISK_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	

		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (DISK_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_DISK_IDX ; i++)
			{
				snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[i]);
				o_rscAttr[DISK_COLUMN[i]] = szBuff;
			}

		
		}

		if(vecTop.size() != 0)
		{
		    rabbit::object o_rootSummary;
			o_rootSummary["NAME"] = m_pGroupRsc->szGroupName;

		    m_pGroupRsc->strSummaryJson.clear();
   
		    std::vector<RESOURCE_ATTR*>::iterator vecIt = vecTop.begin();

		    pstRsc = (*vecIt);
			pstData = (DISK_VALUE*)pstRsc->pData;

			o_rootSummary["LIST"] = pstRsc->szName;
			snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[IDX_DISK_USAGE]);
			o_rootSummary[DISK_COLUMN[IDX_DISK_USAGE]] = szBuff;
			m_pGroupRsc->strSummaryJson.assign(o_rootSummary.str());
			m_pGroupRsc->unExec |= DEF_EXEC_SET_SUMMARY;
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("DISK MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("DISK MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("DISK MakeJson Parsing Error");
        return -1;
    }

	return 0;
}

int DISK::MakeStatJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DISK_VALUE *pstData = NULL;

	CStmResRspApi encApi;
	CStmResRspApi decApi;
	std::string strData;

	encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);
	
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (DISK_VALUE*)it->second->pData;
		for(int i = 0; i < MAX_DISK_IDX ; i++)
		{
			sprintf(szBuff, "%d", i);
			encApi[pstRsc->szID][szBuff].Init(a_tCur, m_pclsMain->GetNodeID(), Rounding(pstData->vecAvgValue[i], 2));
		}

		pstData->vecAvgValue.assign(MAX_DISK_IDX, 0);
		m_unRunCount = 1;
	}

	encApi.EncodeMessage(strData);
	m_pGroupRsc->strStatJson.assign(strData);

	return 0;

}


int DISK::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char 		szFileName[DEF_MEM_BUF_128];
	uint64_t	unTotal = 0;
	uint64_t	unFree = 0;
	uint64_t	unUsed = 0;
	uint64_t	unUsage = 0;

	char		szTotal[DEF_MEM_BUF_64];
	char		szUsage[DEF_MEM_BUF_64];


	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DISK_VALUE *pstData = NULL;

	vecTop.clear();
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		memset(szTotal, 0x00, sizeof(szTotal));
		memset(szUsage, 0x00, sizeof(szUsage));

		pstRsc = it->second;
		pstData = (DISK_VALUE*)it->second->pData;
		sprintf(szBuffer, "df -m | grep %s", pstRsc->szArgs);

		
		if( (fp = popen(szBuffer, "r")) == NULL )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			return -1;
		}

		while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
		{
//			printf("szBuffer %s\n", szBuffer);
			sscanf(szBuffer, "%s %lu %lu %lu %[^%]",
							szFileName, &unTotal, &unUsed, &unFree, szUsage);
						
		}

		unUsage = strtoul(szUsage, NULL, 10);
						
		pstData->vecAvgValue[IDX_DISK_TOTAL] = unTotal ;
		pstData->vecAvgValue[IDX_DISK_FREE] = unFree ;
		pstData->vecAvgValue[IDX_DISK_USED] = unUsed; 
		pstData->vecAvgValue[IDX_DISK_USAGE] = unUsage ;

        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_DISK_OVER, 
                                pstRsc->szName, 
                                Rounding(pstData->vecAvgValue[IDX_DISK_USAGE], 2), 
                                NULL, NULL
                             );

		m_pclsLog->DEBUG("NEW DISK Usage %.2f, Total %.2f, Free %.2f, Used %.2f",
						pstData->vecAvgValue[IDX_DISK_USAGE]	,
						pstData->vecAvgValue[IDX_DISK_TOTAL]	,
						pstData->vecAvgValue[IDX_DISK_FREE]	,
						pstData->vecAvgValue[IDX_DISK_USED]	
						);

		vecTop.push_back(pstRsc);	

		pclose(fp);
	}

	InsertTopData();

	return 0;
}

extern "C"
{
	RSABase *PLUG0005_DISK()
	{
		return new DISK;
	}
}
