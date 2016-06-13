#include "DIMM.hpp"

DIMM::DIMM()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

	printf("DIMM is Starting\n");
}


DIMM::~DIMM()
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
			delete (DIMM_VALUE*)pstAttr->pData;
	}

}

int DIMM::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	DIMM_VALUE *pstDimmData = NULL;

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
			it->second->pData = (void*)new DIMM_VALUE;
			pstDimmData = (DIMM_VALUE*)it->second->pData;
			pstDimmData->vecStringValue.assign(MAX_DIMM_IDX, "");
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_DIMM_IDX);

	}	
	return 0;
}

int DIMM::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    DIMM_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];

	    m_pGroupRsc->strRootTrapJson.clear();

    	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	    {
    	    pstRsc = it->second;
        	pstData = (DIMM_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

	        o_rscAttr = o_root[pstRsc->szName];

    	    o_rscAttr["CODE"] = DEF_ALM_CODE_DIMM_HALT;
        	o_rscAttr["TARGET"] = pstRsc->szName;

	        snprintf(szBuff, sizeof(szBuff), "%s", pstData->vecStringValue[IDX_DIMM_STATUS].c_str());
    	    o_rscAttr["VALUE"] = szBuff;

	    }

    	m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("DIMM MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("DIMM MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("DIMM MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}


int DIMM::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DIMM_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (DIMM_VALUE*)pstRsc->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_DIMM_IDX ; i++)
			{
				sprintf(szBuff, "%s", pstData->vecStringValue[i].c_str());
				o_rscAttr[DIMM_COLUMN[i]] = szBuff;
			}
		
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("DIMM MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("DIMM MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("DIMM MakeJson Parsing Error");
        return -1;
    }

    MakeTrapJson();

	return 0;
}


int DIMM::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char szProcessor[DEF_MEM_BUF_64];
	char szModule[DEF_MEM_BUF_64];
	char szSize[DEF_MEM_BUF_64];
	char szSpeed[DEF_MEM_BUF_64];
	char szStatus[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	DIMM_VALUE *pstData = NULL;
	std::pair<map<string, RESOURCE_ATTR *>::iterator, bool> ret_RSC;	
	std::pair<map<string, string>::iterator, bool> ret_ATTR;	

	const char *command = 
						"/sbin/hpasmcli -s 'show dimm' | awk ' "\
						"/^Processor #: /{printf \"%s \",$3} "\
						"/^Module #: /{printf \"%s \",$3} "\
						"/^Size: /{printf \"%s \",$2}"\
						"/^Speed: /{printf \"%s \",$2}"\
						"/^Status: /{printf \"%s\\n\",$2}'";


//		snprintf(szBuffer, sizeof(szBuffer), "%s | grep %s", command, pstRsc->szArgs);	

	if( (fp = popen(command, "r")) == NULL )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}

	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		
		sscanf( szBuffer, "%[0-9] %[0-9] %[0-9] %[0-9] %s"
						, szProcessor 
						, szModule
						, szSize
						, szSpeed
						, szStatus
				);

		if( strcasecmp(szStatus, "Ok") & strcasecmp(szStatus, "N/A") )
		{
			szStatus[0] = 'N';
			szStatus[1] = 0x00;
		}
		else
		{
			szStatus[0] = 'Y';
			szStatus[1] = 0x00;
		}


		pstData = NULL;	
		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;

			sprintf(szBuffer, "%s_%02d", szProcessor, atoi(szModule));
			if( 0 == strncmp(szBuffer, pstRsc->szArgs, strlen(szBuffer)) )
			{
				pstData = (DIMM_VALUE*)it->second->pData;
				break;
			}
		}

//		it = m_pmapRsc->find( szBuffer );

		if( NULL != pstData )
		{
			pstData->vecStringValue[IDX_DIMM_NO] = szProcessor;
			pstData->vecStringValue[IDX_DIMM_MODULE] = szModule;
			pstData->vecStringValue[IDX_DIMM_STATUS] = szStatus;
		}
	}

	pclose(fp);

	return 0;
}

extern "C"
{
	RSABase *PLUG0008_DIMM()
	{
		return new DIMM;
	}
}
