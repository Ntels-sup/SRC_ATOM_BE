#include "FAN.hpp"

FAN::FAN()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


FAN::~FAN()
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
			delete (FAN_VALUE*)pstAttr->pData;
	}

}

int FAN::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	FAN_VALUE *pstFanData = NULL;

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
			it->second->pData = (void*)new FAN_VALUE;
			pstFanData = (FAN_VALUE*)it->second->pData;
			pstFanData->vecCharValue.assign(MAX_FAN_STATUS_IDX, 0);
		}
	
		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_FAN_STATUS_IDX);

	}	
	return 0;
}

int FAN::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    FAN_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;

	    m_pGroupRsc->strRootTrapJson.clear();

    	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	    {
    	    pstRsc = it->second;
        	pstData = (FAN_VALUE*)it->second->pData;

	        o_rscAttr = o_root[pstRsc->szName];

    	    o_rscAttr["CODE"] = DEF_ALM_CODE_FAN_HALT; 
        	o_rscAttr["TARGET"] = pstRsc->szName;

	        snprintf(szBuff, sizeof(szBuff), "%c", pstData->vecCharValue[IDX_FAN_STATUS_NO]);
    	    o_rscAttr["VALUE"] = szBuff;

	    }

    	m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("FAN MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("FAN MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("FAN MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}


int FAN::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	FAN_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (FAN_VALUE*)pstRsc->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_FAN_STATUS_IDX ; i++)
			{
				sprintf(szBuff, "%c", pstData->vecCharValue[i]);
				o_rscAttr[FAN_STATUS_COLUMN[i]] = szBuff;
			}
		
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("FAN MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("FAN MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("FAN MakeJson Parsing Error");
        return -1;
    }

    MakeTrapJson();

	return 0;
}


int FAN::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char szID[DEF_MEM_BUF_64];
	char szLocation[DEF_MEM_BUF_64];
	char szStatus[DEF_MEM_BUF_64];
	char szSpeed[DEF_MEM_BUF_64];
	char szMax[DEF_MEM_BUF_64];
	char szRedundant[DEF_MEM_BUF_64];
	char szPartner[DEF_MEM_BUF_64];
	char szPluggable[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	FAN_VALUE *pstData = NULL;

	sprintf(szBuffer, "/sbin/hpasmcli -s 'show fan' | grep \\^# | tr -d '#' " );

	if( (fp = popen(szBuffer, "r")) == NULL )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}

	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		sscanf( szBuffer, "%[0-9] %s %s %s %[^%]%% %s %s %s"
						, szID
						, szLocation
						, szStatus
						, szSpeed
						, szMax
						, szRedundant
						, szPartner
						, szPluggable
				);

		if( strcasecmp(szStatus, "Yes") )
		{
			szStatus[IDX_FAN_STATUS_NO] = 'N';
		}
		else
		{
			szStatus[IDX_FAN_STATUS_NO] = 'Y';
		}

		pstData = NULL;
		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			
			if( 0 == strncmp(szID, pstRsc->szArgs, strlen(szID)))
			{
				pstData = (FAN_VALUE*)it->second->pData;
				break;
			}
		}

		if( NULL != pstData)
			pstData->vecCharValue[IDX_FAN_STATUS_NO] = szStatus[IDX_FAN_STATUS_NO];

	}

	pclose(fp);


	return 0;
}

extern "C"
{
	RSABase *PLUG0007_FAN()
	{
		return new FAN;
	}
}
