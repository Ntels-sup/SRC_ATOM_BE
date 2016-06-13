#include "CAtomAPI.hpp"

bool ATOM_API::m_bRun = false;

ATOM_API::ATOM_API()
{
	m_pclsIPC = NULL;
	m_pclsReqProtocol = NULL;
	m_pclsAppCtl = NULL;
	m_pclsLog = NULL;
	m_pclsConfig = NULL;
	m_pclsTrace = NULL;

	memset(m_szPkgName, 0x00, sizeof(m_szPkgName));
	memset(m_szNodeType, 0x00, sizeof(m_szNodeType));
	memset(m_szProcName, 0x00, sizeof(m_szProcName));

	m_nNodeNo = 0;
	m_nProcNo = 0;

	this->m_bRun = false;

	m_nCmdIdx = 0;
	m_vecCommand.clear();
}



ATOM_API::~ATOM_API()
{
	if(m_pclsIPC)
		delete m_pclsIPC;

	if(m_pclsReqProtocol)
		delete m_pclsReqProtocol;

	if(m_pclsAppCtl)
		delete m_pclsAppCtl;

	if(m_pclsLog)
		delete m_pclsLog;
	
	if(m_pclsConfig)
		delete m_pclsConfig;

	if(m_pclsTrace)
		delete m_pclsTrace;
}

void ATOM_API::StopSigHandler(int a_nSigNo)
{
	m_bRun = false;
}

int ATOM_API::Init(int a_nArgc, char *a_parrArgs[], int a_nLogLevel, int a_nInstanceID, int a_nCmdType)
{
	int ret = 0;
	bool bBackup = false;
	bool bMSync = false;

	m_pclsAppCtl = new CAppControl();
	if( false == m_pclsAppCtl->Init(a_nArgc, a_parrArgs) )
	{
		printf("Fail to AppControl Class\n");
		SetErrorMsg(-E_ATOM_CTL_INIT, "Fail to AppControl Class");
		return -E_ATOM_CTL_INIT;
	}
	
	snprintf(m_szPkgName, sizeof(m_szPkgName), "%s", m_pclsAppCtl->GetPkgName());
	snprintf(m_szNodeType, sizeof(m_szNodeType), "%s", m_pclsAppCtl->GetNodeType());
	snprintf(m_szProcName, sizeof(m_szProcName), "%s", m_pclsAppCtl->GetProcName());

	m_nNodeNo = m_pclsAppCtl->GetNodeNo();
	m_nProcNo = m_pclsAppCtl->GetProcNo();

	m_pclsConfig = new CConfig();
	if( (ret = m_pclsConfig->Initialize()) < 0)
	{
		printf("Fail to Config Init, ErrCode : %d\n", ret);
		SetErrorMsg(-E_ATOM_CONF_INIT, "Fail to Config Init");
		return -E_ATOM_CONF_INIT;		
	}

	m_pclsLog = new CFileLog(&ret);
	if(ret < 0)
	{
		printf("Fail to Log Alloc\n");
		SetErrorMsg(-E_ATOM_LOG_ALLOC, "Fail to Log Alloc");
		return -E_ATOM_LOG_ALLOC;
	}

	ret = m_pclsLog->Initialize(
							m_pclsConfig->GetGlobalConfigValue("LOG_PATH"),
							m_pclsAppCtl->GetSvcName(),
							m_szProcName,
							ATOM_DEF_LOG_DUP_PERIOD,
							a_nLogLevel
						);
	if(ret < 0)
    {
        printf("Fail to Log Init\n");
		SetErrorMsg(-E_ATOM_LOG_INIT, "Fail to Log Init");
        return -E_ATOM_LOG_INIT;
    }

	m_pclsReqProtocol = new CProtocol();

	if(m_pclsConfig->GetConfigValue("QUEUE", "BACKUP_YN"))
		bBackup = m_pclsConfig->GetConfigValue("QUEUE", "BACKUP_YN")[0] == 'Y' ? true : false;
	if(m_pclsConfig->GetConfigValue("QUEUE", "MSYNC_YN"))
		bMSync = m_pclsConfig->GetConfigValue("QUEUE", "MSYNC_YN")[0] == 'Y' ? true : false;

//	m_pclsIPC = new CLQManager((char*)"vOFCS", (char*)"AP", (char*)"DPC02", 0, false, false, NULL);
	m_pclsIPC = new CLQManager(
									m_szPkgName, 
									m_szNodeType, 
									m_szProcName, 
									a_nInstanceID, 
									bBackup,
									bMSync, 
									NULL
								);

	if(m_pclsIPC->Initialize(a_nCmdType) != 0)
	{
		SetErrorMsg(-E_ATOM_Q_INIT, m_pclsIPC->GetErrorMsg());
		return -E_ATOM_Q_INIT;
	}

	m_pclsTrace = new CTraceAPI();
	m_stTraceInfo.vecData.assign(ATOM_DEF_MAX_TRACE_IDX, "");

	for(int i = 0; i < ATOM_DEF_MAX_TRACE_IDX; i++)
	{
		m_stTraceInfo.narrTraceReqNo[i] = -1;
	}

	this->m_bRun = true;

	signal(SIGTERM, StopSigHandler);

	m_pclsLog->INFO("\n");
	m_pclsLog->INFO(" [ Process Info ] ===========================");
	m_pclsLog->INFO("PKG_NAME    : %s", m_szPkgName);	
	m_pclsLog->INFO("NODE_TYPE   : %s", m_szNodeType);	
	m_pclsLog->INFO("NODE_NO     : %d", m_nNodeNo);	
	m_pclsLog->INFO("NODE_NAME   : %s", m_pclsAppCtl->GetNodeName());	
	m_pclsLog->INFO("PROC_NAME   : %s", m_szProcName);	
	m_pclsLog->INFO("PROC_NO     : %d", m_nProcNo);	
	m_pclsLog->INFO("Q_BACKUP_YN : %d [0 : false, 1 : true]", bBackup);	
	m_pclsLog->INFO("Q_MSYNC_YN  : %d [0 : false, 1 : true]", bMSync);	
	m_pclsLog->INFO(" ============================================");
	m_pclsLog->INFO("\n");

	return 0;
}


//##################################################
// IPC Data/Command Read/Write Func
//##################################################
int ATOM_API::ReadWait(int a_nTime)
{
	int ret = 0;
	ret = m_pclsIPC->ReadWait(a_nTime);

	if( DEF_SIG_DATA != ret )
		return -1;

	return 0;
}

int ATOM_API::GetData(char **a_pData)
{
	int ret = 0;

	ret = m_pclsIPC->ReadData(a_pData);

	if(ret < 0 && ret != E_Q_NOENT)
		SetErrorMsg(ret, m_pclsIPC->GetErrorMsg());

	return ret;
}

int ATOM_API::SendData(int a_nIdx)
{
	int ret = 0;
	
	ret = m_pclsIPC->WriteData(a_nIdx);
	if(ret < 0)
		SetErrorMsg(ret, m_pclsIPC->GetErrorMsg());

	return ret;
}

int ATOM_API::SendData(char *a_pData, int a_nLength, int a_nIdx)
{
	int ret = 0;

	ret = m_pclsIPC->WriteData(a_pData, a_nLength, a_nIdx);
	if(ret < 0)
		SetErrorMsg(ret, m_pclsIPC->GetErrorMsg());

	return ret;
}

void  ATOM_API::ReadComplete()
{
	m_pclsIPC->ReadComplete();
}

void ATOM_API::FreeReadData()
{
	m_pclsIPC->FreeReadData();
}

CProtocol *ATOM_API::BlockCommand(int a_nCmd, int a_nTime)
{
	int ret = 0;
	char *pData = NULL;
	time_t tStart, tCur;

	CProtocol *a_pclsReqProtocol = NULL;

	time(&tStart);
	while(1)
	{
		time(&tCur);
		if( (tCur - tStart) >= a_nTime )
			break;

		ret = m_pclsIPC->ReceiveCommand(&pData);
		if( 0 < ret )
		{
			if( NULL != pData )
			{
				a_pclsReqProtocol = new CProtocol();
				a_pclsReqProtocol->Clear();

				if( false == a_pclsReqProtocol->SetAll(pData, ret) )
				{
					m_pclsLog->ERROR("Fail to Protocol Init");
					FreeReadData();
					SetErrorMsg(-E_ATOM_CMD_SET_FAIL, "Fail to Protocol Class Init");
					continue;
				}

				if(atoi(a_pclsReqProtocol->GetCommand().c_str()) == a_nCmd)
					break;

				m_vecCommand.push_back(a_pclsReqProtocol);
				a_pclsReqProtocol = NULL;
			}
			else
			{
				m_pclsLog->ERROR("Command Pointer is NULL");
				FreeReadData();
				SetErrorMsg(-E_ATOM_CMD_PTR_ERR, "Command Pointer is NULL");
				continue;
			}
		}
	}

	return a_pclsReqProtocol;
}


int ATOM_API::GetCommand()
{
	int ret = 0;
	char *pData = NULL;

	if(m_vecCommand.size() == 0)
	{
		ret = m_pclsIPC->ReceiveCommand(&pData);
		if( 0 < ret )
		{
			if( NULL != pData )
			{
				m_pclsReqProtocol->Clear();
				if( false == m_pclsReqProtocol->SetAll(pData, ret) )
				{
					m_pclsLog->ERROR("Fail to Protocol Init");
					FreeReadData();
					SetErrorMsg(-E_ATOM_CMD_SET_FAIL, "Fail to Protocol Class Init");
					return -E_ATOM_CMD_SET_FAIL;
				}

			}
			else
			{
				m_pclsLog->ERROR("Command Pointer is NULL");
				FreeReadData();
				SetErrorMsg(-E_ATOM_CMD_PTR_ERR, "Command Pointer is NULL");
				return -E_ATOM_CMD_PTR_ERR;
			}

			return ProcessCommand(m_pclsReqProtocol);	
		}
		else 
		{
			return ret;
		}

	}
	else
	{
		if( NULL != m_pclsReqProtocol )
			delete m_pclsReqProtocol;

		m_pclsReqProtocol = m_vecCommand[0];

		ret = ProcessCommand(m_pclsReqProtocol);

		m_vecCommand.erase(m_vecCommand.begin());

		return ret;
	}

	return ret;
}


int ATOM_API::ProcessCommand(CProtocol *a_pclsReqProtocol)
{
	int nCmd = 0;
	int nOpt = 0;
	std::string strReq;
	std::string strRes;

	CProtocol clsResProtocol;

	nCmd = atoi(a_pclsReqProtocol->GetCommand().c_str());

	switch( nCmd )
	{
		//Application Controll Cmd by PRA
		case CMD_APP_CTL :
			strReq.assign(a_pclsReqProtocol->GetPayload());
			nCmd = m_pclsAppCtl->Request(strReq, &nOpt);

			switch(nCmd)
			{
				case ATOM_CMD_APP_STOP :
					strRes.assign(m_pclsAppCtl->Response(true, "OK"));
					this->m_bRun = false;

					clsResProtocol.SetResponse(*a_pclsReqProtocol);
					clsResProtocol.SetPayload(strRes);
					return SendCommand(&clsResProtocol);

				case ATOM_CMD_APP_INIT :
//					strRes.assign(m_pclsAppCtl->Response(true, "OK"));
//					clsResProtocol.SetResponse(*a_pclsReqProtocol);
//					clsResProtocol.SetPayload(strRes);
//					return SendCommand(&clsResProtocol);
					return ATOM_CMD_APP_INIT;

				case ATOM_CMD_APP_SUSPEND :
					return ATOM_CMD_APP_SUSPEND;

				case ATOM_CMD_APP_RESUME :
					return ATOM_CMD_APP_RESUME;

				case ATOM_CMD_APP_LOGLEVEL :
					if(nOpt >= LV_CRITICAL && nOpt <= LV_DEBUG)
					{
						m_pclsLog->SetLogLevel(nOpt);
					}
					strRes.assign(m_pclsAppCtl->Response(true, "OK"));

					clsResProtocol.SetResponse(*a_pclsReqProtocol);
					clsResProtocol.SetPayload(strRes);
					return SendCommand(&clsResProtocol);

				default :
//					SetErrorMsg(-E_ATOM_CMD_UNKNOWN, "Unknown AppCtl Command %d", nCmd);
//					SendTrap(ATOM_DEF_ALM_UNKNOWN_CMD, m_szProcName, 0, "", "");
//					return -E_ATOM_CMD_UNKNOWN;
					return nCmd;
			}
			break;
		//End of Application Control Cmd
			
		case CMD_TRACE_ON:
		case CMD_TRACE_OFF:
			if(m_pclsTrace->GetTraceRequest(a_pclsReqProtocol, &clsResProtocol) < 0)
			{
				SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "Trace Command Parsing Error");
				return -E_ATOM_CMD_PARSE_ERR;
			}
				
			return SendCommand(&clsResProtocol);
		case CMD_STS_APP:
			return nCmd;
        case CMD_VNF_PERF_TPS:
            return nCmd;
		case ATOM_CMD_APP_TEST:
			return SendResponse(true, "OK");
		default :
//			SetErrorMsg(-E_ATOM_CMD_UNKNOWN, "Unknown Command %d", nCmd);
//			SendTrap(ATOM_DEF_ALM_UNKNOWN_CMD, m_szProcName, 0, "", "");
//			return -E_ATOM_CMD_UNKNOWN;
			return nCmd;
	}

	return -1;
}

int ATOM_API::SendResponse(bool a_bRet, const char *a_szResult)
{
	CProtocol clsProtocol;

	std::string strRes;
	strRes.assign(m_pclsAppCtl->Response(a_bRet, a_szResult));

	clsProtocol.Clear();
	clsProtocol.SetResponse(*m_pclsReqProtocol);
	clsProtocol.SetPayload(strRes);
	return SendCommand(&clsProtocol);
}


int ATOM_API::SendCommandResult(const char *a_pData, int a_nLength)
{
	int ret = 0;

	CProtocol clsProto;
	std::vector<char> vecStream;

	clsProto.Clear();
	clsProto.SetResponse(*m_pclsReqProtocol);
	clsProto.SetPayload(a_pData, a_nLength);	

	clsProto.GenStream(vecStream);

	ret = m_pclsIPC->SendCommandResult(&vecStream[0], vecStream.size());

	if(ret < 0)
	{
		m_pclsLog->ERROR("(1) Send Command Error Payload : \n%s", clsProto.GetPayload().c_str());
		SetErrorMsg(ret, m_pclsIPC->GetErrorMsg());
	}
	return ret;
}

int ATOM_API::SendCommand(CProtocol *a_pclsProtocol)
{
	int ret = 0;

	std::vector<char> vecStream;
	
	a_pclsProtocol->GenStream(vecStream);
	
	ret = m_pclsIPC->SendCommandResult(&vecStream[0], vecStream.size());
	if(ret < 0)
	{
		m_pclsLog->ERROR("(2) Send Command Error Payload : \n%s", a_pclsProtocol->GetPayload().c_str());
		SetErrorMsg(ret, m_pclsIPC->GetErrorMsg());
	}

	return ret;

}

int ATOM_API::SendCommand(CProtocol *a_pclsProtocol, char *a_szProcName, int a_nInstanceID)
{
	int ret = 0;
	std::vector<char> vecStream;
	a_pclsProtocol->GenStream(vecStream);

	if( (int)vecStream.size() != a_pclsProtocol->GetLength() )
	{
		SetErrorMsg(-E_ATOM_CMD_SEND_FAIL, "Fail to Send Command");
		return -E_ATOM_CMD_SEND_FAIL;
	}

	ret = m_pclsIPC->SendCommand(a_szProcName, a_nInstanceID, &vecStream[0], vecStream.size());

	if(ret < 0)
	{
		m_pclsLog->ERROR("(3) Send Command Error Payload : \n%s", a_pclsProtocol->GetPayload().c_str());
		SetErrorMsg(ret, m_pclsIPC->GetErrorMsg());
	}
	return ret;
}

int ATOM_API::GetQueueIndex(int a_nType, const char *a_szProc)
{
	int ret = 0;

	if(a_nType == ATOM_DEF_QUEUE_TYPE_READ)
		ret = m_pclsIPC->GetReadQueueIndex((char*)a_szProc);
	else if(a_nType == ATOM_DEF_QUEUE_TYPE_WRITE)
		ret = m_pclsIPC->GetWriteQueueIndex((char*)a_szProc);

	if(ret < 0)
		SetErrorMsg(ret, "Failed to Get Queue Index");

	return ret;
}

int ATOM_API::GetQueueCount(int a_nType)
{
	if(a_nType == ATOM_DEF_QUEUE_TYPE_READ)
		return m_pclsIPC->GetReadRingCount();
	else if(a_nType == ATOM_DEF_QUEUE_TYPE_WRITE)
		return m_pclsIPC->GetWriteRingCount();

	return -1;
}

// IPC Func End


//##################################################
// RSA Func
//##################################################
int ATOM_API::GetIPAddress(const char *a_szIfName, int a_nVersion, std::string &a_strResult)
{
	int ret = 0;
	std::string strResult;

	CProtocol *pclsResProtocol;
	CProtocol clsReqProtocol;
	clsReqProtocol.Clear();
	
	char szBuffer[ATOM_DEF_MEM_BUF_64];
	snprintf(szBuffer, sizeof(szBuffer), "%d", a_nVersion);

	try{
        rabbit::object o_root;
        rabbit::object o_body = o_root["BODY"];
        o_body["nic"] = a_szIfName;
        o_body["ip_version"] = szBuffer;

		m_pclsLog->DEBUG("TEST TEST TEST %s", o_root.str().c_str());

		clsReqProtocol.SetFlagRequest();
		clsReqProtocol.SetCommand(CMD_RSA_NIC_IP_QUERY);
		clsReqProtocol.SetPayload(o_root.str());
		clsReqProtocol.SetSource(m_nNodeNo, m_nProcNo);
		clsReqProtocol.SetDestination(m_nNodeNo, PROCID_ATOM_NA_RSA);

		ret = SendCommand(&clsReqProtocol);
		if( ret < 0 )
		{
			m_pclsLog->ERROR("GetIPAddress Req Send Failed");
			return ret;
		}
		
    } catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress, %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress , %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(...) {
        m_pclsLog->ERROR("ATOM API GetIPAddress Parsing Error");
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    }	

	pclsResProtocol = BlockCommand(CMD_RSA_NIC_IP_QUERY, ATOM_DEF_CMD_BLOCK_TIME);

	if(pclsResProtocol == NULL)
	{
		SetErrorMsg(-E_ATOM_RES_RECV_FAIL, "GetIPAddress Response Timeout");
		return -E_ATOM_RES_RECV_FAIL;
	}

	try {
		rabbit::document resDoc;
		rabbit::object o_resBody;

		resDoc.parse(pclsResProtocol->GetPayload());
		o_resBody = resDoc["BODY"];
		
		if( o_resBody["ip_addr"].str().size() != 0 )
		{
			a_strResult.assign(o_resBody["ip_addr"].str());
			return 0;
		}
    } catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress, %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress, %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(...) {
        m_pclsLog->ERROR("ATOM API GetIPAddress Parsing Error");
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    }

		
	return -1;
	
}

int ATOM_API::GetPeerIPAddress(const char *a_szPkgName, const char *a_szNodeType, std::map<std::string, std::string> &a_mapResult)
{
	int ret = 0;
	std::string strResult;

	CProtocol *pclsResProtocol;
	CProtocol clsReqProtocol;
	clsReqProtocol.Clear();
	
	try{
        rabbit::object o_root;
        rabbit::object o_body = o_root["BODY"];
        o_body["pkg_name"] = a_szPkgName;
        o_body["node_type"] = a_szNodeType;


		clsReqProtocol.SetFlagRequest();
		clsReqProtocol.SetCommand(CMD_RSA_PEER_IP_QUERY);
		clsReqProtocol.SetPayload(o_root.str());
		clsReqProtocol.SetSource(m_nNodeNo, m_nProcNo);
		clsReqProtocol.SetDestination(m_nNodeNo, PROCID_ATOM_NA_RSA);

		ret = SendCommand(&clsReqProtocol);
		if( ret < 0 )
		{
			m_pclsLog->ERROR("GetIPAddress Req Send Failed");
			return ret;
		}
		
    } catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress, %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress , %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(...) {
        m_pclsLog->ERROR("ATOM API GetIPAddress Parsing Error");
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    }	

	pclsResProtocol = BlockCommand(CMD_RSA_PEER_IP_QUERY, ATOM_DEF_CMD_BLOCK_TIME);

	if(pclsResProtocol == NULL)
	{
		SetErrorMsg(-E_ATOM_RES_RECV_FAIL, "GetIPAddress Response Timeout");
		return -E_ATOM_RES_RECV_FAIL;
	}

	a_mapResult.clear();
	try {
		rabbit::document resDoc;
		rabbit::object o_resBody;
		rabbit::array a_resList;

		resDoc.parse(pclsResProtocol->GetPayload());
		o_resBody = resDoc["BODY"];
		a_resList = o_resBody["node_list"];

		typedef rabbit::array::iterator iter_t;
		for(iter_t it = a_resList.begin(); it != a_resList.end(); ++it)
		{
			a_mapResult.insert( std::pair<string, string>(it->as_string(), o_resBody[it->as_string()]["ip_addr"].str()) );
		}
				
    } catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress, %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("ATOM API GetIPAddress, %s", e.what());
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    } catch(...) {
        m_pclsLog->ERROR("ATOM API GetIPAddress Parsing Error");
		SetErrorMsg(-E_ATOM_CMD_PARSE_ERR, "GetIPAddress Response Parsing Error");
        return -E_ATOM_CMD_PARSE_ERR;
    }

		
	return 0;
	
}

// RSA Func End

//##################################################
// TRACE Func
//##################################################
bool ATOM_API::TraceIsOn()
{
	return m_pclsTrace->IsOn();
}

bool ATOM_API::CheckTrace()
{
	bool bFind = false;

	m_stTraceInfo.narrTraceReqNo[ATOM_DEF_IDX_TRACE_IMSI] = 
								m_pclsTrace->CheckIMSI(m_stTraceInfo.vecData[ATOM_DEF_IDX_TRACE_IMSI]);
	
	m_stTraceInfo.narrTraceReqNo[ATOM_DEF_IDX_TRACE_MSISDN] = 
								m_pclsTrace->CheckIMSI(m_stTraceInfo.vecData[ATOM_DEF_IDX_TRACE_IMSI]);

	m_stTraceInfo.narrTraceReqNo[ATOM_DEF_IDX_TRACE_CALLER_IP] = 
								m_pclsTrace->CheckIMSI(m_stTraceInfo.vecData[ATOM_DEF_IDX_TRACE_IMSI]);

	m_stTraceInfo.narrTraceReqNo[ATOM_DEF_IDX_TRACE_CALLEE_IP] = 
								m_pclsTrace->CheckIMSI(m_stTraceInfo.vecData[ATOM_DEF_IDX_TRACE_IMSI]);

	for(int i = 0; i < ATOM_DEF_MAX_TRACE_IDX ; i++)
	{
		if(m_stTraceInfo.narrTraceReqNo[i] >= 0)
		{
			bFind = true;
			break;
		}
	}

	return bFind;
}

int ATOM_API::SendTraceData(const char *a_szData)
{
	CProtocol clsResProtocol;
	std::string strData;
	strData.assign(a_szData);

	for(int i = 0; i < ATOM_DEF_MAX_TRACE_IDX ; i++)
	{
		if(m_stTraceInfo.narrTraceReqNo[i] >= 0)
		{
			strData.assign(a_szData);
			clsResProtocol.Clear();
			m_pclsTrace->SetTraceData(&clsResProtocol, m_stTraceInfo.narrTraceReqNo[i], true, strData);
			SendCommand(&clsResProtocol);
		}
	}

	return 0;
}


int ATOM_API::SetTraceArgument(int a_nIdx, const char* a_szData)
{
	if(a_nIdx >= ATOM_DEF_MAX_TRACE_IDX)
	{
		SetErrorMsg(-E_ATOM_MEMORY_OVER, "Trace Argument Idx is Over Max Index");
		return -E_ATOM_MEMORY_OVER;
	}

	m_stTraceInfo.vecData[a_nIdx].assign(a_szData);

	return 0;

}


// Trace Func End




//##################################################
// STAT Func
//##################################################

int ATOM_API::SendStat(char *a_szTable, vector<string> &a_vecPri, vector<int> &a_vecData, bool a_bHist)
{
	unsigned int encLen = 0;
	time_t tCur;
	char szEncBuff[ATOM_DEF_MEM_BUF_1M];
	std::string strReqPayload;

	CStaAnsApi encApi;
	CStaReqApi reqApi;

	CProtocol clsProtocol;

	clsProtocol.Clear();

	if(a_bHist)
	{
		time(&tCur);
		encApi[a_szTable].SetTableInfo(tCur, STAAPI_TABLE_TYPE_HIST);
		clsProtocol.SetCommand(CMD_STS_APP_HIST);
		clsProtocol.SetFlagNotify();
		clsProtocol.SetSource(m_nNodeNo, m_nProcNo);
		clsProtocol.SetDestination(m_nNodeNo, PROCID_ATOM_NA_STA);
		clsProtocol.SetSequence(0);
	}
	else
	{
		strReqPayload = m_pclsReqProtocol->GetPayload();
		reqApi.DecodeMessage(strReqPayload);
		tCur = reqApi.GetTime();

		encApi[a_szTable].SetTableInfo(tCur, STAAPI_TABLE_TYPE_STS);
		clsProtocol.SetResponse(*m_pclsReqProtocol);
	}

	for(uint32_t i = 0; i < a_vecPri.size(); i++)
	{
		encApi[a_szTable].InsertPrimaryKey(a_vecPri[i].c_str());
	}

	for(uint32_t i = 0; i < a_vecData.size(); i++)
	{
		encApi[a_szTable].InsertValue(a_vecData[i]);
	}

	encApi.EncodeMessage(szEncBuff, sizeof(szEncBuff), &encLen);

	clsProtocol.SetPayload(szEncBuff, encLen);

	return this->SendCommand(&clsProtocol);
}
// Stat Func End


//##################################################
// Alarm Func
//##################################################
void ATOM_API::AddProcDateAndDstYn(std::vector<char> &a_vecData)
{
	std::string strTime;
	char cDst;
	
	CTimeUtil::SetTimestampAndDstYn(strTime, cDst, 2);

	a_vecData.insert(a_vecData.end(), strTime.begin(), strTime.end());
	a_vecData.push_back('\0');

	a_vecData.push_back(cDst);
	a_vecData.push_back('\0');

}

int ATOM_API::SendTrap(
						const char * a_szCode, 
						const char *a_szTarget, 
						int			a_nValue, 
						const char *a_szComplement, 
						const char *a_szText
						)
{
	char szBuffer[ATOM_DEF_MEM_BUF_64];
	snprintf(szBuffer, sizeof(szBuffer), "%.15d", a_nValue);
	return SendTrap(a_szCode, a_szTarget, szBuffer, a_szComplement, a_szText);
}


int ATOM_API::SendTrap(
						const char * a_szCode, 
						const char *a_szTarget, 
						double 		a_dValue, 
						const char *a_szComplement, 
						const char *a_szText
						)
{
	char szBuffer[ATOM_DEF_MEM_BUF_64];
	snprintf(szBuffer, sizeof(szBuffer), "%.15f", a_dValue);
	return SendTrap(a_szCode, a_szTarget, szBuffer, a_szComplement, a_szText);
}

int ATOM_API::SendTrap(
						const char * a_szCode, 
						const char *a_szTarget, 
						const char *a_szValue, 
						const char *a_szComplement, 
						const char *a_szText
						)
{
	std::vector<char> vecData;
	vecData.clear();
	vecData.insert(vecData.end(), m_szProcName, m_szProcName + strlen(m_szProcName));
	vecData.push_back('\0');


	if( NULL != a_szCode )
		vecData.insert(vecData.end(), a_szCode, a_szCode + strlen(a_szCode));
	vecData.push_back('\0');

	if( NULL != a_szTarget )
		vecData.insert(vecData.end(), a_szTarget, a_szTarget + strlen(a_szTarget));
	vecData.push_back('\0');

	if( NULL != a_szValue )
		vecData.insert(vecData.end(), a_szValue, a_szValue + strlen(a_szValue));
	vecData.push_back('\0');

	if( NULL != a_szComplement )
		vecData.insert(vecData.end(), a_szComplement, a_szComplement + strlen(a_szComplement));
	vecData.push_back('\0');

	if( NULL != a_szText )
		vecData.insert(vecData.end(), a_szText, a_szText + strlen(a_szText));
	vecData.push_back('\0');

	AddProcDateAndDstYn(vecData);

	return SendNotifyToLocalAPP(CMD_ALM_EVENT, PROCID_ATOM_NA_ALA, vecData);
}

int ATOM_API::PingSuccess(int a_nPeerNo, const char *a_szMyIP, const char *a_szPeerIP)
{
	std::vector<char> vecData;
	vecData.clear();
	vecData.insert(vecData.end(), m_szProcName, m_szProcName + strlen(m_szProcName));
	vecData.push_back('\0');
	
	std::string strTemp(std::to_string((long long)a_nPeerNo));
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');
	vecData.insert(vecData.end(), a_szMyIP, a_szMyIP + strlen(a_szMyIP));
	vecData.push_back('\0');
	vecData.insert(vecData.end(), a_szPeerIP, a_szPeerIP + strlen(a_szPeerIP));
	vecData.push_back('\0');

	strTemp.assign("SUCCESS");
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');
	
	AddProcDateAndDstYn(vecData);

	return SendNotifyToLocalAPP(CMD_STATUS_PING_EVENT, PROCID_ATOM_NA_ALA, vecData);
}

int ATOM_API::PingFail(int a_nPeerNo, const char *a_szMyIP, const char *a_szPeerIP)
{
	std::vector<char> vecData;
	vecData.clear();
	vecData.insert(vecData.end(), m_szProcName, m_szProcName + strlen(m_szProcName));
	vecData.push_back('\0');
	
	std::string strTemp(std::to_string((long long)a_nPeerNo));
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');
	vecData.insert(vecData.end(), a_szMyIP, a_szMyIP + strlen(a_szMyIP));
	vecData.push_back('\0');
	vecData.insert(vecData.end(), a_szPeerIP, a_szPeerIP + strlen(a_szPeerIP));
	vecData.push_back('\0');

	strTemp.assign("FAIL");
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');
	
	AddProcDateAndDstYn(vecData);

	return SendNotifyToLocalAPP(CMD_STATUS_PING_EVENT, PROCID_ATOM_NA_ALA, vecData);
}

int ATOM_API::ConnectReport(const char *a_szMyIP, const char *a_szPeerIP, const char *a_szService)
{
	std::vector<char> vecData;
	vecData.clear();
	vecData.insert(vecData.end(), a_szMyIP, a_szMyIP + strlen(a_szMyIP));
	vecData.push_back('\0');
	vecData.insert(vecData.end(), a_szPeerIP, a_szPeerIP + strlen(a_szPeerIP));
	vecData.push_back('\0');

	vecData.insert(vecData.end(), a_szService, a_szService + strlen(a_szService));
	vecData.push_back('\0');

	std::string strTemp;
	strTemp.assign("CONNECT");
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');
	
	AddProcDateAndDstYn(vecData);

	return SendNotifyToLocalAPP(CMD_STATUS_CONNECT_EVENT, PROCID_ATOM_NA_ALA, vecData);

}

int ATOM_API::DisconnectReport(const char *a_szMyIP, const char *a_szPeerIP, const char *a_szService)
{
	std::vector<char> vecData;

	vecData.clear();
	vecData.insert(vecData.end(), a_szMyIP, a_szMyIP + strlen(a_szMyIP));
	vecData.push_back('\0');

	vecData.insert(vecData.end(), a_szPeerIP, a_szPeerIP + strlen(a_szPeerIP));
	vecData.push_back('\0');

	vecData.insert(vecData.end(), a_szService, a_szService + strlen(a_szService));
	vecData.push_back('\0');

	std::string strTemp;
	strTemp.assign("DISCONNECT");
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');
	
	AddProcDateAndDstYn(vecData);

	return SendNotifyToLocalAPP(CMD_STATUS_CONNECT_EVENT, PROCID_ATOM_NA_ALA, vecData);

}

int ATOM_API::Subscribe(const char *a_szCode, int a_nServerityID, char *a_szPkgName)
{
	std::vector<char> vecData;
	vecData.clear();
	vecData.insert(vecData.end(), a_szCode, a_szCode + strlen(a_szCode));
	vecData.push_back('\0');
	
	std::string strTemp(std::to_string((long long)a_nServerityID));
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');

	vecData.insert(vecData.end(), a_szPkgName, a_szPkgName + strlen(a_szPkgName));
	vecData.push_back('\0');

	AddProcDateAndDstYn(vecData);

	return SendNotifyToLocalAPP(CMD_STATUS_NODE_EVENT, PROCID_ATOM_NA_ALA, vecData);
}

int ATOM_API::RegisterProvider(int a_nCommandID, int a_nDstProc)
{
	std::vector<char> vecData;
	vecData.clear();

	std::string strTemp(std::to_string((long long)m_nNodeNo));
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');

	strTemp =  std::to_string((long long)m_nProcNo);
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');

	strTemp =  std::to_string((long long)a_nCommandID);
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');

	return SendNotifyToLocalAPP(CMD_REGISTER_PROVIDER, PROCID_ATOM_NA_ALA, vecData);
}

int ATOM_API::RegisterTps()
{
	return RegisterProvider(CMD_VNF_PERF_TPS, PROCID_ATOM_NA_EXA);

}

int ATOM_API::SendTps(const char *a_szTpsTitle, const int a_nValue)
{
	std::vector<char> vecData;
	vecData.clear();
	
	vecData.insert(vecData.end(), a_szTpsTitle, a_szTpsTitle + strlen(a_szTpsTitle));
	vecData.push_back('\0');

	std::string strTemp(std::to_string((long long)a_nValue));
	vecData.insert(vecData.end(), strTemp.begin(), strTemp.end());
	vecData.push_back('\0');

	return SendNotifyToLocalAPP(CMD_VNF_PERF_TPS, PROCID_ATOM_NA_ALA, vecData);
}


int ATOM_API::SendNotifyToLocalAPP(int a_nCommand, int a_nDstProc, std::vector<char> &a_vecData)
{
	char szCommand[ATOM_DEF_MEM_BUF_64];
	memset(szCommand, 0x00, sizeof(szCommand));
	snprintf(szCommand, sizeof(szCommand), "%010d", a_nCommand);

	CProtocol clsProtocol;
	clsProtocol.Clear();
	clsProtocol.SetCommand(szCommand);
	clsProtocol.SetFlagNotify();
	clsProtocol.SetSource(m_nNodeNo, m_nProcNo);
	clsProtocol.SetDestination(m_nNodeNo, a_nDstProc);
	clsProtocol.SetSequence(0);
	clsProtocol.SetPayload(a_vecData);	

	return SendCommand(&clsProtocol);
}

// Alarm Func End



void ATOM_API::SetErrorMsg(int a_nErrCode, const char *a_pszFmt, ...)
{
	static char szErrorMsg[DEF_BUFFER_1024];
	va_list ap;
	va_start(ap, a_pszFmt);
	vsprintf(szErrorMsg, a_pszFmt, ap);
	va_end(ap);

	snprintf(m_szErrorMsg, sizeof(m_szErrorMsg), "CODE : %d, ERR_MSG : %s", a_nErrCode,  szErrorMsg);
}

