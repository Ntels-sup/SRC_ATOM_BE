#include "PING.hpp"

PING::PING()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsMain = NULL;
	m_pclsEvent = NULL;
	m_strMyIP.clear();

}


PING::~PING()
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
			delete (PING_VALUE*)pstAttr->pData;
	}

}

int PING::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	RESOURCE_ATTR *pstAttr = NULL;
	PING_VALUE *pstPingData = NULL;	

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
/*		
		if(pstAttr->pData == NULL)
		{
			//Init Stat Data
			it->second->pData = (void*)new PING_VALUE;
			pstPingData = (PING_VALUE*)it->second->pData;
			GetPeerInfo(pstPingData->vecPeerInfo);
//			pstPingData->vecStringValue.assign(MAX_PING_IDX, "");
		}
*/
		// ALWAYS SELECT EDIT _ cjr 20160610
		it->second->pData = (void*)new PING_VALUE;
		pstPingData = (PING_VALUE*)it->second->pData;
		pstPingData->vecPeerInfo.erase(pstPingData->vecPeerInfo.begin(), pstPingData->vecPeerInfo.end() );
		GetPeerInfo(pstPingData->vecPeerInfo);

		m_pclsLog->INFO("Name : %s, Args : %s", 
			pstAttr->szName, pstAttr->szArgs);

	}	

	return 0;
}

int PING::GetPeerInfo(std::vector<PEER_INFO> &a_vecPeerInfo)
{
	int ret = 0;
	char szBuff[DEF_MEM_BUF_1024];
	char szQuery[DEF_MEM_BUF_1024];
	char szImageNo[DEF_MEM_BUF_64];
	char szIP[DEF_MEM_BUF_64];
	char szPeerNo[DEF_MEM_BUF_64];
	char szPeerName[DEF_MEM_BUF_64];
	char szPkgName[DEF_MEM_BUF_64];
	char szPeerType[DEF_MEM_BUF_64];

	DB *pclsDB = m_pclsMain->GetDBConn();
	if(NULL == pclsDB)
	{
		m_pclsLog->ERROR("Fail to DB Conncnect in PING Checker");
		return -1;
	}
	
	FetchMaria fData;
	fData.Set(szIP, sizeof(szIP));
	fData.Set(szImageNo, sizeof(szImageNo));

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), 
			"SELECT IP, IMAGE_NO FROM TAT_NODE "
			"WHERE "
			"NODE_NO='%d'"
			, m_pclsMain->GetNodeID()
			);
	m_pclsLog->DEBUG("Query : %s", szQuery);
	
	ret = pclsDB->Query(&fData, szQuery, strlen(szQuery));
	if(ret < 0)
	{
		m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, pclsDB->GetErrorMsg(ret));
		return -1;
	}


	if(fData.Fetch() == false)
		return -1;

	m_strMyIP.assign(szIP);	

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery),
				"SELECT A.TARGET_ID AS IMAGE_NO "
				"FROM TAT_LINE_DEF A, TAT_NODE_GUI B "
				"WHERE "
				"A.TARGET_ID = B.IMAGE_NO "
				"AND A.SOURCE_TYPE='NO' "
				"AND A.SOURCE_ID='%s' "
				"UNION ALL "
				"SELECT A.SOURCE_ID AS IMAGE_NO "
				"FROM TAT_LINE_DEF A, TAT_NODE_GUI B "
				"WHERE "
				"A.SOURCE_ID = B.IMAGE_NO "
				"AND A.TARGET_TYPE='NO' "
				"AND A.TARGET_ID='%s'"
				, szImageNo
				, szImageNo
			);	
	m_pclsLog->DEBUG("Query : %s", szQuery);

	fData.Clear();
	fData.Set(szImageNo, sizeof(szImageNo));

	ret = pclsDB->Query(&fData, szQuery, strlen(szQuery));
	if(ret < 0)
	{
		m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, pclsDB->GetErrorMsg(ret));
		return -1;
	}

	if( false == fData.Fetch() )
		return 0;

	memset(szBuff, 0x00, sizeof(szBuff));
	snprintf(szBuff, sizeof(szBuff), "'%s'", szImageNo);

	while(true)
	{
		if( false == fData.Fetch() )
			break;
		strcat(szBuff, ", '");
		strcat(szBuff, szImageNo);
		strcat(szBuff, "'");
	}
	
	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery),
				"SELECT NODE_NAME, NODE_NO, IP, PKG_NAME, NODE_TYPE FROM TAT_NODE WHERE IMAGE_NO IN (%s) and USE_YN='Y'", szBuff);

	m_pclsLog->DEBUG("Query : %s", szQuery);

	fData.Clear();

	fData.Set(szPeerName, sizeof(szPeerName));
	fData.Set(szPeerNo, sizeof(szPeerNo));
	fData.Set(szIP, sizeof(szIP));
	fData.Set(szPkgName, sizeof(szPkgName));
	fData.Set(szPeerType, sizeof(szPeerType));

	ret = pclsDB->Query(&fData, szQuery, strlen(szQuery));
	if(ret < 0)
	{
		m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, pclsDB->GetErrorMsg(ret));
		return -1;
	}

	PEER_INFO 	stPeerInfo;

	while(true)
	{
		if( false == fData.Fetch() )
			break;

//		memset(&stPeerInfo, 0x00, sizeof(PEER_INFO));

		stPeerInfo.strPeerName.assign(szPeerName);
		stPeerInfo.strPeerIP.assign(szIP);
		stPeerInfo.nPeerNo = atoi(szPeerNo);
		stPeerInfo.nStatus = DEF_PEER_STATUS_INIT;
		stPeerInfo.strPeerType.assign(szPeerType);
		stPeerInfo.strPkgName.assign(szPkgName);
		a_vecPeerInfo.push_back(stPeerInfo);

	}

	for(uint32_t i = 0; i < a_vecPeerInfo.size(); i ++)
	{
		m_pclsLog->INFO("[%d], Peer %s, PeerIP %s", i, a_vecPeerInfo[i].strPeerName.c_str(), a_vecPeerInfo[i].strPeerIP.c_str());
	}

	return 0;
}

int PING::MakeNodeJson(const char *a_szPkgName, const char *a_szPeerType, std::string &a_strResult)
{
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PING_VALUE *pstData = NULL;

	a_strResult.clear();
	try{
		rabbit::object o_root;
		rabbit::object o_body = o_root["BODY"];
		rabbit::object o_peer;
		o_body["pkg_name"] = a_szPkgName;
		o_body["node_type"] = a_szPeerType;
		rabbit::array a_nodeList = o_body["node_list"];

		for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
		{
			pstRsc = it->second;
			pstData = (PING_VALUE*)it->second->pData;
			for(uint32_t i = 0; i < pstData->vecPeerInfo.size(); i ++)
			{
				if(pstData->vecPeerInfo[i].strPeerType.compare(a_szPeerType) != 0)
				{
					continue;
				}

				a_nodeList.push_back(StringRef(pstData->vecPeerInfo[i].strPeerName.c_str()));
				o_peer = o_body[pstData->vecPeerInfo[i].strPeerName.c_str()];
				o_peer["ip_addr"] = pstData->vecPeerInfo[i].strPeerIP.c_str();
			}
		}

		a_strResult.assign(o_root.str());

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("PING MakeNodeJson, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("PING MakeNodeJson , %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("PING MakeNodeJson Parsing Error");
        return -1;
    }

	return 0;
}

int PING::MakePingJson(std::string &a_strResult)
{
	a_strResult.clear();
	CCliRsp encApi;

    char cStatus = 0;
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PING_VALUE *pstData = NULL;

    encApi.NPrintf(1024, "     NODE_NAME            IP         STATUS\n");
    encApi.NPrintf(1024, "------------------- ---------------- ------\n");

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (PING_VALUE*)it->second->pData;

		for(uint32_t i = 0; i < pstData->vecPeerInfo.size(); i ++)
		{
			if(pstData->vecPeerInfo[i].nStatus == DEF_PEER_STATUS_FAIL)
                cStatus = 'N';
            else if(pstData->vecPeerInfo[i].nStatus == DEF_PEER_STATUS_SUCC)
                cStatus = 'Y';

			encApi.NPrintf(1024, "  %-17s   %-14s    %c\n",
                                    pstData->vecPeerInfo[i].strPeerName.c_str(),
                                    pstData->vecPeerInfo[i].strPeerIP.c_str(),
                                    cStatus
                                    );
		}
	
	}

	encApi.EncodeMessage(a_strResult);

	return 0;
}

int PING::MakeJson(time_t a_tCur)
{
	return 0;
#if 0
	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PING_VALUE *pstData = NULL;

	rabbit::object o_root;
	o_root["NAME"] = m_pGroupRsc->szGroupName;
	rabbit::array a_rscList = o_root["LIST"];

	rabbit::object o_rscAttr;
	
	m_pGroupRsc->strFullJson.clear();

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (PING_VALUE*)it->second->pData;
		a_rscList.push_back(StringRef(pstRsc->szName));

		o_rscAttr = o_root[pstRsc->szName];

		for(int i = 0; i < MAX_PING_IDX ; i++)
		{
			o_rscAttr[PING_COLUMN[i]] = pstData->vecStringValue[IDX_PING_STATUS].c_str();
		}
	}
	m_pGroupRsc->strFullJson.assign(o_root.str());
	m_pGroupRsc->unExec |= DEF_EXEC_SET_FULL;
#endif
	return 0;
}


int PING::Run()
{
	FILE *fp;
	char szBuffer[DEF_MEM_BUF_1024];

	//unsigned Stat
    int ret = 0;
    int nTxPacket = 0;
    int nRxPacket = 0;
	int	nLossRate = 0;

	map<string, RESOURCE_ATTR *>::iterator it;
	RESOURCE_ATTR *pstRsc = NULL;
	PING_VALUE *pstData = NULL;

	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		pstRsc = it->second;
		pstData = (PING_VALUE*)it->second->pData;

		for(uint32_t i = 0; i < pstData->vecPeerInfo.size(); i ++)
		{
//			snprintf(szBuffer, sizeof(szBuffer), "ping -t 1 -W 1 -c 1 %s", pstData->vecPeerInfo[i].strPeerIP.c_str());
			snprintf(szBuffer, sizeof(szBuffer), "ping -W 1 -c 1 %s", pstData->vecPeerInfo[i].strPeerIP.c_str());
			m_pclsLog->DEBUG("ping Command  [%s]", szBuffer);

			if( (fp = popen(szBuffer, "r")) == NULL)
			{
				m_pclsLog->ERROR("poepn() error (%s)", szBuffer);
				return -1;
			}
//			m_pclsLog->INFO("2222222222222222");
//			fgets(szBuffer, sizeof(szBuffer), fp);
//			m_pclsLog->INFO("333333333333333333333333");

			while(fgets(szBuffer, sizeof(szBuffer), fp) != NULL)
			{
				m_pclsLog->DEBUG("szBuffer %s", szBuffer);
				if(strstr(szBuffer, "PING Statistics") != NULL 
					|| strstr(szBuffer, "ping statistics") != NULL)
				{
					if( (fgets(szBuffer, sizeof(szBuffer), fp) == NULL ) )
					{
						 break;
					}
				
					ret = sscanf(szBuffer, "%d packets transmitted, %d packets received"
											, &nTxPacket, &nRxPacket);
			
					if(ret != 2)
					{
						m_pclsLog->ERROR("Wrong Command Result %d : %s", ret, szBuffer);
						continue;
					}	

					nLossRate = ((nTxPacket - nRxPacket) * 100) / nTxPacket;
				}
				else if (strstr(szBuffer, "unknown host") != NULL)
				{
					nLossRate = 100;
				}
			}

			if(nLossRate == 100)
			{
				if( DEF_PEER_STATUS_FAIL != pstData->vecPeerInfo[i].nStatus )
				{
					m_pclsEvent->PingFail(
											pstData->vecPeerInfo[i].nPeerNo,
											m_strMyIP.c_str(),
											pstData->vecPeerInfo[i].strPeerIP.c_str()
										);
				}
	
				pstData->vecPeerInfo[i].nStatus = DEF_PEER_STATUS_FAIL;
			}
			else
			{
				if( DEF_PEER_STATUS_SUCC != pstData->vecPeerInfo[i].nStatus )
				{
					m_pclsEvent->PingSuccess(
											pstData->vecPeerInfo[i].nPeerNo,
											m_strMyIP.c_str(),
											pstData->vecPeerInfo[i].strPeerIP.c_str()
										);
				}
	
				pstData->vecPeerInfo[i].nStatus = DEF_PEER_STATUS_SUCC;
			}

			m_pclsLog->DEBUG("PING_RESULT, PeerNo %d, PeerIP %s, Result, %s",
						pstData->vecPeerInfo[i].nPeerNo,
						pstData->vecPeerInfo[i].strPeerIP.c_str(),
						pstData->vecPeerInfo[i].nStatus == DEF_PEER_STATUS_FAIL ? "[FAIL]" : "[SUCC]"
						);
			pclose(fp);
		}

		
	}


	return 0;
}

extern "C"
{

	RSABase *PLUG0017_PING()
	{
		return new PING;
	}
}
