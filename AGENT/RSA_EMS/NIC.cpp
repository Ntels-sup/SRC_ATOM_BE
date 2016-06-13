#include "NIC.hpp"

NIC::NIC()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;

}


NIC::~NIC()
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
			delete (NIC_VALUE*)pstAttr->pData;
	}

}

int NIC::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	NIC_VALUE *pstMemData = NULL;	

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
			it->second->pData = (void*)new NIC_VALUE;
			pstMemData = (NIC_VALUE*)it->second->pData;
			pstMemData->vecOldValue.assign(MAX_NIC_IDX, 0);
			pstMemData->vecNewValue.assign(MAX_NIC_IDX, "");

			SetIPAddress(pstAttr);
		}

		m_pclsLog->INFO("Name : %s, Args : %s, IPv4: %s, IPv6 : %s", 
			pstAttr->szName, pstAttr->szArgs, pstMemData->szIPv4, pstMemData->szIPv6);

	}	
	return 0;
}

int NIC::SetIPAddress(RESOURCE_ATTR *a_pstResource)
{
	NIC_VALUE *pstData = NULL;

	struct ifreq ifr;
	struct sockaddr_in *sin;
	int sockfd;

	if( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		m_pclsLog->ERROR("Failed to Open Socket");
		return -1;
	}

	memset(&ifr, 0x00, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, a_pstResource->szArgs);
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
	{
		m_pclsLog->ERROR("NIC (%s) ioctl(SIOCGIFHWADDR) error", a_pstResource->szArgs);
		return -1;
	}

	pstData = (NIC_VALUE*)a_pstResource->pData;
	memset(pstData->szIPv4, 0x00, sizeof(pstData->szIPv4));
	memset(pstData->szIPv6, 0x00, sizeof(pstData->szIPv6));

	sin = (struct sockaddr_in*)&ifr.ifr_addr;
	snprintf(pstData->szIPv4, sizeof(pstData->szIPv4), "%s", inet_ntoa(sin->sin_addr));

	FILE *fp = NULL;

	char szIP[INET6_ADDRSTRLEN];
	char szIndex[DEF_MEM_BUF_32];
	char szPrefix[DEF_MEM_BUF_32];
	char szScope[DEF_MEM_BUF_32];
	char szFlags[DEF_MEM_BUF_32];
	char szName[DEF_MEM_BUF_32];

	char szBuffer[DEF_MEM_BUF_256];
	unsigned char szVal[INET6_ADDRSTRLEN];
	char *p = szIP;
	size_t	nCount = 0;

	snprintf(szBuffer, sizeof(szBuffer), "cat /proc/net/if_inet6 | grep %s", a_pstResource->szArgs);

	if( (fp = popen(szBuffer, "r")) == NULL )
	{
		m_pclsLog->ERROR("Failed to popen Cmd(%s)", szBuffer);
		return -1;
	}

	if( NULL == fgets(szBuffer, sizeof(szBuffer), fp) )
	{
		return 0;
	}

	sscanf(szBuffer, "%s %s %s %s %s %s", szIP, szIndex, szPrefix, szScope, szFlags, szName);

	for(nCount = 0; nCount < sizeof(szVal)/sizeof(szVal[0]); nCount++)
	{
		sscanf(p, "%2hhx", &szVal[nCount]);
		p += 2;
	}

	if( NULL != inet_ntop(AF_INET6,  szVal, szIP, INET6_ADDRSTRLEN))
		snprintf(pstData->szIPv6, sizeof(pstData->szIPv6), "%s", szIP);

	return 0;
}

int NIC::MakeIPJson(const char *a_szIFName, const char *a_szVersion, std::string &a_strResult)
{
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	NIC_VALUE *pstData = NULL;

	a_strResult.clear();
	try {
		rabbit::document doc;
		rabbit::object o_body = doc["BODY"];
		o_body["nic"] = a_szIFName;
	    o_body["ip_version"] = a_szVersion;	
		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (NIC_VALUE*)it->second->pData;
			if( strncmp(pstRsc->szArgs, a_szIFName, strlen(a_szIFName)) == 0 )
			{
				switch(atoi(a_szVersion))
				{
					case 4:
						o_body["ip_addr"] = pstData->szIPv4;
						break;
					case 6:
						o_body["ip_addr"] = pstData->szIPv6;
						break;
					default:
						break;
				}
			}
		}

		a_strResult.assign(doc.str());		

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("NIC MakeIPJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("NIC MakeIPJson , %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("NIC MakeIPJson Parsing Error");
        return -1;
    } 

	return 0;

}

int NIC::MakeJson(time_t a_tCur)
{
	bool bStatus = true;

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	NIC_VALUE *pstData = NULL;

	try {
		rabbit::object o_root;
		o_root["NAME"] = m_pGroupRsc->szGroupName;
		rabbit::array a_rscList = o_root["LIST"];

		rabbit::object o_rscAttr;
	
		m_pGroupRsc->strFullJson.clear();
		m_pGroupRsc->strSummaryJson.clear();

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (NIC_VALUE*)it->second->pData;
			a_rscList.push_back(StringRef(pstRsc->szName));

			o_rscAttr = o_root[pstRsc->szName];

			for(int i = 0; i < MAX_NIC_IDX; i++)
			{
				o_rscAttr[NIC_COLUMN[i]] = pstData->vecNewValue[i].c_str();
			}

			if(pstData->vecNewValue[IDX_NIC_STATUS].c_str()[0] == 'N')
				bStatus = false;

		}

#if 1
		rabbit::object o_rootSummary;
		o_rootSummary["NAME"] = m_pGroupRsc->szGroupName;
		o_rootSummary["LIST"] = "NIC";
		o_rootSummary["status"] = bStatus ? "Y" : "N";
//	m_pGroupRsc->strSummaryJson.clear();
//		a_rscListSummary.push_back(StringRef(pstRsc->szName));
//		o_rscAttrSummary = o_rootSummary[pstRsc->szName];
//		o_rscAttrSummary[NIC_COLUMN[IDX_NIC_STATUS]] = pstData->vecNewValue[IDX_NIC_STATUS].c_str();
		m_pGroupRsc->strSummaryJson.assign(o_rootSummary.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_SUMMARY;
#endif

		m_pGroupRsc->strFullJson.assign(o_root.str());
		m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("NIC MakeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("NIC MakeJson , %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("NIC MakeJson Parsing Error");
        return -1;
    } 

	return 0;
}


int NIC::Run()
{
	FILE *fp ;
	char szBuffer[DEF_MEM_BUF_1024];

	char	szIfFlag[DEF_MEM_BUF_64];	
	char	szStatus[DEF_MEM_BUF_64];	

	char	szRXByte[DEF_MEM_BUF_64];	
	char	szRXCnt[DEF_MEM_BUF_64];	
	char	szRXErr[DEF_MEM_BUF_64];	
	char	szRXDrop[DEF_MEM_BUF_64];	
	char	szRXFifo[DEF_MEM_BUF_64];	
	char	szRXFrame[DEF_MEM_BUF_64];	
	char	szRXComp[DEF_MEM_BUF_64];	
	char	szRXMulti[DEF_MEM_BUF_64];	
	
	char	szTXByte[DEF_MEM_BUF_64];	
	char	szTXCnt[DEF_MEM_BUF_64];	
	char	szTXErr[DEF_MEM_BUF_64];	
	char	szTXDrop[DEF_MEM_BUF_64];	
	char	szTXFifo[DEF_MEM_BUF_64];	
	char	szTXFrame[DEF_MEM_BUF_64];	
	char	szTXComp[DEF_MEM_BUF_64];	
	char	szTXMulti[DEF_MEM_BUF_64];	
	
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	NIC_VALUE *pstData = NULL;

	struct ifreq ifr;
	int sockfd;

	char *p;

	uint64_t	unRxByte = 0;
	uint64_t	unTxByte = 0;

	if( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		m_pclsLog->ERROR("Failed to Open Socket");
		return -1;
	}

	
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (NIC_VALUE*)it->second->pData;

		memset(szIfFlag, 0x00, sizeof(szIfFlag));
		memset(szStatus, 0x00, sizeof(szStatus));

		memset(szRXByte, 0x00, sizeof(szRXByte));
		memset(szRXCnt, 0x00, sizeof(szRXCnt));
		memset(szRXErr, 0x00, sizeof(szRXErr));
		memset(szRXDrop, 0x00, sizeof(szRXDrop));
		memset(szRXFifo, 0x00, sizeof(szRXFifo));
		memset(szRXFrame, 0x00, sizeof(szRXFrame));
		memset(szRXComp, 0x00, sizeof(szRXComp));
		memset(szRXMulti, 0x00, sizeof(szRXMulti));

		memset(szTXByte, 0x00, sizeof(szTXByte));
		memset(szTXCnt, 0x00, sizeof(szTXCnt));
		memset(szTXErr, 0x00, sizeof(szTXErr));
		memset(szTXDrop, 0x00, sizeof(szTXDrop));
		memset(szTXFifo, 0x00, sizeof(szTXFifo));
		memset(szTXFrame, 0x00, sizeof(szTXFrame));
		memset(szTXComp, 0x00, sizeof(szTXComp));
		memset(szTXMulti, 0x00, sizeof(szTXMulti));

		unRxByte = 0;
		unTxByte = 0;

		memset(&ifr, 0x00, sizeof(struct ifreq));
		strcpy(ifr.ifr_name, pstRsc->szArgs);

//			if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
//			{
//				m_pclsLog->WARNING("ioctl(SIOCGIFHWADDR) error");
//				continue;
//			}
			
		if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
		{
			m_pclsLog->WARNING("ioctl(SIOCGIFFLAGS) error");
			continue;
		}

		if(ifr.ifr_flags & IFF_LOOPBACK)
		{
			m_pclsLog->DEBUG("iface(%s) = loopback", pstRsc->szArgs);
			continue;
		}

		if (!(ifr.ifr_flags & IFF_BROADCAST) && !(ifr.ifr_flags & IFF_MULTICAST))
		{
			m_pclsLog->DEBUG("iface(%s) = !bcast, !mcast", pstRsc->szArgs);
			continue;
		}

		if(ifr.ifr_flags & IFF_MASTER)
		{
			szIfFlag[0] = 'M';
		}
		else if(ifr.ifr_flags & IFF_SLAVE)
		{
			szIfFlag[0] = 'S';
		}
		else
		{
			szIfFlag[0] = 'N';
		}

		if(ifr.ifr_flags & IFF_RUNNING)
		{
			szStatus[0] = 'Y';
		}
		else
		{
			szStatus[0] = 'N';
		}

		snprintf(szBuffer, sizeof(szBuffer), "cat /proc/net/dev | grep %s" , pstRsc->szArgs);

		if( NULL == (fp = popen(szBuffer, "r")) )
		{
			m_pclsLog->ERROR("popen() error (%s)", szBuffer);
			continue;
		}

		while( NULL != fgets(szBuffer, sizeof(szBuffer), fp) )
		{
			//Find Start Position of Stat
			p = strchr(szBuffer, ':');
			p++;

			sscanf(p, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s"
							,szRXByte
							,szRXCnt
							,szRXErr
							,szRXDrop
							,szRXFifo
							,szRXFrame
							,szRXComp
							,szRXMulti
							,szTXByte
							,szTXCnt
							,szTXErr
							,szTXDrop
							,szTXFifo
							,szTXFrame
							,szTXComp
							,szTXMulti
					);

			
		}

		unRxByte = strtoull(szRXByte, &p, 10);
		unTxByte = strtoull(szTXByte, &p, 10);

		snprintf(szRXByte, sizeof(szRXByte), "%llu", (unsigned long long)(unRxByte - pstData->vecOldValue[IDX_RX_BYTE]));
		snprintf(szTXByte, sizeof(szTXByte), "%llu", (unsigned long long)(unTxByte - pstData->vecOldValue[IDX_TX_BYTE]));
		m_pclsLog->DEBUG("NIC(%s) Status %s, OldRX Byte %llu, OldTX Byte %llu,  RX Byte %s, TX Byte %s"
							,pstRsc->szArgs
							,pstData->vecNewValue[IDX_NIC_STATUS].c_str()
							,pstData->vecOldValue[IDX_RX_BYTE]
							,pstData->vecOldValue[IDX_TX_BYTE]
							,szRXByte
							,szTXByte
					);

        m_pclsEvent->SendTrap(
                                DEF_ALM_CODE_ETHERNET_DOWN, 
                                pstRsc->szName, 
                                szStatus, 
                                NULL, NULL
                             );

		pstData->vecNewValue[IDX_NIC_STATUS].assign(szStatus);
		pstData->vecNewValue[IDX_RX_BYTE].assign(szRXByte);
		pstData->vecNewValue[IDX_TX_BYTE].assign(szTXByte);

		pstData->vecOldValue[IDX_RX_BYTE] = unRxByte;
		pstData->vecOldValue[IDX_TX_BYTE] = unTxByte;

		pclose(fp);	
	}

	return 0;
}

extern "C"
{
	RSABase *PLUG0011_NIC()
	{
		return new NIC;
	}
}
