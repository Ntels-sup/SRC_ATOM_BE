#include "BOND.hpp"

BOND::BOND()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


BOND::~BOND()
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
			delete (BOND_VALUE*)pstAttr->pData;
	}

}

int BOND::Initialize(CFileLog *a_pclsLog,  RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	BOND_VALUE *pstMemData = NULL;	

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
			it->second->pData = (void*)new BOND_VALUE;
			pstMemData = (BOND_VALUE*)it->second->pData;
			pstMemData->vecStringValue.assign(MAX_BOND_IDX, "");
		}

		m_pclsLog->INFO("Name : %s, Args : %s, Idx : %d", 
			pstAttr->szName, pstAttr->szArgs, MAX_BOND_IDX);

	}	
	return 0;
}


int BOND::MakeTrapJson()
{
    char szBuff[DEF_MEM_BUF_128];
    map<string, RESOURCE_ATTR *>::iterator it;
    RESOURCE_ATTR *pstRsc = NULL;
    BOND_VALUE *pstData = NULL;

	try {
	    rabbit::object o_root;
    	rabbit::object o_rscAttr;
		rabbit::array a_rscList = o_root["LIST"];

	    m_pGroupRsc->strRootTrapJson.clear();

	    for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
    	{
    	    pstRsc = it->second;
	        pstData = (BOND_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

	        o_rscAttr = o_root[pstRsc->szName];

    	    o_rscAttr["CODE"] = DEF_ALM_CODE_BOND_DOWN;
        	o_rscAttr["TARGET"] = pstRsc->szName;

	        snprintf(szBuff, sizeof(szBuff), "%s", pstData->vecStringValue[IDX_BOND_STATUS].c_str());
    	    o_rscAttr["VALUE"] = szBuff;

	    }

	    m_pGroupRsc->strRootTrapJson.assign(o_root.str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("BOND MakeTrap, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("BOND MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("BOND MakeTrap Parsing Error");
        return -1;
    }

    return 0;
}


int BOND::MakeJson(time_t a_tCur)
{
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	BOND_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (BOND_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_BOND_IDX; i++)
			{
				o_rscAttr[BOND_COLUMN[i]] = pstData->vecStringValue[i].c_str();

			}
		}
		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("BOND MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("BOND MakeTrap, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("BOND MakeTrap Parsing Error");
        return -1;
    }

    MakeTrapJson();
	return 0;
}

int BOND::GetSlaves(BOND_VALUE *a_pstData, char *a_pszIF)
{
	FILE *fp;
	char szBuffer[DEF_MEM_BUF_1024];
	char szFileName[PATH_MAX];

	snprintf(szFileName, sizeof(szFileName), "/sys/class/net/%s/bonding/slaves", a_pszIF);
	if( (fp = fopen(szFileName, "r")) == NULL )
	{
		m_pclsLog->ERROR("slave fopen(%s) (%s) error (%d)", szFileName, a_pszIF, errno);
		return -1;
	}

	if( fgets(szBuffer, sizeof(szBuffer) - 1, fp ) )
	{
		szBuffer[strlen(szBuffer)-1] = 0x00;

		if(strlen(szBuffer))
			a_pstData->vecStringValue[IDX_BOND_SLAVE].assign(szBuffer);
		else
			a_pstData->vecStringValue[IDX_BOND_SLAVE].assign("");

	}

	fclose(fp);
	
	snprintf(szFileName, sizeof(szFileName), "/sys/class/net/%s/bonding/active_slave", a_pszIF);
	if( (fp = fopen(szFileName, "r")) == NULL )
	{
		m_pclsLog->ERROR("active-slave fopen(%s) (%s) error (%d)", szFileName, a_pszIF, errno);
		return -1;
	}

	if( fgets(szBuffer, sizeof(szBuffer) - 1, fp ) )
	{
		szBuffer[strlen(szBuffer)-1] = 0x00;
		
		if(strlen(szBuffer))
			a_pstData->vecStringValue[IDX_BOND_ACTIVE_SLAVE].assign(szBuffer);
		else
			a_pstData->vecStringValue[IDX_BOND_ACTIVE_SLAVE].assign("");

	}

	fclose(fp);

	return 0;
}

int BOND::Run()
{
	char	szStatus[DEF_MEM_BUF_64];	

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	BOND_VALUE *pstData = NULL;

	struct ifreq ifr;
	struct if_nameindex* pidx;
	struct if_nameindex* head;
	int sockfd;

	if( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		return -1;
	}

	head = if_nameindex();
	
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (BOND_VALUE*)it->second->pData;

		memset(szStatus, 0x00, sizeof(szStatus));

		for(pidx = head; (pidx != NULL) && (pidx->if_name != NULL); pidx++)
		{
			if(strncmp(pstRsc->szArgs, pidx->if_name, IFNAMSIZ))
				continue;


			memset(&ifr, 0, sizeof(ifr));

			strncpy(ifr.ifr_name, pidx->if_name, IFNAMSIZ);
		
			if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
			{
				m_pclsLog->WARNING("ioctl(SIOCGIFHWADDR) error");
				continue;
			}
			
			if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
			{
				m_pclsLog->WARNING("ioctl(SIOCGIFFLAGS) error");
				continue;
			}

			if(ifr.ifr_flags & IFF_LOOPBACK)
			{
				m_pclsLog->DEBUG("iface(%s) = loopback", pidx->if_name);
				continue;
			}

			if (!(ifr.ifr_flags & IFF_BROADCAST) && !(ifr.ifr_flags & IFF_MULTICAST))
			{
				m_pclsLog->DEBUG("iface(%s) = !bcast, !mcast", pidx->if_name);
				continue;
			}

			if(ifr.ifr_flags & IFF_MASTER)
			{
				if(ifr.ifr_flags & IFF_RUNNING)
				{
					szStatus[0] = 'Y';
					GetSlaves(pstData, pidx->if_name);
				}
				else
				{
					szStatus[0] = 'N';
				}
			}

			pstData->vecStringValue[IDX_BOND_STATUS].assign(szStatus);

			break;			
		}

		m_pclsLog->DEBUG("BOND(%s) Status %s, Slaves %s, Active Slaves %s"
							,pstRsc->szArgs
							,pstData->vecStringValue[IDX_BOND_STATUS].c_str()
							,pstData->vecStringValue[IDX_BOND_SLAVE].c_str()
							,pstData->vecStringValue[IDX_BOND_ACTIVE_SLAVE].c_str()
					);

	}

    close(sockfd);
	return 0;
}

extern "C"
{
	RSABase *PLUG0012_NIC_BOND()
	{
		return new BOND;
	}
}
