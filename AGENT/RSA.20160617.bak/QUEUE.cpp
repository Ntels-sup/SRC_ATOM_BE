#include "QUEUE.hpp"

QUEUE::QUEUE()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsCLQ = NULL;
	m_pclsMain = NULL;
	m_pclsConfig = NULL;

}


QUEUE::~QUEUE()
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
            delete (QUEUE_VALUE*)pstAttr->pData;
    }

//	if(m_pclsCLQ)
//		delete m_pclsCLQ;	
}

int QUEUE::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;

	if(a_pclsLog == NULL)
		return -1;

	m_pclsLog = a_pclsLog;
	m_pGroupRsc = a_pRsc;
	m_pmapRsc = &(a_pRsc->mapRsc);	
	m_pclsMain = (RSARoot*)a_pclsMain;
//    m_pclsEvent = m_pclsMain->GetEvent();

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

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_QUEUE_IDX);

	}	

#if 1
	m_pclsConfig = m_pclsMain->GetConfig();

	if(m_pclsConfig == NULL)
	{
		m_pclsLog->ERROR("Get Config Class Error");
		return -1;
	}
	
	setenv("ATOM_HOME", m_pclsConfig->GetConfigValue("RSA", "ATOMQ_HOME_PATH"), 1);
	m_pclsCLQ = new CLQManager(m_pclsMain->GetPkgName(), m_pclsMain->GetNodeType(), NULL, 0, 0, 0, NULL);

	if(m_pclsCLQ->Initialize(DEF_CMD_TYPE_UTIL) < 0)
	{
		m_pclsLog->ERROR("CLQManager Init Failed");
		return -1;
	}
#endif
	if(m_pclsCLQ == NULL)
	{
		m_pclsLog->ERROR("CLQManager Pointer is NULL");
		return -1;
	}

	return 0;
}

int QUEUE::MakeJson(time_t a_tCur)
{
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	QUEUE_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];
	
		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;

			pstData = (QUEUE_VALUE*)pstRsc->pData;
			if( NULL == pstData )
			{
				m_pmapRsc->erase(it);
				continue;
			}

			a_rscList.push_back(StringRef(pstRsc->szName));
			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_QUEUE_IDX ; i++)
			{
				o_rscAttr[QUEUE_COLUMN[i]] = pstData->vecStringValue[i].c_str();
			}
		
		}
		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("QUEUE MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("QUEUE MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("QUEUE MakeJson Parsing Error");
        return -1;
    }

	MakeTrapJson();
	return 0;
}

int QUEUE::MakeTrapJson()
{
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    QUEUE_VALUE *pstData = NULL;

    try {
        rabbit::object o_root;
        rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];

        m_pGroupRsc->strRootTrapJson.clear();

        for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
        {
            pstRsc = it->second;
            pstData = (QUEUE_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

            o_rscAttr = o_root[pstRsc->szName];

            o_rscAttr["CODE"] = DEF_ALM_CODE_QUEUE_OVER;
            o_rscAttr["TARGET"] = pstRsc->szName;
            o_rscAttr["VALUE"] = pstData->vecStringValue[IDX_QUEUE_USAGE].c_str();

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

int QUEUE::Run()
{
	char szBuffer[DEF_MEM_BUF_128];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	QUEUE_VALUE *pstData = NULL;

	float fUsage = 0;
	struct rte_ring *arrRing[RTE_MAX_MEMZONE];
	memset(arrRing, 0x00, sizeof(arrRing));

	m_pclsCLQ->GetRingList(arrRing);
	for(int i = 1; i < RTE_MAX_MEMZONE ; i++)
	{
		if(arrRing[i] == NULL)
			break;
		
		if( strncmp(arrRing[i]->name, "MP", 2) == 0 )
		{
			fUsage = rte_ring_free_count(arrRing[i]) / (float)arrRing[i]->prod.size * 100;
			m_pclsLog->DEBUG("Memory Pool : %25s  / Usage %3.2f", arrRing[i]->name, fUsage);
		}
		else
		{
			fUsage = rte_ring_count(arrRing[i]) / (float)arrRing[i]->prod.size * 100;
			m_pclsLog->DEBUG("Queue       : %25s  / Usage %3.2f", arrRing[i]->name, fUsage);
		}
		
		snprintf(szBuffer, sizeof(szBuffer), "%.2f", fUsage);
		it = m_pmapRsc->find( arrRing[i]->name );
		if(it != m_pmapRsc->end())
		{
			pstRsc = it->second;					
			pstData = (QUEUE_VALUE*)pstRsc->pData;
			pstData->vecStringValue[IDX_QUEUE_USAGE].assign(szBuffer);
		}
		else
		{
			pstRsc = new RESOURCE_ATTR;
			memset(pstRsc, 0x00, sizeof(RESOURCE_ATTR));
			pstRsc->pData = (void*)new QUEUE_VALUE;
			pstData = (QUEUE_VALUE*)pstRsc->pData;
			snprintf(pstRsc->szName, sizeof(pstRsc->szName), "%s", arrRing[i]->name);

			pstData->vecStringValue.assign(MAX_QUEUE_IDX, "");
			pstData->vecStringValue[IDX_QUEUE_USAGE].assign(szBuffer);
			m_pmapRsc->insert( std::pair<string, RESOURCE_ATTR*>(arrRing[i]->name, pstRsc) );
			
		}

#if 0
        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_QUEUE_OVER, 
                                pstRsc->szName, 
                                Rounding(fUsage,2), 
                                NULL, NULL
                             );
#endif
	}

		

	return 0;
}

extern "C"
{
	RSABase *PLUG0016_QUEUE()
	{
		return new QUEUE;
	}

}
