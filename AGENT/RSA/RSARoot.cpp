#include "RSARoot.hpp"

RSARoot::RSARoot(CFileLog *a_pclsLog, CConfig *a_pclsConfig)
{
	m_pclsLog = a_pclsLog;
	m_pclsConfig = a_pclsConfig;
	m_pstRsc = NULL;
    m_pclsClient = NULL;
	m_nGroupCount = 0;
	m_mapRsc.clear();
	m_nNodeID = 0;
	m_pclsDB = NULL;
	memset(m_pszDBInfo, 0x00, sizeof(m_pszDBInfo));
}


RSARoot::~RSARoot()
{
	for(int i = 0; i < m_nGroupCount ; i++)
	{
		if( NULL != m_pstRsc[i].pclsRSA )
		{
			delete (RSABase*)m_pstRsc[i].pclsRSA;
		}

		if( NULL != m_pstRsc[i].pHandle )
		{
			dlclose(m_pstRsc[i].pHandle);
		}
	}


	if(m_pstRsc)
		delete [] m_pstRsc;
}


int RSARoot::Initialize()
{
	m_pclsLog->INFO("RSA Root Initialize Func Start");


	snprintf(m_pszDBInfo[IDX_DB_CONN_INFO_HOST], sizeof(m_pszDBInfo[IDX_DB_CONN_INFO_HOST]), "%s"
							, m_pclsConfig->GetGlobalConfigValue("DB_HOST"));
	snprintf(m_pszDBInfo[IDX_DB_CONN_INFO_PORT], sizeof(m_pszDBInfo[IDX_DB_CONN_INFO_PORT]), "%s"
							, m_pclsConfig->GetGlobalConfigValue("DB_PORT"));
	snprintf(m_pszDBInfo[IDX_DB_CONN_INFO_USER], sizeof(m_pszDBInfo[IDX_DB_CONN_INFO_USER]), "%s"
							, m_pclsConfig->GetGlobalConfigValue("DB_USER"));
	snprintf(m_pszDBInfo[IDX_DB_CONN_INFO_PASS], sizeof(m_pszDBInfo[IDX_DB_CONN_INFO_PASS]), "%s"
							, m_pclsConfig->GetGlobalConfigValue("DB_PASS"));
	snprintf(m_pszDBInfo[IDX_DB_CONN_INFO_DB], sizeof(m_pszDBInfo[IDX_DB_CONN_INFO_DB]), "%s"
							, m_pclsConfig->GetGlobalConfigValue("DB_DATABASE"));

	m_pclsLog->INFO("DB HOST : %s", m_pszDBInfo[IDX_DB_CONN_INFO_HOST]);
	m_pclsLog->INFO("DB PORT : %s", m_pszDBInfo[IDX_DB_CONN_INFO_PORT]);
	m_pclsLog->INFO("DB USER : %s", m_pszDBInfo[IDX_DB_CONN_INFO_USER]);
	m_pclsLog->INFO("DB PASS : %s", m_pszDBInfo[IDX_DB_CONN_INFO_PASS]);
	m_pclsLog->INFO("DB DATABASE : %s", m_pszDBInfo[IDX_DB_CONN_INFO_DB]);

	if(LoadConfig() < 0)
	{
		m_pclsLog->ERROR("Fail to Load Config");
		return -1;
	}

	if(InitSharedLibrary() < 0)
	{
		m_pclsLog->ERROR("Fail to Load Shared Library");
		return -1;
	}

	m_pclsSock = new CMesgExchSocketServer();
	if(NULL == m_pclsSock)
	{
		m_pclsLog->ERROR("Fail to Create Socket");
		return -1;
	}	

	if(Run() < 0)
	{
		m_pclsLog->ERROR("Fail to Run Process");
		return -1;
	}
	return 0;
}

int RSARoot::LoadConfig()
{
	int ret = 0;
	char szQuery[DEF_MEM_BUF_1024];

	//Create DB Instance
	m_pclsDB = new (std::nothrow) MariaDB();
	if(m_pclsDB == NULL)
	{
		m_pclsLog->ERROR("Fail to DB Connect");
		return -1;
	}

	if( m_pclsDB->Connect(
						m_pszDBInfo[IDX_DB_CONN_INFO_HOST],
						atoi(m_pszDBInfo[IDX_DB_CONN_INFO_PORT]),
						m_pszDBInfo[IDX_DB_CONN_INFO_USER],
						m_pszDBInfo[IDX_DB_CONN_INFO_PASS],
						m_pszDBInfo[IDX_DB_CONN_INFO_DB]
					) < 0 )
	{
		m_pclsLog->ERROR("Failed to DB Connect / host[%s], port[%s], user[%s], pw[%s], db[%s]"
						, m_pszDBInfo[IDX_DB_CONN_INFO_HOST]
						, m_pszDBInfo[IDX_DB_CONN_INFO_PORT]
						, m_pszDBInfo[IDX_DB_CONN_INFO_USER]
						, m_pszDBInfo[IDX_DB_CONN_INFO_PASS]
						, m_pszDBInfo[IDX_DB_CONN_INFO_DB]
					);
		delete m_pclsDB;
		return -1;
	}

	//Define Variables For Fetch Data From Query Result set
	char szGroupID 	[DEF_MEM_BUF_64];
	char szGroupName[DEF_MEM_BUF_64];
	char szPlugName [DEF_MEM_BUF_64];
	char szPeriod 	[DEF_MEM_BUF_64];
	char szVNFM		[DEF_MEM_BUF_64];

	FetchMaria fData;
	fData.Set(szGroupID 	, sizeof(szGroupID));
	fData.Set(szGroupName 	, sizeof(szGroupName));
	fData.Set(szPlugName 	, sizeof(szPlugName));
	fData.Set(szPeriod 		, sizeof(szPeriod));
	fData.Set(szVNFM	, sizeof(szVNFM));

	memset(szQuery, 0x00, sizeof(szQuery));
	sprintf(szQuery, "SELECT RSC_GRP_ID, RSC_GRP_NAME, PLUG_NAME, PERIOD, VNFM_SEND_YN FROM TAT_RSC_GRP_DEF \
						WHERE USE_YN='Y' AND ROOT_YN='Y'");
	ret = m_pclsDB->Query(&fData, szQuery, strlen(szQuery)); 
	if(ret < 0)
	{
		m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
		delete m_pclsDB;
		return -1;
	}

	m_pstRsc = new RESOURCE[ret];	
	for(int i = 0; i < ret ; i++)
	{
		memset(m_pstRsc[i].szGroupName, 0x00, sizeof(m_pstRsc[i].szGroupName));	
		memset(m_pstRsc[i].szGroupID, 0x00, sizeof(m_pstRsc[i].szGroupID));	
		memset(m_pstRsc[i].szPlugName, 0x00, sizeof(m_pstRsc[i].szPlugName));	
		m_pstRsc[i].nPeriod = 0;
		m_pstRsc[i].bRoot = false;
		m_pstRsc[i].bVNFM = false;
		m_pstRsc[i].mapRsc.clear();
        m_pstRsc[i].strFullJson.clear();
        m_pstRsc[i].strRootTrapJson.clear();

	}


	//Fetch Resource Group Data
	while(true)
	{
		if(fData.Fetch() == false)
			break;

		sprintf(m_pstRsc[m_nGroupCount].szGroupName, "%s", szGroupName);
		sprintf(m_pstRsc[m_nGroupCount].szGroupID, "%s", szGroupID);
		sprintf(m_pstRsc[m_nGroupCount].szPlugName, "%s", szPlugName);
		m_pstRsc[m_nGroupCount].nPeriod = atoi(szPeriod);
		m_pstRsc[m_nGroupCount].bVNFM = (szVNFM[0] == 'Y') ? true : false;

		if(m_pstRsc[m_nGroupCount].nPeriod < 0)
		{
			m_pclsLog->ERROR("Invalid Period");
			delete m_pclsDB;
			return -1;
		}
		m_mapRsc.insert( std::pair<string, RESOURCE*>(m_pstRsc[m_nGroupCount].szGroupName, &(m_pstRsc[m_nGroupCount])) );

		m_nGroupCount ++;
	}

		
	//Define Variables For Fetch Data From Query Result set
	RESOURCE_ATTR *pstRscAttr;
	char szRscName 	[DEF_MEM_BUF_64];
	char szRscID 	[DEF_MEM_BUF_64];
	char szRscArgs	[DEF_MEM_BUF_64];



	for(int i = 0; i < m_nGroupCount ; i++)
	{
		m_pclsLog->INFO("\n");
		m_pclsLog->INFO("Group ID : %s, Group Name : %-10s, Plug-in Name : %s, Period : %d, VNFM : %d"
						, m_pstRsc[i].szGroupID
						, m_pstRsc[i].szGroupName
						, m_pstRsc[i].szPlugName
						, m_pstRsc[i].nPeriod
						, m_pstRsc[i].bVNFM
						);

		pstRscAttr = NULL;

		memset(szQuery, 0x00, sizeof(szQuery));
		sprintf(szQuery, "SELECT RSC_NAME, RSC_ID, ARGS FROM TAT_RSC_DEF A, TAT_RSC_GRP_DEF B \
							WHERE \
							A.RSC_GRP_ID = B.RSC_GRP_ID\
							AND A.USE_YN='Y' \
							AND A.RSC_GRP_ID = '%s'\
							AND A.PKG_NAME = '%s'\
							AND A.NODE_TYPE = '%s'\
							AND B.ROOT_YN='Y'"
							, m_pstRsc[i].szGroupID
							//TEST Code 추후 필히 바꿔야 함
							, "VOFCS"
							, "AP"
						);

		fData.Clear();
		fData.Set(szRscName	, sizeof(szRscName));
		fData.Set(szRscID	, sizeof(szRscID));
		fData.Set(szRscArgs	, sizeof(szRscArgs));


		ret = m_pclsDB->Query(&fData, szQuery, strlen(szQuery)); 
		if(ret < 0)
		{
			m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
			delete m_pclsDB;
			return -1;
		}

		while(true)
		{
			if(fData.Fetch() == false)
			{
				break;
			}

			pstRscAttr = new RESOURCE_ATTR;
			memset(pstRscAttr, 0x00, sizeof(RESOURCE_ATTR));
			sprintf(pstRscAttr->szID, "%s", szRscID);
			sprintf(pstRscAttr->szName, "%s", szRscName);
			sprintf(pstRscAttr->szArgs, "%s", szRscArgs);

			m_pclsLog->INFO("-- Resource ID : %s, Resource Name : %-10s, Args : %s"
							, szRscID
							, pstRscAttr->szName
							, pstRscAttr->szArgs
							);

			m_pstRsc[i].mapRsc.insert( pair<string, RESOURCE_ATTR*>(pstRscAttr->szName, pstRscAttr) );
		}

	}
	m_pclsLog->INFO("\n");


	return 0;
}

int RSARoot::InitSharedLibrary()
{
	char szBuff[DEF_MEM_BUF_1024];
	RSABase* (*dlfunc)();
	RSABase *pclsRSA;

	for(int i = 0; i < m_nGroupCount; i++)
	{
		sprintf(szBuff, "%s/%s.so"
						, m_pclsConfig->GetConfigValue("RSA", "SHARED_LIB_PATH")
						, m_pstRsc[i].szPlugName
				);

		m_pclsLog->INFO("PlugIn Names %s", szBuff);
		m_pstRsc[i].pHandle = dlopen(szBuff, RTLD_LAZY);
		if(m_pstRsc[i].pHandle == NULL)
		{
			m_pclsLog->ERROR("dlopen Error %s", dlerror());
			return -1;
		}

		dlfunc = (RSABase * (*)())dlsym(m_pstRsc[i].pHandle, m_pstRsc[i].szPlugName);
		m_pstRsc[i].pclsRSA = (void*)dlfunc();
		pclsRSA = (RSABase*)m_pstRsc[i].pclsRSA;
		pclsRSA->Initialize(m_pclsLog, &(m_pstRsc[i]), (void*)this);
//		pclsRSA->Run();
	}

	return 0;
}

int RSARoot::SendResponseToRSA(CSocket *a_pclsClient, int a_nCmd)
{
	char szBuffer[DEF_MEM_BUF_64];

	snprintf(szBuffer, sizeof(szBuffer), "%d", a_nCmd);
	m_pclsSock->SetCommand(szBuffer);
	m_pclsSock->SetFlagResponse();
	m_pclsSock->SetSource(0, PROCID_ATOM_NA_RSA);
	m_pclsSock->SetDestination(0, PROCID_ATOM_NA_RSA);

	if (false == m_pclsSock->SendMesg(a_pclsClient))
	{
		m_pclsLog->ERROR("message send failed");
		m_pclsLog->ERROR("- %s", a_pclsClient->m_strErrorMsg.c_str());
		return -1;
	}

	return 0;
}

int RSARoot::RecvMesgFromRSA(CSocket *a_pclsClient)
{
	if(m_pclsSock->RecvMesg(a_pclsClient, NULL, 5) < 0) 
	{
		m_pclsLog->ERROR("message receive failed, errno=%d", errno);
		m_pclsLog->ERROR("Socket, %s", a_pclsClient->m_strErrorMsg.c_str());
		m_pclsLog->ERROR("Protocol, %s", m_pclsSock->CProtocol::m_strErrorMsg.c_str());
		return -1;
	}

	m_pclsSock->Print(m_pclsLog, LV_DEBUG, true);
	
	return 0;
	
}

int RSARoot::ProcessInitMsg(string &a_strBuff)
{
	rabbit::document doc;
	doc.parse(m_pclsSock->GetPayload());

	rabbit::object o_root = doc["BODY"];
	rabbit::object o_pkg = o_root["PKG_NAME"];
	rabbit::object o_node_no = o_root["NODE_NO"];
	rabbit::object o_node_type = o_root["NODE_TYPE"];

	m_nNodeID = 0;
	memset(m_szPkgName, 0x00, sizeof(m_szPkgName));
	memset(m_szNodeType, 0x00, sizeof(m_szNodeType));

	m_nNodeID = atoi(o_node_no.str().c_str());
	snprintf(m_szPkgName, sizeof(m_szPkgName), "%s", m_szPkgName);
	snprintf(m_szNodeType, sizeof(m_szNodeType), "%s", m_szNodeType);

	return 0;
}

int RSARoot::ProcessRSAMsg(string &a_strBuff)
{
	time_t tCur;
	rabbit::document doc;
	doc.parse(m_pclsSock->GetPayload());

	rabbit::object o_root = doc["BODY"];
	rabbit::object o_rsc = o_root["RESOURCE"];
	rabbit::object o_node = o_root["NODE_NO"];
	rabbit::object o_time = o_root["TIME"];

	m_nNodeID = atoi(o_node.str().c_str());
	tCur = atoi(o_time.str().c_str());	

	RSABase *pclsRSA;
	RESOURCE *pRscGroup = NULL;
	map<string, RESOURCE*>::iterator it;


    Document docRes;
    docRes.SetObject();
    Document::AllocatorType &allocator = docRes.GetAllocator();
    Document docCopy;
    Value vBody(kObjectType);


	it = m_mapRsc.find(o_rsc.str().c_str());		
	if(it != m_mapRsc.end())
	{
		pRscGroup = it->second;

		if( NULL == pRscGroup->pclsRSA )
			return -1;

		pclsRSA = (RSABase*)pRscGroup->pclsRSA;

		if(pclsRSA->Run() < 0)
			return -1;

		pclsRSA->MakeJson(tCur);

        if( pRscGroup->strFullJson.size() > 0 )
        {
            docCopy.Parse(pRscGroup->strFullJson.c_str());
            vBody.Swap(docCopy);
            docRes.AddMember("FULL", vBody, allocator);
        }

        if( pRscGroup->strRootTrapJson.size() > 0 )
        {
            docCopy.Parse(pRscGroup->strRootTrapJson.c_str());
            vBody.Swap(docCopy);
            docRes.AddMember("TRAP", vBody, allocator);
        }

        StringBuffer strbuf;
        Writer<StringBuffer> w(strbuf);
        docRes.Accept(w);

        a_strBuff.assign(strbuf.GetString());

		return 0;
	}


	return -1;
}

int RSARoot::ProcessStatMsg(string &a_strBuff)
{
	time_t tCur;
	rabbit::document doc;
	doc.parse(m_pclsSock->GetPayload());

	rabbit::object o_root = doc["BODY"];
	rabbit::object o_rsc = o_root["RESOURCE"];
	rabbit::object o_node = o_root["NODE_NO"];
	rabbit::object o_time = o_root["TIME"];

	m_nNodeID = atoi(o_node.str().c_str());
	tCur = atoi(o_time.str().c_str());	

	RSABase *pclsRSA;
	RESOURCE *pRscGroup = NULL;
	map<string, RESOURCE*>::iterator it;

	it = m_mapRsc.find(o_rsc.str().c_str());		
	if(it != m_mapRsc.end())
	{
		pRscGroup = it->second;

		if( NULL == pRscGroup->pclsRSA )
			return -1;

		pclsRSA = (RSABase*)pRscGroup->pclsRSA;

		if(pclsRSA->MakeStatJson(tCur) < 0)
			return -1;

		a_strBuff.assign(pRscGroup->strStatJson);

		return 0;
	}


	return -1;
}

int RSARoot::CheckTrapMsg(std::string &a_strJson)
{
	rabbit::document doc;
	doc.parse(m_pclsSock->GetPayload());

	rabbit::object o_root = doc["BODY"];
	rabbit::object o_rsc = o_root["RESOURCE"];
	rabbit::object o_node = o_root["NODE_NO"];
	rabbit::object o_time = o_root["TIME"];
#if 0
	rabbit::document doc;
	rabbit::object o_root = doc["BODY"];

    o_root["CODE"] = StringRef(a_szCode);
    o_root["TARGET"] = StringRef(a_szTarget);
    o_root["VALUE"] = StringRef(a_szValue);
#endif
    m_pclsSock->SetPayload(doc.str().c_str());
	if(SendResponseToRSA(m_pclsClient, CMD_RSA_ROOT_TRAP) < 0)
	{
		m_pclsSock->ClosePeer(m_pclsClient);
        return -1;
	}

    return 0;
}

int RSARoot::Run()
{
	int nCmd = 0;
	string strBuff;

	if(false == m_pclsSock->Listen("127.0.0.1", 5888))
	{
		m_pclsLog->ERROR("Connect Failed");
		return -1;
	}

	CSocket *client = NULL;
	
	while(true)
	{
		client = m_pclsSock->Select(0, 0);
		if (NULL == client)
		{
			m_pclsLog->ERROR("%s", client->m_strErrorMsg.c_str());
			break;
		}
        m_pclsClient = client;

		string strIp;
		int nPort = 0;
		client->GetPeerIpPort(&strIp, &nPort);
		m_pclsLog->DEBUG("Connected, peer IP : %s, PORT : %d", strIp.c_str(), nPort);

		if(RecvMesgFromRSA(client) < 0)
		{
			m_pclsSock->ClosePeer(client);
			continue;
		}

		strBuff.clear();

		switch( atoi(m_pclsSock->GetCommand().c_str()) )
		{
			case CMD_RSA_ROOT_MSG :
				if(ProcessRSAMsg(strBuff) < 0)
				{
//					m_pclsSock->ClosePeer(client);
//					continue;
				}

				nCmd = CMD_RSA_ROOT_MSG;

				break;

			case CMD_RSA_ROOT_STAT :
				if(ProcessStatMsg(strBuff) < 0)
				{
//					continue;
				}
				nCmd = CMD_RSA_ROOT_STAT;
				break;
			case CMD_RSA_ROOT_INIT :
				if(ProcessInitMsg(strBuff) < 0)
				{
					m_pclsLog->ERROR("Process Init Msg Failed");
				}
				continue;
				break;
		}

		m_pclsSock->SetPayload(strBuff);
		if(SendResponseToRSA(client, nCmd) < 0)
		{
			m_pclsSock->ClosePeer(client);
			continue;
		}

	}

	m_pclsSock->CloseServer();
	m_pclsSock->ClosePeer(client);

	return 0;
}


int main(int argc, char *args[])
{
	int ret = 0;
	RSARoot *pRSA = NULL;

	if(argc < 2)
	{
		printf("Please Insert Config File Path\n");
		printf("RSA_ROOT [CONFIG FILE PATH]\n");
		return -1;
	}

	setenv("ATOM_CFG_HOME", args[1], 1);
	CConfig *p_clsConfig = new CConfig();
	if(p_clsConfig->Initialize(args[1]) < 0)
	{
		printf("Failed to Init Config");
		return -1;
	}

	CFileLog *pLog = new CFileLog(&ret);
	if(ret < 0)
	{
		printf("Log Init Failed\n");
		return -1;
	}	

	pLog->Initialize(p_clsConfig->GetGlobalConfigValue("LOG_PATH"), NULL, (char*)"RSA_ROOT", 3, LV_DEBUG);
	pLog->INFO("RSA Test Start");

	pRSA = new RSARoot(pLog, p_clsConfig);
	pRSA->Initialize();

	delete pLog;	
	return 0;
}

