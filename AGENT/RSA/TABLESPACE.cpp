#include "TABLESPACE.hpp"

bool tablespace_sort_function (RESOURCE_ATTR *a_pstSrc, RESOURCE_ATTR *a_pstDst)
{
	DB_VALUE *pstDataSrc = (DB_VALUE*)a_pstSrc->pData;
	DB_VALUE *pstDataDst = (DB_VALUE*)a_pstDst->pData;
	
	return pstDataSrc->vecAvgValue[IDX_DB_USAGE] > pstDataDst->vecAvgValue[IDX_DB_USAGE];
}

TABLESPACE::TABLESPACE()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;
	m_pclsDB = NULL;
    m_unTotalSize = 0;
	vecTop.clear();
	printf("DB is Starting\n");
}


TABLESPACE::~TABLESPACE()
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
			delete (DB_VALUE*)pstAttr->pData;
	}

}

int TABLESPACE::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	DB_VALUE *pstMemData = NULL;	

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
			it->second->pData = (void*)new DB_VALUE;
			pstMemData = (DB_VALUE*)it->second->pData;
			pstMemData->vecAvgValue.assign(MAX_DB_IDX, 0);
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_DB_IDX);

	}	

	m_pclsDB = m_pclsMain->GetDBConn();
	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("DB Class is NULL");
		return -1;
	}
#if 0
    int ret = 0;
    char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char szVariableName[DEF_MEM_BUF_128];
	char szVariableValue[DEF_MEM_BUF_128];

	FetchMaria fData;
    fData.Set(szVariableName, sizeof(szVariableName));
    fData.Set(szVariableValue, sizeof(szVariableValue));

    sprintf(szBuffer, "show variables where Variable_name ='innodb_data_file_path'");
    ret = m_pclsDB->Query(&fData, szBuffer, strlen(szBuffer));
    if(ret < 0)
    {
        m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
        return -1;
    }

    if(fData.Fetch() == false)
    {
        return -1;
    }

    m_pclsLog->DEBUG("DB Variable Name : %s, VariableValue : %s",
                    szVariableName, szVariableValue);

    m_unTotalSize = GetDBFileSize(szVariableValue);
    if(m_unTotalSize == 0)
    {
        m_pclsLog->ERROR("DB File Size Parsing Error");
        return -1;
    }

    m_pclsLog->DEBUG("DB TOTAL SIZE %u", m_unTotalSize);
#endif

	return 0;
}

int TABLESPACE::InsertTopData()
{
	int ret = 0;
	char szBuffer[DEF_MEM_BUF_1024];
	DB_VALUE *pstData = NULL;

	snprintf(szBuffer, sizeof(szBuffer), "DELETE FROM TAT_RSC_TOP WHERE NODE_NO = '%d' AND RSC_GRP_ID = '%s'"
							, m_pclsMain->GetNodeID()
							, m_pGroupRsc->szGroupID
			);

	if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0 )
	{
		m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}	

	std::sort(vecTop.begin(), vecTop.end(), tablespace_sort_function);
	int i = 0;
	for(std::vector<RESOURCE_ATTR*>::iterator it=vecTop.begin(); it != vecTop.end() && i < DEF_MAX_TOP_COUNT; ++it, i++)
	{
		pstData = (DB_VALUE*)(*it)->pData;

		snprintf(szBuffer, sizeof(szBuffer), "INSERT INTO TAT_RSC_TOP VALUES ('%s', '%s', '%d', sysdate(), '%.2f')"
												, m_pGroupRsc->szGroupID
												, (*it)->szID
												, m_pclsMain->GetNodeID()
												, pstData->vecAvgValue[IDX_DB_USAGE]
				);		

		if( (ret=m_pclsDB->Execute(szBuffer, strlen(szBuffer))) <= 0 )
		{
			m_pclsLog->ERROR("Execute Fail [%d] [%s] [%d:%s]", ret, szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
			return -1;
		}	
	}
	return 0;
}

int TABLESPACE::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DB_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (DB_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_DB_IDX ; i++)
			{
				sprintf(szBuff, "%.2f", pstData->vecAvgValue[i]);
				o_rscAttr[DB_COLUMN[i]] = szBuff;
			}
		
		}


	    m_pGroupRsc->strSummaryJson.clear();

		if(vecTop.size() != 0)
		{
    		rabbit::object o_rootSummary;
    		o_rootSummary["NAME"] = m_pGroupRsc->szGroupName;

	    	std::vector<RESOURCE_ATTR*>::iterator vecIt = vecTop.begin();

    		pstRsc = (*vecIt);
    		pstData = (DB_VALUE*)pstRsc->pData;

			o_rootSummary["LIST"] = pstRsc->szName;
    		snprintf(szBuff, sizeof(szBuff), "%.2f", pstData->vecAvgValue[IDX_DB_USAGE]);
	    	o_rootSummary[DB_COLUMN[IDX_DB_USAGE]] = szBuff;

			m_pGroupRsc->unExec |= DEF_EXEC_SET_SUMMARY;
			m_pGroupRsc->strSummaryJson.assign(o_rootSummary.str());
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());

		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("DB MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("DB MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("DB MakeJson Parsing Error");
        return -1;
    }

	return 0;
}

int TABLESPACE::MakeStatJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DB_VALUE *pstData = NULL;

	CStmResRspApi encApi;
	CStmResRspApi decApi;
	std::string strData;

	encApi.Init(1, CStmResApi::RESAPI_RESULT_CODE_SUCCESS);
	
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (DB_VALUE*)it->second->pData;
		for(int i = 0; i < MAX_DB_IDX ; i++)
		{
			sprintf(szBuff, "%d", i+1);
			encApi[pstRsc->szID][szBuff].Init(a_tCur, m_pclsMain->GetNodeID(), Rounding(pstData->vecAvgValue[i], 2));
		}

	}

	encApi.EncodeMessage(strData);
	m_pGroupRsc->strStatJson.assign(strData);

	return 0;

}

int TABLESPACE::GetDBConnInfo(char *a_pszArgs, char **a_pszConnInfo)
{
	char * p = NULL;
	int nIdx = 0;
	p = strtok( a_pszArgs, " ");

	if(p == NULL)
		return -1;

	a_pszConnInfo[nIdx++] = p;

	while( NULL != (p = strtok( NULL, " ")) )
	{
		a_pszConnInfo[nIdx++] = p;
	}

	return 0;
}

uint64_t TABLESPACE::ParsingFileSize(char *a_pszFileConf)
{
	char szBuff[DEF_MEM_BUF_128];
	memset(szBuff, 0x00, sizeof(szBuff));
	sprintf(szBuff, "%s", a_pszFileConf);	

	char *p = NULL;
	uint64_t unSize = 0;

	p = strtok ( szBuff, ":");
	if(p == NULL)
		return -1;	

	p = strtok(NULL, ":");
	switch( p[strlen(p)-1] )
	{
		case 'K':
			p[strlen(p)-1] = 0x00;
			unSize = atoi(p);
			unSize >>= 10;
			break;
		case 'M':
			p[strlen(p)-1] = 0x00;
			unSize = atoi(p);
			break;
		case 'G':
			p[strlen(p)-1] = 0x00;
			unSize = atoi(p);
			unSize <<= 10;
			break;
		default:
			return 0;
	}

	return unSize;
}

uint64_t TABLESPACE::GetDBFileSize(char *a_pszValue)
{
	char *p = NULL;
	uint64_t unTotalSize = 0;	
	uint64_t ret = 0;

	char *arrStr[DEF_MAX_FILE_COUNT];
	int nIdx = 0;

	p = strtok( a_pszValue,";");
	if(p == NULL)
		return 0;

	arrStr[nIdx++] = p;
	
	while( NULL != (p = strtok(NULL, ";")) )
	{
		arrStr[nIdx++] = p;
		if(nIdx >= DEF_MAX_FILE_COUNT)
		{
			m_pclsLog->ERROR("DB Max File Count is Over");
			return 0;
		}
	}

	for(int i = 0; i < nIdx ; i++)
	{
		ret = ParsingFileSize(arrStr[i]);
		if(ret == 0)
		{
			m_pclsLog->ERROR("DB File Size Parsing Error\n");
			return 0;
		}
		unTotalSize += ret;
	}

	return unTotalSize;

}

int TABLESPACE::Run()
{
	char szBuffer[DEF_MEM_BUF_1024];

	int ret = 0;
    char szUsedSize[DEF_MEM_BUF_64];
    char szFreeSize[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DB_VALUE *pstData = NULL;

    FetchMaria fData;

	vecTop.clear();

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (DB_VALUE*)it->second->pData;

		sprintf(szBuffer, "SELECT "\
							"SUM(data_length + index_length) / 1024 / 1024 \"Size(MB)\"," \
							"data_free / 1024 / 1024 \"FreeSize(MB)\" " \
							"FROM information_schema.TABLES " \
							"WHERE TABLE_SCHEMA='%s'", pstRsc->szArgs);

        m_pclsLog->DEBUG("QUERY : %s", szBuffer);

		memset(szUsedSize, 0x00, sizeof(szUsedSize));
		memset(szFreeSize, 0x00, sizeof(szFreeSize));

		fData.Clear();
		fData.Set(szUsedSize, sizeof(szUsedSize));
		fData.Set(szFreeSize, sizeof(szFreeSize));

		ret = m_pclsDB->Query(&fData, szBuffer, strlen(szBuffer));	
		if(ret < 0)
		{
			m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
			return -1;
		}


		if(fData.Fetch() == false)
		{
			return -1;
		}

        m_pclsLog->DEBUG("szUsedSize %s, szFreeSize %s", szUsedSize, szFreeSize);
	
		m_unTotalSize = atof(szUsedSize) + atof(szFreeSize);
		pstData->vecAvgValue[IDX_DB_TOTAL] 	= m_unTotalSize ;
		pstData->vecAvgValue[IDX_DB_USED] 	= atof(szUsedSize);
		pstData->vecAvgValue[IDX_DB_FREE] 	= atof(szFreeSize);
//				pstData->vecAvgValue[IDX_DB_TOTAL] - pstData->vecAvgValue[IDX_DB_USED];
		pstData->vecAvgValue[IDX_DB_USAGE] 	= 
				(pstData->vecAvgValue[IDX_DB_USED] / pstData->vecAvgValue[IDX_DB_TOTAL]) * 100;


        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_TABLESPACE_OVER, 
                                pstRsc->szName, 
                                Rounding(pstData->vecAvgValue[IDX_DB_USAGE], 2), 
                                NULL, NULL
                             );

		m_pclsLog->DEBUG("NEW DB Usage %.2f, Total %.2f, Free %.2f, Used %.2f",
						pstData->vecAvgValue[IDX_DB_USAGE]	,
						pstData->vecAvgValue[IDX_DB_TOTAL]	,
						pstData->vecAvgValue[IDX_DB_FREE]	,
						pstData->vecAvgValue[IDX_DB_USED]	
						);
		
		
		vecTop.push_back(pstRsc);

	}

	InsertTopData();

	return 0;
}

extern "C"
{

	RSABase *PLUG0006_DB()
	{
		return new TABLESPACE;
	}
}
