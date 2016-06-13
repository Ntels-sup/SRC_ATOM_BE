#include "EX_DISK.hpp"

EX_DISK::EX_DISK()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


EX_DISK::~EX_DISK()
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
			delete (EX_DISK_VALUE*)pstAttr->pData;
	}

}

int EX_DISK::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	EX_DISK_VALUE *pstData = NULL;

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
			it->second->pData = (void*)new EX_DISK_VALUE;
			pstData = (EX_DISK_VALUE*)it->second->pData;
			pstData->vecStringValue.assign(MAX_EX_DISK_IDX, "");
		}
		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_EX_DISK_IDX);

	}	
	return 0;
}

int EX_DISK::MakeTrapJson()
{
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    EX_DISK_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;

	    m_pGroupRsc->strRootTrapJson.clear();

    	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	    {
    	    pstRsc = it->second;
	        pstData = (EX_DISK_VALUE*)it->second->pData;

        	o_rscAttr = o_root[pstRsc->szName];

    	    o_rscAttr["CODE"] = DEF_ALM_CODE_EX_DISK_HALT;
	        o_rscAttr["TARGET"] = pstRsc->szName;
        	o_rscAttr["VALUE"] = pstData->vecStringValue[IDX_EX_DISK_STATUS].c_str();

    	}

	    m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("EX_DISK MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("EX_DISK MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("EX_DISK MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}


int EX_DISK::MakeJson(time_t a_tCur)
{
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	EX_DISK_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (EX_DISK_VALUE*)pstRsc->pData;
		
			if( NULL == pstData )
			{
				m_pmapRsc->erase(it);
				continue;
			}

			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_EX_DISK_IDX ; i++)
			{
				o_rscAttr[EX_DISK_COLUMN[i]] = pstData->vecStringValue[i].c_str();
	
			}
		
		}
		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("EX_DISK MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("EX_DISK MakeJson, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("EX_DISK MakeJsonParsing Error");
        return -1;
    }

    MakeTrapJson();

	return 0;
}


int EX_DISK::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
	char szMount[DEF_MEM_BUF_64];
	char szDevice[DEF_MEM_BUF_64];
	char szFileSystem[DEF_MEM_BUF_64];
	char szDummy[20][DEF_MEM_BUF_64];

	char szStatus[DEF_MEM_BUF_64];
	char szUtil[DEF_MEM_BUF_64];
//	char sz[DEF_MEM_BUF_64];
//	char szStatus[DEF_MEM_BUF_64];

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	EX_DISK_VALUE *pstData = NULL;

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; ++it)
	{
		memset(szDevice, 0x00, sizeof(szDevice));
		memset(szFileSystem, 0x00, sizeof(szFileSystem));
		memset(szUtil, 0x00, sizeof(szUtil));
		memset(szStatus, 0x00, sizeof(szStatus));
        szStatus[0] = 'N';

		pstRsc = it->second;
		pstData = (EX_DISK_VALUE*)it->second->pData;

		sprintf(szBuffer, "cat /etc/mtab | grep %s", pstRsc->szArgs);
		if( (fp = popen(szBuffer, "r")) == NULL )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			return -1;
		}

		if( NULL != fgets(szBuffer, sizeof(szBuffer), fp) )
		{

			sscanf( szBuffer, "%s %s %s %s %s %s"
							, szMount
							, szDevice
							, szFileSystem
							, szDummy[0]
							, szDummy[1]
							, szDummy[2]
					);

		}
		else 
		{
			m_pclsLog->ERROR("EX_DISK mtab Read Failed");
			pclose(fp);
			return -1;
		}

		pclose(fp);

		sprintf(szBuffer, "iostat -dxN | grep %s", pstRsc->szArgs);
		if( NULL == (fp = popen(szBuffer, "r")) )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			pclose(fp);
			return -1;
		}

		if( NULL != fgets(szBuffer, sizeof(szBuffer), fp) )
		{
			sscanf( szBuffer, "%s %s %s %s %s %s %s %s %s %s %s %s"
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szDummy[0]
								, szUtil
					);

			szStatus[0] = 'Y';
		}
		else
		{
			m_pclsLog->ERROR("EX_DISK iostat Read Failed");
			pclose(fp);
			return -1;
		}		

		pclose(fp);

		m_pclsLog->DEBUG("DISK %s, Status %s, Device %s, FileSystem %s, Util %s"
								, pstRsc->szArgs
								, szStatus
								, szDevice
								, szFileSystem
								, szUtil
						);

		pstData->vecStringValue[IDX_EX_DISK_STATUS].assign(szStatus);		
		pstData->vecStringValue[IDX_EX_DISK_DEVICE].assign(szDevice);		
		pstData->vecStringValue[IDX_EX_DISK_FILE_SYSTEM].assign(szFileSystem);		
		pstData->vecStringValue[IDX_EX_DISK_UTIL].assign(szUtil);		

	}

	return 0;
}

extern "C"
{
	RSABase *PLUG0014_EX_DISK()
	{
		return new EX_DISK;
	}
}
