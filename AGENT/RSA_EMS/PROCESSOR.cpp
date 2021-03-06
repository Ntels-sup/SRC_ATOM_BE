#include "PROCESSOR.hpp"

PROCESSOR::PROCESSOR()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


PROCESSOR::~PROCESSOR()
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
			delete (PROCESSOR_VALUE*)pstAttr->pData;
	}

}

int PROCESSOR::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	PROCESSOR_VALUE *pstProcData = NULL;	

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
			it->second->pData = (void*)new PROCESSOR_VALUE;
			pstProcData = (PROCESSOR_VALUE*)it->second->pData;
			pstProcData->vecCharValue.assign(MAX_PROCESSOR_STATUS_IDX, 0);
		}
	
		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_PROCESSOR_STATUS_IDX);

	}	
	return 0;
}

int PROCESSOR::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    PROCESSOR_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
	    rabbit::object o_rscAttr;

    	m_pGroupRsc->strRootTrapJson.clear();

	    for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
    	{
        	pstRsc = it->second;
	        pstData = (PROCESSOR_VALUE*)it->second->pData;

    	    o_rscAttr = o_root[pstRsc->szName];

        	o_rscAttr["CODE"] = DEF_ALM_CODE_PROCESSOR_HALT;
    	    o_rscAttr["TARGET"] = pstRsc->szName;
	
	        snprintf(szBuff, sizeof(szBuff), "%c", pstData->vecCharValue[IDX_PROCESSOR_STATUS_NO]);
    	    o_rscAttr["VALUE"] = szBuff;

	    }

    	m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("PROCESSOR MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("PROCESSOR MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("PROCESSOR MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}

int PROCESSOR::MakeJson(time_t a_tCur)
{
	char szBuff[DEF_MEM_BUF_128];
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PROCESSOR_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (PROCESSOR_VALUE*)pstRsc->pData;
				
			if( NULL == pstData )
			{
				m_pmapRsc->erase(it);
				continue;
			}

			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_PROCESSOR_STATUS_IDX ; i++)
			{
				sprintf(szBuff, "%c", pstData->vecCharValue[i]);
				o_rscAttr[PROCESSOR_STATUS_COLUMN[i]] = szBuff;

			}
		
		}

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("PROCESSOR MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("PROCESSOR MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("PROCESSOR MakeJson Parsing Error");
        return -1;
    }

    MakeTrapJson();

	return 0;
}


int PROCESSOR::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char szProcessor[DEF_MEM_BUF_64];
	char szName[DEF_MEM_BUF_64];
	char szCore[DEF_MEM_BUF_64];
	char szThread[DEF_MEM_BUF_64];
	char szSocket[DEF_MEM_BUF_64];
	char szStatus[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PROCESSOR_VALUE *pstData = NULL;
	std::pair<map<string, RESOURCE_ATTR *>::iterator, bool> ret_RSC;	
	std::pair<map<string, string>::iterator, bool> ret_ATTR;	

	const char *command =
			"/sbin/hpasmcli -s 'show server' | awk 'BEGIN{ OFS=\"\"; }"
			"/\\Processor: /{printf \"%s \",$2} "
			"/Name.*:/{$1=\"\";$2=\"\";printf \"\\\"%s\\\" \",$0} "
			"/Core.*:/{printf \"%s \",$3} "
			"/Thread.*:/{printf \"%s \",$3} "
			"/Socket.*:/{printf \"%s \",$3} "
			"/Status.*:/{printf \"%s\\n\",$3}'";



	if( (fp = popen(command, "r")) == NULL )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}

	while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
	{
		sscanf( szBuffer, "%[0-9] \"%[^\"]\" %[0-9] %[0-9] %[0-9] %s"
						, szProcessor
						, szName
						, szCore
						, szThread
						, szSocket
						, szStatus
				);

		if( strcasecmp(szStatus, "Ok") )
		{
			szStatus[IDX_PROCESSOR_STATUS_NO] = 'N';
			szStatus[MAX_PROCESSOR_STATUS_IDX] = 0x00;
		}
		else
		{
			szStatus[IDX_PROCESSOR_STATUS_NO] = 'Y';
			szStatus[MAX_PROCESSOR_STATUS_IDX] = 0x00;
		}

		pstData = NULL;
		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			
			if( 0 == strncmp(szProcessor, pstRsc->szArgs, strlen(szProcessor)) )
			{
				pstData = (PROCESSOR_VALUE*)it->second->pData;
				break;
			}
		}

		if( NULL != pstData )
		{
			pstData->vecCharValue[IDX_PROCESSOR_STATUS_NO] = szStatus[IDX_PROCESSOR_STATUS_NO];
		}

	}

	pclose(fp);

	return 0;
}

extern "C"
{
	RSABase *PLUG0010_PROCESSOR()
	{
		return new PROCESSOR;
	}
}
