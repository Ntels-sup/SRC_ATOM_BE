#include "RSAMain.hpp"


RSAMain::RSAMain(CFileLog *a_pclsLog, CConfig *a_pclsConfig)
{
	m_pclsLog = a_pclsLog;
	m_pclsConfig = a_pclsConfig;
	m_pstRsc = NULL;
	m_nGroupCount = 0;
	m_pclsRootSock = NULL;
	m_pstModuleOption = NULL;
	m_pclsMsg = NULL;
	m_pclsDB = NULL;

	memset(m_pszDBInfo, 0x00, sizeof(m_pszDBInfo));
}


RSAMain::~RSAMain()
{
	FreeResourceData();

	if(m_pclsEvent)
		delete m_pclsEvent;

	if(m_pclsMsg)
		delete m_pclsMsg;
}

int RSAMain::FreeResourceData()
{
	map<string, RESOURCE_ATTR *>::iterator it;

	for(int i = 0; i < m_nGroupCount ; i ++)
	{
		if( NULL != m_pstRsc[i].pclsRSA )
		{
			delete (RSABase*)m_pstRsc[i].pclsRSA;
		}

		if( NULL != m_pstRsc[i].pHandle )
		{
			dlclose(m_pstRsc[i].pHandle);
		}

		for(it = m_pstRsc[i].mapRsc.begin(); it != m_pstRsc[i].mapRsc.end(); ++it)
		{
			if(it->second != NULL)
				delete it->second;
		}

		m_pstRsc[i].mapRsc.clear();	
	}

	if(m_pstRsc)
		delete [] m_pstRsc;

	m_nGroupCount = 0;

	return 0;
}

int RSAMain::Initialize(CModule::ST_MODULE_OPTIONS* a_pstModuleOption)
{
	m_pclsLog->INFO("RSA Initialize Func Start");

	m_pstModuleOption = a_pstModuleOption;

	m_pclsDB = m_pstModuleOption->m_pclsDB;
	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("DB Class Is NULL");
		return -1;
	}

	if(LoadConfig() < 0)
	{
		m_pclsLog->ERROR("Fail to Load Config");
		return -1;
	}


    m_pclsEvent = new EventAPI();
    m_pclsEvent->SetNodeInfo(m_pstModuleOption->m_nNodeNo, m_pstModuleOption->m_szNodeName);
    m_pclsEvent->SetProcInfo( PROCID_ATOM_NA_RSA, "ATOM_NA_RSA" );
    m_pclsEvent->SetIPCandAddr( m_pstModuleOption->m_pclsModIpc, m_pstModuleOption->m_pclsAddress);

    if( false == m_pclsEvent->Init() )
    {
        m_pclsLog->ERROR("Fail to Init Event API in RSAMsg");
        return -1;
    }

    m_pclsMsg = new RSAMsg(m_pclsLog, m_pclsConfig, m_pclsEvent, m_pstModuleOption);
    //Root 와의 연결이 완료되지 않아도 RSA 를 기동
    if(m_pclsMsg->Initialize() < 0)
    {
        m_pclsLog->WARNING("Fail to Init RSA Msg");
        return -1;
    }

    if(InitSharedLibrary() < 0)
	{
		m_pclsLog->ERROR("Fail to Load Shared Library");
		return -1;
	}

	return 0;
}

int RSAMain::LoadConfig()
{
	int ret = 0;
	char szQuery[DEF_MEM_BUF_1024];
	


	//Define Variables For Fetch Data From Query Result set
	char szGroupID 	[DEF_MEM_BUF_64];
	char szGroupName[DEF_MEM_BUF_64];
	char szPlugName [DEF_MEM_BUF_64];
	char szPeriod 	[DEF_MEM_BUF_64];
	char szVNFM 	[DEF_MEM_BUF_64];
	char szROOT 	[DEF_MEM_BUF_64];
	char szSTAT 	[DEF_MEM_BUF_64];

	FetchMaria fData;
	fData.Set(szGroupID 	, sizeof(szGroupID));
	fData.Set(szGroupName 	, sizeof(szGroupName));
	fData.Set(szPlugName 	, sizeof(szPlugName));
	fData.Set(szPeriod 		, sizeof(szPeriod));
	fData.Set(szVNFM 		, sizeof(szVNFM));
	fData.Set(szROOT 		, sizeof(szROOT));
	fData.Set(szSTAT 		, sizeof(szSTAT));

	memset(szQuery, 0x00, sizeof(szQuery));
	sprintf(szQuery, "SELECT RSC_GRP_ID, RSC_GRP_NAME, PLUG_NAME, PERIOD, VNFM_SEND_YN, ROOT_YN, STAT_YN FROM TAT_RSC_GRP_DEF \
						WHERE USE_YN='Y'");
	ret = m_pclsDB->Query( &fData, szQuery, strlen(szQuery)); 
	if(ret < 0)
	{
		m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
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
		m_pstRsc[i].bStat = false;
		m_pstRsc[i].tNextCheck = 0;
		m_pstRsc[i].unExec = DEF_EXEC_DEFAULT;
		m_pstRsc[i].pclsRSA = NULL;
		m_pstRsc[i].pHandle = NULL;
		m_pstRsc[i].strFullJson.clear();
		m_pstRsc[i].strSummaryJson.clear();
		m_pstRsc[i].strStatJson.clear();
		m_pstRsc[i].mapRsc.clear();
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
		m_pstRsc[m_nGroupCount].bRoot = (szROOT[0] == 'Y') ? true : false;
		m_pstRsc[m_nGroupCount].bStat = (szSTAT[0] == 'Y') ? true : false;
		if(m_pstRsc[m_nGroupCount].nPeriod < 0)
		{
			m_pclsLog->ERROR("Invalid Period");
			return -1;
		}
		
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
							AND A.RSC_GRP_ID = '%s' \
							AND A.PKG_NAME ='%s'\
							AND A.NODE_TYPE ='%s'"
							, m_pstRsc[i].szGroupID
							//TEST Code 추후 필히 바꿔야 함
							, m_pstModuleOption->m_szPkgName
							, m_pstModuleOption->m_szNodeType
						);

		fData.Clear();
		fData.Set(szRscName	, sizeof(szRscName));
		fData.Set(szRscID	, sizeof(szRscID));
		fData.Set(szRscArgs	, sizeof(szRscArgs));


		ret = m_pclsDB->Query(&fData, szQuery, strlen(szQuery)); 
		if(ret < 0)
		{
			m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
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

int RSAMain::InitSharedLibrary()
{
	char szBuff[DEF_MEM_BUF_1024];
	RSABase* (*dlfunc)();
	RSABase *pclsRSA;

	for(int i = 0; i < m_nGroupCount; i++)
	{
		if(m_pstRsc[i].bRoot)
			continue;

		if(m_pstRsc[i].mapRsc.size() == 0)
		{
			m_pclsLog->INFO("Resource Group (%s) don't have Resource Attr, Skip Init Shared Library", m_pstRsc[i].szGroupName);
			continue;
		}

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
		if( pclsRSA->Initialize(m_pclsLog, &(m_pstRsc[i]), (void*)this) < 0 )
		{
			delete pclsRSA;
			m_pstRsc[i].pclsRSA = NULL;
		}
	}

	return 0;
}

int RSAMain::ExecutePlugin(time_t a_tCur)
{
	int ret = 0;
	int nExecCnt = 0;
	RSABase *pclsRSA;

	for(int i = 0; i < m_nGroupCount ; i++)
	{
		if( a_tCur < m_pstRsc[i].tNextCheck )
		{
			continue;
		}

		if(m_pstRsc[i].bRoot)
		{
			if(m_pstRsc[i].mapRsc.size() == 0)
				continue;				

			if(m_pclsMsg->SendRequestToRoot(&(m_pstRsc[i]), CMD_RSA_ROOT_MSG, m_pstRsc[i].tNextCheck) < 0)
			{
				continue;
			}

			if(m_pclsMsg->RecvMesgFromRoot(&(m_pstRsc[i])) < 0)
			{
				continue;
			}

			if(m_pstRsc[i].strFullJson.size() > 0)	
			{
				m_pstRsc[i].unExec |= DEF_EXEC_SET_FULL;
				nExecCnt++;
			}
			m_pstRsc[i].tNextCheck = a_tCur - (a_tCur % m_pstRsc[i].nPeriod) + m_pstRsc[i].nPeriod;

			continue;
		}

		if( NULL == m_pstRsc[i].pclsRSA )
			continue;

		pclsRSA = (RSABase*)m_pstRsc[i].pclsRSA;

		ret = pclsRSA->Run();
		pclsRSA->MakeJson(m_pstRsc[i].tNextCheck);
//		m_pclsLog->DEBUG("dddddddddddddddddddddd %s", m_pstRsc[i].strStatJson.c_str());

		if( 0 == ret )
		{
			nExecCnt++;
		}

		m_pstRsc[i].tNextCheck = a_tCur - (a_tCur % m_pstRsc[i].nPeriod) + m_pstRsc[i].nPeriod;
		
	}

	return nExecCnt;
}


int RSAMain::Run()
{
	int ret = 0;
	time_t	tCur = 0;

	CModuleIPC * clsIPC = m_pstModuleOption->m_pclsModIpc;
	CProtocol clsProto;

	ret = ExecutePlugin(tCur);

	while(true)
	{
		time(&tCur);
		if (clsIPC->RecvMesg(m_pstModuleOption->m_nProcNo, clsProto, -1) > 0) {
			m_pclsLog->DEBUG("Receive Message");
			clsProto.Print(m_pclsLog, LV_DEBUG, true);
			// cjr _ 20160610
			m_pclsMsg->ProcessCmd(&clsProto, tCur, m_pstRsc, m_nGroupCount, (void*)this);
		}

        ret = ExecutePlugin(tCur);

		if(ret > 0)
			m_pclsMsg->SendResponseMsg(tCur, m_pstRsc, m_nGroupCount);

#if 0
		m_pclsMsg->SendMonitoringInfo(tCur);
		m_pclsMsg->SendSummaryInfo(tCur);
		m_pclsMsg->SendPerformanceInfo(tCur);
#endif	
		sleep(1);
	}

	return 0;
}

void* RSAStart(void *pArg)
{
	CModule::ST_MODULE_OPTIONS* stOption = static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);

//	stOption->m_szPkgName;      // ex) VOFCS 
//	stOption->m_szNodeType;     // ex) AP or LB or TM
//	stOption->m_szProcName;     // ex) 하단 MODULE_OP.szProcName  과 같다. 
//	stOption->m_nNodeId;        // 모듈의 node id 
//	stOption->m_nProcId;        // 모듈의 process id
//	stOption->m_pclsModIpc;     // 모듈간의 메세지 교환 채널
//	stOption->m_pclsAddress;    // Agent 내의 모듈, Process ID 검색

	int ret = 0;
	CConfig *p_clsConfig = NULL;
	RSAMain *p_clsRSA = NULL;

	p_clsConfig = new CConfig();
	if(p_clsConfig->Initialize(stOption->m_szCfgFile) < 0)
	{
		printf("Failed to Init Config\n");
		return NULL;
	}

	CFileLog *p_clsLog = new CFileLog(&ret);
	if(ret < 0)
	{
		printf("Log Init Failed\n");
		goto EXIT;
	}

	p_clsLog->Initialize(
							p_clsConfig->GetGlobalConfigValue("LOG_PATH"),  //Log Base Path
							NULL, 			//Service Name
							"RSA",  	//Process Name
							3,   			//Dup Period
							LV_DEBUG   		//Log Level
						);



	p_clsRSA = new RSAMain(p_clsLog, p_clsConfig);

	if(p_clsRSA->Initialize(stOption) < 0)
	{
		p_clsLog->ERROR("Failed to RSAMain Init");
		goto EXIT;
	}
	
	if(p_clsRSA->Run() < 0)
	{
		p_clsLog->ERROR("Fail to Run Process");
		goto EXIT;
	}

EXIT:
	if(p_clsRSA)
		delete p_clsRSA;
	if(p_clsConfig)
		delete p_clsConfig;
	if(p_clsLog)
		delete p_clsLog;

	return NULL;
}

MODULE_OP = {
	"ATOM_NA_RSA",
	true,
	NULL,
	RSAStart,
	NULL
};
