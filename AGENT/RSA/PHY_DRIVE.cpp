#include "PHY_DRIVE.hpp"

PHY_DRIVE::PHY_DRIVE()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


PHY_DRIVE::~PHY_DRIVE()
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
			delete (PHY_DRIVE_VALUE*)pstAttr->pData;
	}

}

int PHY_DRIVE::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	PHY_DRIVE_VALUE *pstPhyData = NULL;
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
	
		if( NULL == pstAttr->pData )
		{
			it->second->pData = (void*)new PHY_DRIVE_VALUE;
			pstPhyData = (PHY_DRIVE_VALUE*)it->second->pData;
			pstPhyData->vecStringValue.assign(MAX_PHY_DRIVE_IDX, "");
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_PHY_DRIVE_IDX);

	}	
	return 0;
}

int PHY_DRIVE::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    PHY_DRIVE_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];

	    m_pGroupRsc->strRootTrapJson.clear();

    	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	    {
    	    pstRsc = it->second;
        	pstData = (PHY_DRIVE_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

	        o_rscAttr = o_root[pstRsc->szName];

    	    o_rscAttr["CODE"] = DEF_ALM_CODE_PHY_DRIVE_HALT;
        	o_rscAttr["TARGET"] = pstRsc->szName;

	        snprintf(szBuff, sizeof(szBuff), "%s", pstData->vecStringValue[IDX_PHY_DRIVE_STATUS].c_str());
    	    o_rscAttr["VALUE"] = szBuff;

	    }

	    m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("PHY_DRIVE MakeTrap , %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("PHY_DRIVE MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("PHY_DRIVE MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}


int PHY_DRIVE::MakeJson(time_t a_tCur)
{
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PHY_DRIVE_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (PHY_DRIVE_VALUE*)pstRsc->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_PHY_DRIVE_IDX ; i++)
			{
				o_rscAttr[PHY_DRIVE_COLUMN[i]] = pstData->vecStringValue[i].c_str();
			}
		
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("PHY_DRIVE MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("PHY_DRIVE MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("PHY_DRIVE MakeJson Parsing Error");
        return -1;
    }

    MakeTrapJson();

	return 0;
}


int PHY_DRIVE::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char szName[DEF_MEM_BUF_64];
	char szPort[DEF_MEM_BUF_64];
	char szBox[DEF_MEM_BUF_64];
	char szBay[DEF_MEM_BUF_64];
	char szStatus[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PHY_DRIVE_VALUE *pstData = NULL;

#if defined(_USE_HPACUCLI)
	sprintf(szBuffer, "/usr/sbin/hpacucli ctrl all show config | grep physicaldrive");
#elif defined(_USE_HPSSACLI)
	sprintf(szBuffer, "/usr/sbin/hpssacli ctrl all show config | grep physicaldrive");
#endif
	if( (fp = popen(szBuffer, "r")) == NULL )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}

	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		memset(szName, 0x00, sizeof(szName));
		memset(szPort, 0x00, sizeof(szPort));
		memset(szBox, 0x00, sizeof(szBox));
		memset(szBay, 0x00, sizeof(szBay));
		memset(szStatus, 0x00, sizeof(szStatus));

		sscanf( szBuffer, "%s %[^I:]I:%[^:]:%[0-9]"
						, szName
						, szPort
						, szBox
						, szBay
				);

		if( 0 == strstr(szBuffer, "OK") )
		{
			szStatus[0] = 'N';
		}
		else
		{
			szStatus[0] = 'Y';
		}

		pstData = NULL;	
		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			
			snprintf(szBuffer, sizeof(szBuffer), "%s_%s_%s", szPort, szBox, szBay);
			if( 0 == strncmp(szBuffer, pstRsc->szArgs, strlen(szBuffer)) )
			{
				pstData = (PHY_DRIVE_VALUE*)it->second->pData;
				break;
			}
		}

		if( NULL != pstData )
		{
			pstData->vecStringValue[IDX_PHY_DRIVE_STATUS].assign(szStatus);
			pstData->vecStringValue[IDX_PHY_DRIVE_PORT].assign(szPort);
			pstData->vecStringValue[IDX_PHY_DRIVE_BOX].assign(szBox);
			pstData->vecStringValue[IDX_PHY_DRIVE_BAY].assign(szBay);

		}

	}

	pclose(fp);

	return 0;
}

extern "C"
{
	RSABase *PLUG0013_PHY_DRIVE()
	{
		return new PHY_DRIVE;
	}
}
