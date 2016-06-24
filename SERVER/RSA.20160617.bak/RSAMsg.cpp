#include "RSAMsg.hpp"

RSAMsg::RSAMsg(CFileLog *a_pclsLog, CConfig *a_pclsConfig, EventAPI *a_pclsEvent, CModule::ST_MODULE_OPTIONS* a_pstModuleOption)
{
	m_pclsLog = a_pclsLog;
	m_pclsConfig = a_pclsConfig;	
	m_pstModuleOption = a_pstModuleOption;
    m_pclsRootSock = NULL;
    m_pclsEvent = a_pclsEvent;

	m_mapMonitor.clear();
	m_mapSummary.clear();
	m_mapPerf.clear();

	m_strFull.clear();
	m_strSummary.clear();
	m_strStat.clear();
}


RSAMsg::~RSAMsg()
{
	std::map<string, DST_INFO * >::iterator it;
	if(m_mapMonitor.size())
	{
		for(it = m_mapMonitor.begin(); it != m_mapMonitor.end() ; ++it)
		{
			if(it->second)
				delete it->second;
		}
	}


	if(m_mapPerf.size())
	{
		for(it = m_mapPerf.begin(); it != m_mapPerf.end() ; ++it)
		{
			if(it->second)
				delete it->second;
		}

	}

	if(m_mapSummary.size())
	{
		for(it = m_mapSummary.begin(); it != m_mapSummary.end() ; ++it)
		{
			if(it->second)
				delete it->second;
		}

	}

	if(m_pclsRootSock)
		delete m_pclsRootSock;
}

int RSAMsg::Initialize()
{
    if( NULL == m_pclsLog )
        return -1;

    if( NULL == m_pclsConfig )
        return -1;

    if( NULL == m_pstModuleOption )
        return -1;

    if( NULL == m_pclsEvent )
        return -1;

	return 0;
}

int RSAMsg::ProcessCmd(CProtocol *a_pclsProto, time_t a_tCur, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	int nCmd = atoi(a_pclsProto->GetCommand().c_str());
	switch (nCmd) {
		case CMD_PING :
			break;

		case CMD_RSA_PERF_REPORT :
		case CMD_RSA_MONITORING_REPORT :
		case CMD_RSA_SUMMARY_REPORT :
			ParsingReportCommand(a_pclsProto, a_tCur, nCmd);
			break;
		case CMD_STS_RSA :
			m_pclsLog->DEBUG("Recv Sts Rsa Req");
			ProcessStatCommand(a_pclsProto, a_pstRsc, a_nGroupCnt);
			break;
		case CMD_RSA_MONITORING_REPORT_STOP :
		case CMD_RSA_SUMMARY_REPORT_STOP :
			ParsingStopCommand(a_pclsProto, nCmd);
			break;
		case CMD_CLI_COMMAND:
			ProcessCLICommand(a_pclsProto, a_pstRsc, a_nGroupCnt);
			break;
		case CMD_RSA_NIC_IP_QUERY:
			ProcessNicIpCommand(a_pclsProto, a_pstRsc, a_nGroupCnt);
			break;
		case CMD_RSA_PEER_IP_QUERY:
			ProcessPeerIpCommand(a_pclsProto, a_pstRsc, a_nGroupCnt);
			break;
//		case CMD_PROC_LOGLEVEL :
//			break;
	}

	return 0;
}

int RSAMsg::ParsingStopCommand(CProtocol *a_pclsProto, int a_nCmd)
{
	DST_INFO stDstInfo;
	char szBuffer[DEF_MEM_BUF_64];

	std::map<string, DST_INFO *>::iterator it;

	stDstInfo.nSeq = a_pclsProto->GetSequence();	
	a_pclsProto->GetSource(stDstInfo.nNode, stDstInfo.nProc);

	snprintf(szBuffer, sizeof(szBuffer), "%d:%d:%d", stDstInfo.nNode, stDstInfo.nProc, stDstInfo.nSeq); 

	switch(a_nCmd)
	{
		case CMD_RSA_MONITORING_REPORT_STOP :
			m_pclsLog->DEBUG("Recv Monitoring Report Stop Req");
			it = m_mapMonitor.find(szBuffer);
			if( it != m_mapMonitor.end() )
				m_mapMonitor.erase(it);
			break;
		case CMD_RSA_SUMMARY_REPORT_STOP :
			m_pclsLog->DEBUG("Recv Summary Report Stop Req");
			it = m_mapSummary.find(szBuffer);
			if( it != m_mapSummary.end() )
				m_mapSummary.erase(it);
			break;
		default:
			return -1;
	}

	return 0;
}


int RSAMsg::ParsingReportCommand(CProtocol *a_pclsProto, time_t a_tCur, int a_nCmd)
{
	DST_INFO stDstInfo;
	memset(&stDstInfo, 0x00, sizeof(DST_INFO));

	Document docRoot;	

	stDstInfo.nSeq = a_pclsProto->GetSequence();	
	a_pclsProto->GetSource(stDstInfo.nNode, stDstInfo.nProc);


	docRoot.Parse(a_pclsProto->GetPayload().c_str());	
	if(!docRoot.HasMember("BODY"))
	{
		return -1;
	}

	const Value& o_body = docRoot["BODY"];
	if(!o_body.HasMember("period"))
	{
		return -1;
	}
		
	if(!o_body.HasMember("timeout"))
	{
		return -1;
	}

	if(o_body["period"].IsNumber())
	{
		stDstInfo.nPeriod = o_body["period"].GetInt();
	}
	else
	{
		stDstInfo.nPeriod = atoi(o_body["period"].GetString());
	}

	if( 0 == stDstInfo.nPeriod )
	{
		stDstInfo.nPeriod = 5;
	}

	if(o_body["timeout"].IsNumber())
	{
		stDstInfo.tTimeOut = a_tCur + o_body["timeout"].GetInt();
	}
	else
	{
		stDstInfo.tTimeOut = a_tCur + atoi(o_body["timeout"].GetString());
	}

//	stDstInfo.tNextCheck = a_tCur - (a_tCur % stDstInfo.nPeriod) + stDstInfo.nPeriod;

	switch(a_nCmd)
	{
		case CMD_RSA_PERF_REPORT :
			m_pclsLog->DEBUG("Recv Performance Report Req");
			RegistPerformanceDestination(&stDstInfo);
			break;
		case CMD_RSA_MONITORING_REPORT :
			m_pclsLog->DEBUG("Recv Monitoring Report Req");
			RegistMonitoringDestination(&stDstInfo);
			break;
		case CMD_RSA_SUMMARY_REPORT :
			m_pclsLog->DEBUG("Recv Summary Report Req");
			RegistSummaryDestination(&stDstInfo);
			break;
		
		default:
			return -1;
	}

	return 0;
}


int RSAMsg::RegistMonitoringDestination(DST_INFO *a_pstDstInfo)
{
	DST_INFO *pstDstInfo;
	char szBuffer[DEF_MEM_BUF_64];

	snprintf(szBuffer, sizeof(szBuffer), "%d:%d:%d", a_pstDstInfo->nNode, a_pstDstInfo->nProc, a_pstDstInfo->nSeq); 

	std::map<string, DST_INFO *>::iterator it;
	it = m_mapMonitor.find(szBuffer);

	if(it == m_mapMonitor.end())
	{
		pstDstInfo = new DST_INFO;
		memcpy(pstDstInfo, a_pstDstInfo, sizeof(DST_INFO));
		m_mapMonitor.insert( std::pair<string, DST_INFO *>(szBuffer, pstDstInfo) );
	}	
	else
	{
		pstDstInfo = it->second;
		memcpy(pstDstInfo, a_pstDstInfo, sizeof(DST_INFO));
	}

	return 0;
}

int RSAMsg::RegistSummaryDestination(DST_INFO *a_pstDstInfo)
{
	DST_INFO *pstDstInfo;
	char szBuffer[DEF_MEM_BUF_64];

	snprintf(szBuffer, sizeof(szBuffer), "%d:%d:%d", a_pstDstInfo->nNode, a_pstDstInfo->nProc, a_pstDstInfo->nSeq); 

	std::map<string, DST_INFO *>::iterator it;
	it = m_mapSummary.find(szBuffer);

	if(it == m_mapSummary.end())
	{
		pstDstInfo = new DST_INFO;
		memcpy(pstDstInfo, a_pstDstInfo, sizeof(DST_INFO));
		m_mapSummary.insert( std::pair<string, DST_INFO *>(szBuffer, pstDstInfo) );
	}	
	else
	{
		pstDstInfo = it->second;
		memcpy(pstDstInfo, a_pstDstInfo, sizeof(DST_INFO));
	}

	return 0;

}

int RSAMsg::RegistPerformanceDestination(DST_INFO *a_pstDstInfo)
{
	DST_INFO *pstDstInfo;
	char szBuffer[DEF_MEM_BUF_64];

	snprintf(szBuffer, sizeof(szBuffer), "%d:%d:%d", a_pstDstInfo->nNode, a_pstDstInfo->nProc, a_pstDstInfo->nSeq); 

	std::map<string, DST_INFO *>::iterator it;
	it = m_mapPerf.find(szBuffer);

	if(it == m_mapPerf.end())
	{
		pstDstInfo = new DST_INFO;
		memcpy(pstDstInfo, a_pstDstInfo, sizeof(DST_INFO));
		m_mapPerf.insert( std::pair<string, DST_INFO *>(szBuffer, pstDstInfo) );
	}	
	else
	{
		pstDstInfo = it->second;
		memcpy(pstDstInfo, a_pstDstInfo, sizeof(DST_INFO));
	}

	return 0;
}

int RSAMsg::ConnectRoot()
{
	if( NULL != m_pclsRootSock )
		return 0;	

	int nRootPort = 0;
	nRootPort = atoi(m_pclsConfig->GetConfigValue("RSA", "ROOT_PORT"));

	m_pclsRootSock = new CMesgExchSocket();
	if(NULL == m_pclsRootSock)
	{
		m_pclsLog->ERROR("Fail to Create Root Socket");
		return -1;
	}

	if( false == (m_pclsRootSock->Connect("127.0.0.1", nRootPort)) )
	{
		m_pclsLog->ERROR("Fail to Connect Root Socket");
		delete m_pclsRootSock;
		m_pclsRootSock = NULL;
		return -1;
	}

	SendInitMsgToRoot();
	
	return 0;

}

int RSAMsg::SendInitMsgToRoot()
{
	char szBuffer[DEF_MEM_BUF_64];
	try {
		rabbit::object o_root;
		rabbit::object o_body = o_root["BODY"];

		snprintf(szBuffer, sizeof(szBuffer), "%d", m_pstModuleOption->m_nNodeNo);
		o_body["NODE_NO"] = szBuffer;

		snprintf(szBuffer, sizeof(szBuffer), "%s", m_pstModuleOption->m_szPkgName);
		o_body["PKG_NAME"] = szBuffer;

		snprintf(szBuffer, sizeof(szBuffer), "%s", m_pstModuleOption->m_szNodeType);
		o_body["NODE_TYPE"] = szBuffer;

		snprintf(szBuffer, sizeof(szBuffer), "%d", CMD_RSA_ROOT_INIT);
		m_pclsRootSock->SetCommand(szBuffer);
		m_pclsRootSock->SetFlagRequest();
		m_pclsRootSock->SetSource(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
		m_pclsRootSock->SetDestination(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
		m_pclsRootSock->SetPayload(o_root.str().c_str());

	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("RSAMsg SendInitMsgToRoot, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("RSAMsg SendInitMsgToRoot, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("RSAMsg SendInitMsgToRoot Parsing Error");
        return -1;
    }
 

	if (m_pclsRootSock->SendMesg() == false) {
		m_pclsLog->ERROR("message send failed\n");
		m_pclsLog->ERROR("- %s\n", m_pclsRootSock->CSocket::m_strErrorMsg.c_str());

		delete m_pclsRootSock;
		m_pclsRootSock = NULL;

		return -1;
	}

	return 0;
}


int RSAMsg::SendRequestToRoot(RESOURCE *a_pstRsc, int a_nCmd, time_t a_tCur)
{
	char szBuffer[DEF_MEM_BUF_64];

	if(ConnectRoot() < 0)
		return -1;

	try {
		rabbit::object o_root;
		rabbit::object o_body = o_root["BODY"];
		o_body["RESOURCE"] = a_pstRsc->szGroupName;

		snprintf(szBuffer, sizeof(szBuffer), "%d", m_pstModuleOption->m_nNodeNo);
		o_body["NODE_NO"] = szBuffer;

		snprintf(szBuffer, sizeof(szBuffer), "%ld", a_tCur);
		o_body["TIME"] = szBuffer;

		snprintf(szBuffer, sizeof(szBuffer), "%d", a_nCmd);
		m_pclsRootSock->SetCommand(szBuffer);
		m_pclsRootSock->SetFlagRequest();
		m_pclsRootSock->SetSource(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
		m_pclsRootSock->SetDestination(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
		m_pclsRootSock->SetPayload(o_root.str().c_str());
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("RSAMsg SendRequestToRoot, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("RSAMsg SendRequestToRoot, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("RSAMsg SendRequestToRoot Parsing Error");
        return -1;
    }
 

	if (m_pclsRootSock->SendMesg() == false) {
		m_pclsLog->ERROR("message send failed\n");
		m_pclsLog->ERROR("- %s\n", m_pclsRootSock->CSocket::m_strErrorMsg.c_str());

		delete m_pclsRootSock;
		m_pclsRootSock = NULL;

		return -1;
	}

	return 0;
}

int RSAMsg::RecvMesgFromRoot(RESOURCE *a_pstRsc)
{
	if(ConnectRoot() < 0)
		return -1;

	if(m_pclsRootSock->RecvMesg(NULL, 5) < 0)
	{
		m_pclsLog->ERROR("message receive failed, errno=%d", errno);
		m_pclsLog->ERROR("Socket, %s", m_pclsRootSock->CSocket::m_strErrorMsg.c_str());
		m_pclsLog->ERROR("Protocol, %s", m_pclsRootSock->CProtocol::m_strErrorMsg.c_str());

		delete m_pclsRootSock;
		m_pclsRootSock = NULL;

		return -1;
	}

	m_pclsRootSock->Print(m_pclsLog, LV_DEBUG, true);

	try {
	    rabbit::document doc;
	    doc.parse(m_pclsRootSock->GetPayload());

    	rabbit::object o_full = doc["FULL"];
	    rabbit::object o_trap = doc["TRAP"];

		a_pstRsc->strFullJson.clear();
		a_pstRsc->strStatJson.clear();

		switch(atoi(m_pclsRootSock->GetCommand().c_str()))
		{
			case CMD_RSA_ROOT_MSG :
        	    if(o_full.str().size() > 2)
    				a_pstRsc->strFullJson.assign(o_full.str());
	            if(o_trap.str().size() > 2)
    	            SendTrapMsg(a_pstRsc, o_trap.str().c_str());
				break;
			case CMD_RSA_ROOT_STAT :
				a_pstRsc->strStatJson.assign(m_pclsRootSock->GetPayload());
			break;
//        case CMD_RSA_ROOT_TRAP :
//            SendTrapMsg(m_pclsRootSock->GetPayload().c_str());
		}
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("RSAMsg RecvMesgFromRoot, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("RSAMsg RecvMesgFromRoot, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("RSAMsg RecvMesgFromRoot Parsing Error");
        return -1;
    }


	return 0;
}

int RSAMsg::SendTrapMsg(RESOURCE *a_pstRsc, const char* a_strMsg)
{
	try {
	    rabbit::document doc;
	    doc.parse(a_strMsg);

    	rabbit::array a_list = doc["LIST"];
    	rabbit::object o_attr;
	    rabbit::object o_code;
    	rabbit::object o_target;
	    rabbit::object o_value;

//    	RESOURCE_ATTR *pstRsc = NULL;

		typedef rabbit::array::iterator iter_t;
		for(iter_t it = a_list.begin(); it != a_list.end(); ++it)
		{
        	o_attr = doc[it->as_string()];
	        o_code = o_attr["CODE"];
    	    o_target = o_attr["TARGET"];
        	o_value = o_attr["VALUE"];

	        m_pclsLog->DEBUG("SendTrap : Code, %s, Target, %s, Value, %s", o_code.str().c_str(), o_target.str().c_str(), o_value.str().c_str());
    	    m_pclsEvent->SendTrap(
        	                    o_code.str().c_str(),
            	                o_target.str().c_str(),
                	            o_value.str().c_str(),
                    	        NULL,
                        	    NULL
	                         );
			

		}

#if 0
	    map<string, RESOURCE_ATTR *>::iterator it;
    	for(it = a_pstRsc->mapRsc.begin(); it != a_pstRsc->mapRsc.end(); ++it)
	    {
    	    pstRsc = it->second;
        	o_attr = doc[pstRsc->szName];
	        o_code = o_attr["CODE"];
    	    o_target = o_attr["TARGET"];
        	o_value = o_attr["VALUE"];

	        m_pclsLog->DEBUG("SendTrap : Code, %s, Target, %s, Value, %s", o_code.str().c_str(), o_target.str().c_str(), o_value.str().c_str());
    	    m_pclsEvent->SendTrap(
        	                    o_code.str().c_str(),
            	                o_target.str().c_str(),
                	            o_value.str().c_str(),
                    	        NULL,
                        	    NULL
	                         );
    	}
#endif
	} catch(rabbit::type_mismatch &e) {
        m_pclsLog->ERROR("RSAMsg SendTrapMsg, %s", e.what());
        return -1;
    } catch(rabbit::parse_error &e) {
        m_pclsLog->ERROR("RSAMsg SendTrapMsg, %s", e.what());
        return -1;
    } catch(...) {
        m_pclsLog->ERROR("RSAMsg SendTrapMsg Parsing Error");
        return -1;
    }


    return 0;
}

int RSAMsg::SendMonitoringMsg(DST_INFO *a_pstDstInfo, const char* a_szPayload)
{
	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;

	CProtocol clsResp;
	clsResp.Clear();
	clsResp.SetCommand(CMD_RSA_MONITORING_REPORT);
	clsResp.SetSequence(a_pstDstInfo->nSeq);
	clsResp.SetFlagResponse();

	clsResp.SetSource(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
	clsResp.SetDestination(a_pstDstInfo->nNode, a_pstDstInfo->nProc);
	clsResp.SetPayload(a_szPayload);

	if(clsIPC->SendMesg(clsResp) == false)
	{
		m_pclsLog->ERROR("Failed to Send Monitoring Msg : [%s]\n", clsIPC->m_strErrorMsg.c_str());
	}

	return 0;

}
int RSAMsg::SendSummaryMsg(DST_INFO *a_pstDstInfo, const char* a_szPayload)
{
	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;

	CProtocol clsResp;
	clsResp.Clear();
	clsResp.SetCommand(CMD_RSA_SUMMARY_REPORT);
	clsResp.SetSequence(a_pstDstInfo->nSeq);
	clsResp.SetFlagResponse();

	clsResp.SetSource(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
	clsResp.SetDestination(a_pstDstInfo->nNode, a_pstDstInfo->nProc);
	clsResp.SetPayload(a_szPayload);

	if(clsIPC->SendMesg(clsResp) == false)
	{
		m_pclsLog->ERROR("Failed to Send Summary Msg : [%s]\n", clsIPC->m_strErrorMsg.c_str());
	}

	return 0;

}

int RSAMsg::SendPerformanceMsg(DST_INFO *a_pstDstInfo, const char* a_szPayload)
{
	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;

	CProtocol clsResp;
	clsResp.Clear();
	clsResp.SetCommand(CMD_RSA_PERF_REPORT);
	clsResp.SetSequence(a_pstDstInfo->nSeq);
	clsResp.SetFlagResponse();

	clsResp.SetSource(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
	clsResp.SetDestination(a_pstDstInfo->nNode, a_pstDstInfo->nProc);
	clsResp.SetPayload(a_szPayload);

	if(clsIPC->SendMesg(clsResp) == false)
	{
		m_pclsLog->ERROR("Failed to Send Performance Msg : [%s]\n", clsIPC->m_strErrorMsg.c_str());
	}

	return 0;

}


int RSAMsg::SendResponseMsg(time_t a_tCur, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	time_t tCur;
	struct tm *pstTime;

	tCur = a_tCur - (a_tCur % 5);

	pstTime = localtime( &tCur );

	SendMonitoringInfo(a_tCur, pstTime, a_pstRsc, a_nGroupCnt);
	SendSummaryInfo(a_tCur, pstTime, a_pstRsc, a_nGroupCnt);
	SendPerformanceInfo(a_tCur, pstTime, a_pstRsc, a_nGroupCnt);
	
	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		a_pstRsc[i].unExec &= DEF_EXEC_UNSET_FULL;
		a_pstRsc[i].unExec &= DEF_EXEC_UNSET_SUMMARY;
	}

	return 0;
}


int RSAMsg::MakeFullJson(struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt, bool a_bVNFM)
{
	char szBuff[DEF_MEM_BUF_128];

	Document docRoot;
	docRoot.SetObject();
	Document::AllocatorType &allocator = docRoot.GetAllocator();

	Document docCopy;

	Value vBody(kObjectType);

	memset(szBuff, 0x00, sizeof(szBuff));
	strftime(szBuff, sizeof(szBuff), "%Y-%m-%d %H:%M:%S", a_pstCur);

	vBody.AddMember("time", StringRef(szBuff), allocator);

	if( 1 != a_pstCur->tm_isdst)
	{
		vBody.AddMember("dst_yn", "N", allocator);
	}
	else
	{
		vBody.AddMember("dst_yn", "Y", allocator);
	}

	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		if( 0 == (DEF_EXEC_SET_FULL & a_pstRsc[i].unExec) )
			continue;

		if(a_pstRsc[i].strFullJson.size() == 0)
			continue;

		if(a_bVNFM)
		{
			if( false == a_pstRsc[i].bVNFM )
				continue;
		}

		docCopy.Parse(a_pstRsc[i].strFullJson.c_str());

		Value vGroup(kObjectType);
		vGroup.Swap(docCopy);

		vBody.AddMember(StringRef(a_pstRsc[i].szGroupID), vGroup, allocator);

//		a_pstRsc[i].unExec &= DEF_EXEC_UNSET_FULL;
	}

	docRoot.AddMember("BODY", vBody, allocator);

	StringBuffer strbuf;
	Writer<StringBuffer> w(strbuf);
	docRoot.Accept(w);

	m_pclsLog->DEBUG("Doc => %s", strbuf.GetString());

	m_strFull.assign(strbuf.GetString());
	return 0;
}

int RSAMsg::MakeSummaryJson(struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	Document docRoot;
	docRoot.SetObject();
	Document::AllocatorType &allocator = docRoot.GetAllocator();

	Document docCopy;

	Value vBody(kObjectType);
	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		if( 0 == (DEF_EXEC_SET_SUMMARY & a_pstRsc[i].unExec) )
			continue;

		if(a_pstRsc[i].strSummaryJson.size() == 0)
			continue;

		docCopy.Parse(a_pstRsc[i].strSummaryJson.c_str());

		Value vGroup(kObjectType);
		vGroup.Swap(docCopy);

		vBody.AddMember(StringRef(a_pstRsc[i].szGroupID), vGroup, allocator);


//		a_pstRsc[i].unExec &= DEF_EXEC_UNSET_SUMMARY;

	}

	docRoot.AddMember("BODY", vBody, allocator);

	StringBuffer strbuf;
	Writer<StringBuffer> w(strbuf);
	docRoot.Accept(w);

	m_pclsLog->DEBUG("Summary => %s", strbuf.GetString());
	m_strSummary.assign(strbuf.GetString());

	return 0;
}

int RSAMsg::ProcessStatCommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	Document docRecv;
	RSABase *pclsRSA;
	int nSessionID = 0;
	time_t tStatTime = 0;


	docRecv.Parse(a_pclsProto->GetPayload().c_str());

	if( !docRecv.HasMember("BODY") )
	{
		return -1;
	}
	
	const Value& o_RecvBody = docRecv["BODY"];
	if( !o_RecvBody.HasMember("session_id") )
	{
		return -1;
	}

	if( !o_RecvBody.HasMember("collect_time") )
	{
		return -1;
	}

	nSessionID = o_RecvBody["session_id"].GetInt();
	tStatTime = o_RecvBody["collect_time"].GetInt();

	Document docRoot;
	docRoot.SetObject();
	Document::AllocatorType &allocator = docRoot.GetAllocator();

	Value vBody(kObjectType);
	Value vArray(kArrayType);
	vBody.AddMember("session_id", nSessionID, allocator);
	vBody.AddMember("result_code",CStmResApi::RESAPI_RESULT_CODE_SUCCESS, allocator);

	Document docCopy;
	
	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		if( NULL == a_pstRsc[i].pclsRSA )
			continue;

		if( false == a_pstRsc[i].bStat )
			continue;

		pclsRSA = (RSABase*)a_pstRsc[i].pclsRSA;

		pclsRSA->MakeStatJson(tStatTime);

		if(a_pstRsc[i].strStatJson.size() == 0)
			continue;

		docCopy.Parse(a_pstRsc[i].strStatJson.c_str());

		Value& o_CopyBody = docCopy["BODY"];
		Value& a_CopyRsc = o_CopyBody["res_group_array"];
	
		for(uint32_t j = 0; j < a_CopyRsc.Size(); j++)
		{
			Value& o_CopyAttr = a_CopyRsc[j];			
			vArray.PushBack((Value&)o_CopyAttr, allocator);
		}
	}

	vBody.AddMember("res_group_array", vArray, allocator);	
	docRoot.AddMember("BODY", vBody, allocator);	

	StringBuffer strbuf;
	Writer<StringBuffer> w(strbuf);
	docRoot.Accept(w);

	m_pclsLog->DEBUG("Stat => %s", strbuf.GetString());
//	printf("Stat => %s\n", strbuf.GetString());
	m_strStat.assign(strbuf.GetString());

	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;

	CProtocol clsResp;
	clsResp.Clear();
	clsResp.SetCommand(a_pclsProto->GetCommand().c_str());
	clsResp.SetSequence(a_pclsProto->GetSequence());
	clsResp.SetFlagResponse();

	clsResp.SetSource(m_pstModuleOption->m_nNodeNo, PROCID_ATOM_NM_RSA);
	clsResp.SetDestination(a_pclsProto->GetSource());
	clsResp.SetPayload(strbuf.GetString());

	clsIPC->SendMesg(clsResp);

	return 0;
}

int RSAMsg::ProcessPeerIpCommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	std::string strPkgName;
	std::string strNodeType;

	strPkgName.clear();
	strNodeType.clear();

	try {
	    rabbit::document doc;
		rabbit::object o_body;

	    doc.parse(a_pclsProto->GetPayload());
		o_body = doc["BODY"];

		strPkgName.assign(o_body["pkg_name"].str());
		strNodeType.assign(o_body["node_type"].str());

	} catch(rabbit::type_mismatch &e) {
		m_pclsLog->ERROR("ProcessPeerIpCommand, %s", e.what());
		return -1;
	} catch(rabbit::parse_error &e) {
		m_pclsLog->ERROR("ProcessPeerIpCommand, %s", e.what());
		return -1;
	} catch(...) {
		m_pclsLog->ERROR("ProcessPeerIpCommand Parsing Error");
		return -1;
	}

	bool bFind = false;
	std::string strResult;

	RSABase *pclsRSA;

	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		if( strncmp(a_pstRsc[i].szGroupID, DEF_GRP_ID_PING, strlen(DEF_GRP_ID_PING) ) != 0 )
			continue;

		bFind = true;

		if( NULL == a_pstRsc[i].pclsRSA )
		{
			return -1;
		}

		pclsRSA = (RSABase*)a_pstRsc[i].pclsRSA;

		pclsRSA->MakeNodeJson(strPkgName.c_str(), strNodeType.c_str(), strResult);

		if(strResult.size() == 0)
		{
			return -1;
		}

		break;
	}

	if(bFind == false)
	{
		return -1;
	}

	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;
	CProtocol clsResp;

	clsResp.Clear();
	clsResp.SetResponse(*a_pclsProto);
	clsResp.SetPayload(strResult);

	clsIPC->SendMesg(clsResp);
	clsResp.Print(m_pclsLog, LV_DEBUG, true);

	return 0;
}

int RSAMsg::ProcessNicIpCommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	std::string strIfName;
	std::string strVersion;

	strIfName.clear();
	strVersion.clear();

	try {
	    rabbit::document doc;
		rabbit::object o_body;
		rabbit::object o_session;
		rabbit::object o_cmd;

	    doc.parse(a_pclsProto->GetPayload());
		o_body = doc["BODY"];
		strIfName.assign(o_body["nic"].str());
		strVersion.assign(o_body["ip_version"].str());

	} catch(rabbit::type_mismatch &e) {
		m_pclsLog->ERROR("ProcessNicIpCommand, %s", e.what());
		return -1;
	} catch(rabbit::parse_error &e) {
		m_pclsLog->ERROR("ProcessNicIpCommand, %s", e.what());
		return -1;
	} catch(...) {
		m_pclsLog->ERROR("ProcessNicIpCommand Parsing Error");
		return -1;
	}

	bool bFind = false;
	std::string strResult;

	RSABase *pclsRSA;

	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		if( strncmp(a_pstRsc[i].szGroupID, DEF_GRP_ID_NIC, strlen(DEF_GRP_ID_NIC) ) != 0 )
			continue;

		bFind = true;

		if( NULL == a_pstRsc[i].pclsRSA )
		{
			return -1;
		}

		pclsRSA = (RSABase*)a_pstRsc[i].pclsRSA;

		pclsRSA->MakeIPJson(strIfName.c_str(), strVersion.c_str(), strResult);

		if(strResult.size() == 0)
		{
			return -1;
		}

		break;
	}

	m_pclsLog->DEBUG("strResult %s\n", strResult.c_str());
	if(bFind == false)
	{
		return -1;
	}

	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;
	CProtocol clsResp;

	clsResp.Clear();
	clsResp.SetResponse(*a_pclsProto);
	clsResp.SetPayload(strResult);

	int ret = 0;
	ret = clsIPC->SendMesg(clsResp);
	m_pclsLog->DEBUG("result is 222222222222222222   %d", ret);
	clsResp.Print(m_pclsLog, LV_DEBUG, true);

	return 0;
}


int RSAMsg::ProcessCLICommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt)
{

	int nSessionID = 0;

	try {
	    rabbit::document doc;
		rabbit::object o_body;
		rabbit::object o_session;
		rabbit::object o_cmd;

	    doc.parse(a_pclsProto->GetPayload());
		o_body = doc["BODY"];

		if( strncmp(o_body["command"].str().c_str(), "DISP-EXT", strlen("DISP-EXT")) != 0 )
		{
			m_pclsLog->ERROR("ProcessCLICommand, Wrong Command Recv (%s)", o_body["command"].str().c_str());
			return -1;
		}
		
		nSessionID = o_body["session_id"].as_int();
		
	} catch(rabbit::type_mismatch &e) {
		m_pclsLog->ERROR("ProcessCLICommand, %s", e.what());
		return -1;
	} catch(rabbit::parse_error &e) {
		m_pclsLog->ERROR("ProcessCLICommand, %s", e.what());
		return -1;
	} catch(...) {
		m_pclsLog->ERROR("ProcessCLICommand Parsing Error");
		return -1;
	}


	bool bFind = false;
	std::string strResult;
	CCliRsp decApi;

	RSABase *pclsRSA;

	for(int i = 0; i < a_nGroupCnt ; i++)
	{
		if( strncmp(a_pstRsc[i].szGroupID, DEF_GRP_ID_PING, strlen(DEF_GRP_ID_PING) ) != 0 )
			continue;

		bFind = true;

		if( NULL == a_pstRsc[i].pclsRSA )
		{
			decApi.SetSessionId(nSessionID);
			decApi.SetSeqType(CCliRsp::SEQ_TYPE_END);
			decApi.SetResultCode(0, "Failed to Get Ping Result");
			break;
		}

		pclsRSA = (RSABase*)a_pstRsc[i].pclsRSA;

		pclsRSA->MakePingJson(strResult);

		if(strResult.size() == 0)
		{
			decApi.SetSessionId(nSessionID);
			decApi.SetSeqType(CCliRsp::SEQ_TYPE_END);
			decApi.SetResultCode(0, "Failed to Get Ping Result");
			break;
		}

		decApi.DecodeMessage(strResult);
		decApi.SetSessionId(nSessionID);
		decApi.SetSeqType(CCliRsp::SEQ_TYPE_END);
		decApi.SetResultCode(1, "Success");
		
	}

	if(bFind == false)
	{
		decApi.SetSessionId(nSessionID);
		decApi.SetSeqType(CCliRsp::SEQ_TYPE_END);
		decApi.SetResultCode(0, "Failed to Find Ping Module");
	}

	strResult.clear();
	decApi.EncodeMessage(strResult);

	CModuleIPC* clsIPC = m_pstModuleOption->m_pclsModIpc;
	CProtocol clsResp;

	clsResp.Clear();
	clsResp.SetResponse(*a_pclsProto);
	clsResp.SetPayload(strResult);

	clsIPC->SendMesg(clsResp);
	clsResp.Print(m_pclsLog, LV_DEBUG, true);
		
	return 0;
}

int RSAMsg::SendMonitoringInfo(time_t a_tCur, struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	std::map<string, DST_INFO *>::iterator it;

	MakeFullJson(a_pstCur, a_pstRsc, a_nGroupCnt, false);
	for(it = m_mapMonitor.begin(); it != m_mapMonitor.end(); ++it)
	{
		if(it->second->tNextCheck <= a_tCur)
		{
			SendMonitoringMsg(it->second, m_strFull.c_str());
			
			it->second->tNextCheck = a_tCur - (a_tCur % it->second->nPeriod) + it->second->nPeriod;

			if( it->second->tNextCheck > it->second->tTimeOut )
				m_mapMonitor.erase(it);
		}
	}	


	return 0;
}


int RSAMsg::SendSummaryInfo(time_t a_tCur, struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	std::map<string, DST_INFO *>::iterator it;

	MakeSummaryJson(a_pstCur, a_pstRsc, a_nGroupCnt);
	for(it = m_mapSummary.begin(); it != m_mapSummary.end(); ++it)
	{
		if(it->second->tNextCheck <= a_tCur)
		{
			SendSummaryMsg(it->second, m_strSummary.c_str());
			
			it->second->tNextCheck = a_tCur - (a_tCur % it->second->nPeriod) + it->second->nPeriod;

			if(it->second->tNextCheck > it->second->tTimeOut)
				m_mapSummary.erase(it);
		}
	}	


	return 0;
}

int RSAMsg::SendPerformanceInfo(time_t a_tCur, struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt)
{
	std::map<string, DST_INFO *>::iterator it;

	MakeFullJson(a_pstCur, a_pstRsc, a_nGroupCnt, true);
	for(it = m_mapPerf.begin(); it != m_mapPerf.end(); ++it)
	{
		if(it->second->tNextCheck <= a_tCur)
		{
			SendPerformanceMsg(it->second, m_strFull.c_str());
			
			it->second->tNextCheck = a_tCur - (a_tCur % it->second->nPeriod) + it->second->nPeriod;

			if(it->second->tNextCheck > it->second->tTimeOut)
				m_mapPerf.erase(it);
		}
	}	

	return 0;
}

