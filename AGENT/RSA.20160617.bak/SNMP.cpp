#include "SNMP.hpp"

SNMP::SNMP()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


SNMP::~SNMP()
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
			delete (SNMP_VALUE*)pstAttr->pData;
	}

}

int SNMP::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	SNMP_VALUE *pstTempData = NULL;	

	if(a_pclsLog == NULL)
		return -1;

	m_pclsLog = a_pclsLog;
	m_pGroupRsc = a_pRsc;
	m_pmapRsc = &(a_pRsc->mapRsc);	
	m_pclsMain = (RSARoot*)a_pclsMain;
	
	CConfig *pclsConfig = m_pclsMain->GetConfig();
	m_strTrapLogPath.assign(pclsConfig->GetConfigValue("RSA", "SNMP_TRAP_LOG"));

	if( 0 == m_strTrapLogPath.size() )
	{
		m_pclsLog->ERROR("SNMP TRAP LOG PATH is Null");
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
			it->second->pData = (void*)new SNMP_VALUE;
			pstTempData = (SNMP_VALUE*)it->second->pData;
			pstTempData->bFind = false;
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_SNMP_IDX);

	}	

	return 0;
}

int SNMP::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    SNMP_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];

	    m_pGroupRsc->strRootTrapJson.clear();

	    for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
    	{
	        pstRsc = it->second;
        	pstData = (SNMP_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

    	    o_rscAttr = o_root[pstRsc->szName];

	        o_rscAttr["CODE"] = DEF_ALM_CODE_EXTERNAL_TRAP;
    	    o_rscAttr["TARGET"] = pstRsc->szName;

    	    snprintf(szBuff, sizeof(szBuff), "%c", pstData->bFind ? 'Y' : 'N' );
	        pstData->bFind = false;
	        o_rscAttr["VALUE"] = szBuff;

    	}

	    m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("SNMP MakeTrap , %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("SNMP MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("SNMP MakeTrap Parsing Error");
        return -1;
    }
 
    return 0;
}

int SNMP::MakeJson(time_t a_tCur)
{
	return 0;
#if 0
	char szBuff[DEF_MEM_BUF_128];	
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	SNMP_VALUE *pstData = NULL;

	rabbit::object o_root;
	o_root["NAME"] = m_pGroupRsc->szGroupName;
	rabbit::array a_rscList = o_root["LIST"];

	rabbit::object o_rscAttr;
	
	
	m_pGroupRsc->strFullJson.clear();

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (SNMP_VALUE*)it->second->pData;
		a_rscList.push_back(StringRef(pstRsc->szName));

		o_rscAttr = o_root[pstRsc->szName];

		for(int i = 0; i < MAX_SNMP_IDX ; i++)
		{
			sprintf(szBuff, "%.2f", pstData->vecCurValue[i]);
			o_rscAttr[SNMP_COLUMN[i]] = szBuff;

		}
	}

	m_pGroupRsc->strFullJson.assign(o_root.str());
	m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
#endif
	return 0;
}

int SNMP::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	SNMP_VALUE *pstData = NULL;

	bool bFind = false;
	int ret = 0;
	int nSeverity = 0;
	int nMessageCode = 0;
	int nNodeId = 0;
	char szDetails[DEF_MEM_BUF_256];


	sprintf(szBuffer, "%s", m_strTrapLogPath.c_str());
		
	if (access(szBuffer, F_OK) == -1)
	{
		m_pclsLog->WARNING("There is no Trap Log File (%s) ", szBuffer);
		return -1;
	}

	if( (fp = fopen(szBuffer, "r")) == NULL )
	{
		m_pclsLog->ERROR("fopen() error (%s)", szBuffer);
		return -1;
	}


	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		bFind = false;

		m_pclsLog->DEBUG("szBuffer %s\n", szBuffer);

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (SNMP_VALUE*)it->second->pData;
	
			if (strstr(szBuffer, pstRsc->szArgs) == NULL)
				continue;

			pstData->bFind = true;
			bFind = true;
			break;
		}

		if( false == bFind )
			continue;

		ret = sscanf(szBuffer,
			"%*s %d %d %d %[^\n]s", &nSeverity, &nMessageCode, &nNodeId, szDetails);
	
#if 0
		if (ret == 4)		
		{
			switch (nSeverity)
			{
				case 1: // critical
                    m_pclsEvent->SendTrap(DEF_ALM_CODE_EXTERNAL_TRAP, "ThreePar", "CRITICAL", NULL, NULL);
                    break;
				case 2: // major
                    m_pclsEvent->SendTrap(DEF_ALM_CODE_EXTERNAL_TRAP, "ThreePar", "MAJOR", "NULL", "NULL");
                    break;
				case 3: // minor
                    m_pclsEvent->SendTrap(DEF_ALM_CODE_EXTERNAL_TRAP, "ThreePar", "MINOR", "NULL", "NULL");
                    break;
				case 4: // clear
					break;
				default: // unknown
					m_pclsLog->WARNING("Unknown severity(%d): %s", nSeverity, szBuffer);
					break;
			}
		}
		else
		{
			m_pclsLog->WARNING("Input format wrong: Count(%d) Buffer(%s)", ret, szBuffer);
			continue;
		}
#endif
		m_pclsLog->DEBUG("nSeverity %d, nMessageCode %d, nNodeId %d, szDetails %s", 
							nSeverity, nMessageCode, nNodeId, szDetails);
	}

	fclose(fp);
    truncate(m_strTrapLogPath.c_str(), 0); 

	return 0;
}

extern "C"
{
	RSABase *PLUG0018_SNMP()
	{
		return new SNMP;
	}
}
