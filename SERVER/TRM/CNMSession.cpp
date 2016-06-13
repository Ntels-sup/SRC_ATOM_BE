#include "CMain.hpp"
#include "TRM_Define.hpp"
#include "CScheduler.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <string>
#include <cerrno>
#include <cstdlib>
#include <vector>
#include <iostream>

using namespace rapidjson;
using std::cout;
using std::endl;
using std::string;


using std::string;
using std::vector;

//extern symbol
extern CFileLog* g_pcLog;
extern CConfigTRM g_pcCFG;

CNMSession::CNMSession()
{
	m_pDB = NULL;

	if(Initial() == TRM_OK)
	{
		m_nStartFlag = TRM_OK;
	}
	else
	{
		m_nStartFlag = TRM_NOK;
	}
}

CNMSession::~CNMSession()
{
	Final();

}

int CNMSession::Initial()
{
	// Connect to MN
    if(m_sock.Connect(g_pcCFG.NM.m_strNMAddr.c_str(), g_pcCFG.NM.m_nNMPort) == false)
//    if(m_sock.Connect("127.0.0.1", 10000) == false)
    {	
        printf("Connection Failed : IP : %s, Port : %d\n", g_pcCFG.NM.m_strNMAddr.c_str(), g_pcCFG.NM.m_nNMPort );
        return TRM_NOK;
    }

	return TRM_OK;
}
int CNMSession::Regist(ST_COWORK_INFO *a_coworkinfo)
{
	int 		nret = 0;

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->nm_proc_no);             // Src NM
    m_sock.SetFlagRequest();
	m_sock.SetCommand(TRM_REGIST);

    rabbit::object  menu = m_root["BODY"];

    menu["pkgname"]    = a_coworkinfo->pkg_name;
    menu["nodetype"]    = a_coworkinfo->node_type;
    menu["procname"]    = g_pcCFG.LOG.m_strProcName;
    menu["procno"]    = a_coworkinfo->trm_proc_no;

	m_sock.SetPayload(m_root.str());

    g_pcLog->INFO("Send TRM Registration Msg");

	nret = m_sock.SendMesg();
	
    if (nret < 0) {
        g_pcLog->ERROR("Fail to Send Message : %s", m_sock.CSocket::m_strErrorMsg.c_str());
        return TRM_NOK;
    }

	return TRM_OK;
}

int CNMSession::Run()
{
	g_pcLog->INFO("CNMSession Run");

	if(m_nStartFlag == TRM_NOK)
		return TRM_NOK;

	return TRM_OK;
}

int CNMSession::RecvMsg()
{
    int  			nRecvFlag = 0;
	int  			nRetRecv = 0;
#ifdef TRM_DEBUG
	char 			imsi[256];
	int  			nLength = 0;
	string 			strimsi;
	string 			strpayload;
#endif
    string 			strBuf;

    if( m_sock.IsConnected() == false)
    {
        Initial();
    }

	nRecvFlag = m_sock.RecvMesg(NULL, -1);
	if(nRecvFlag < 0)
	{
        g_pcLog->ERROR("message receive failed errorno=%d, socket [%s]", 
									errno, m_sock.CSocket::m_strErrorMsg.c_str());
        printf("socket, %s\n", m_sock.CSocket::m_strErrorMsg.c_str());

		m_sock.Clear();

		return TRM_RECV_ERROR;
	}

    m_sock.GetCommand(strBuf);
#ifdef TRM_DEBUG
	// header
    g_pcLog->INFO("Version: %d", m_sock.GetVersion());
    g_pcLog->INFO("Command: %s", strBuf.c_str());

	strpayload = m_sock.GetPayload();
    g_pcLog->INFO("payload: [%s]", strpayload.c_str());
	strimsi = m_sock.GetFlag();
    g_pcLog->INFO("flag: %s", strimsi.c_str());
	nLength = m_sock.GetLength();
    g_pcLog->INFO("Length: %d", nLength);
	strcpy(imsi, strBuf.c_str());
#endif

    // Receive
	if(strncmp(TRM_REGIST, strBuf.c_str(), strBuf.length()) == 0) 	// TRACE_ON
	{
		return TRM_RECV_REGIST;
	}
	if(strncmp(TRM_TRACE_ON, strBuf.c_str(), strBuf.length()) == 0) 	// TRACE_ON
	{
		return TRM_RECV_TRACE_ON;
	}
	else if(strncmp(TRM_TRACE_OFF, strBuf.c_str(), strBuf.length()) == 0) 	// TRACE_OFF
	{
		return TRM_RECV_TRACE_OFF;
	}
	else if(strncmp(TRM_TRACE_DATA, strBuf.c_str(), strBuf.length()) == 0) 	// TRACE_DATA
	{
		return TRM_RECV_TRACE_DATA;
	}
	else if(strncmp(ATOM_PROC_CTL, strBuf.c_str(), strBuf.length()) == 0) 	// PROC_CTRL
	{
        g_pcLog->INFO("TRM_RECV_PROC_CTRL");
        nRetRecv = ProcessCtrl();
        return nRetRecv;
	}
	else if(strncmp(TRM_TRACE_RECONFIG, strBuf.c_str(), strBuf.length()) == 0) 	// PROC_CTRL
	{
        return TRM_RECV_INIT;
	}
	else if(strncmp(TRM_TRACE_REGIST_ROUTE, strBuf.c_str(), strBuf.length()) == 0) 	// PROC_CTRL
	{
        return TRM_RECV_REGIST_ROUTE;
	}
	else
	{
		m_sock.Clear();
		return TRM_NOK;
	}

	return TRM_OK;
}

int CNMSession::ProcessCtrl()
{
    std::string			strLogPayload;
	std::string			strAction;
	int					nLogLevel = 0;

    strLogPayload = m_sock.GetPayload();

    g_pcLog->INFO("request: [%s]", strLogPayload.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strLogPayload.c_str());

        strAction = string(doc["BODY"]["action"].as_string());
        nLogLevel = doc["BODY"]["loglevel"].as_int();

#ifdef TRM_DEBUG
        cout << "[BODY][action]    :" << "[" << strAction << "]" << endl;
        cout << "[BODY][LogLevel]    :" << "[" << nLogLevel << "]" << endl;
#endif
        if(strAction.compare(TRM_ACTION_LOGLEVEL) == 0)
        {
            g_pcLog->SetLogLevel(nLogLevel);
            g_pcLog->INFO("Cur Log Level %d", g_pcLog->GetLogLevel());
            m_sock.Clear();

            return TRM_OK;
        }
        else if(strAction.compare(TRM_ACTION_START) == 0)
        {
            m_sock.Clear();
            return TRM_RECV_PROC_START;
        }
        else if(strAction.compare(TRM_ACTION_STOP) == 0)
        {
            m_sock.Clear();
            return TRM_RECV_PROC_STOP;
        }
        else if(strAction.compare(TRM_ACTION_INIT) == 0)
        {
            m_sock.Clear();
            return TRM_RECV_INIT;
        }
        else
        {
            g_pcLog->INFO("Check Action Message ");
            m_sock.Clear();
        }

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
        return TRM_NOK;
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
        return TRM_NOK;
    } catch(...) {
        m_sock.Clear();
        g_pcLog->INFO("Unkown Error");
        return TRM_NOK;
    }

    return TRM_NOK;
}

int CNMSession::SetRequestMsg(ST_TRACE_REQUEST *a_tracerequest)
{
	ST_TRACE_REQUEST 	stTraceRequest;
	stTraceRequest 		= *a_tracerequest;

    rabbit::object  menu = m_root["BODY"];

    g_pcLog->INFO("oper_no [%ld]", stTraceRequest.oper_no);
    menu["oper_no"]  = stTraceRequest.oper_no;
    g_pcLog->INFO("pkg_name [%s]", stTraceRequest.pkg_name);
    menu["pkg_name"]    = stTraceRequest.pkg_name;
    g_pcLog->INFO("node_no [%d]", stTraceRequest.node_no);
    menu["node_no"]    = stTraceRequest.node_no;
    g_pcLog->INFO("trace [%s]", stTraceRequest.trace);
    menu["trace"]    = stTraceRequest.trace;
    g_pcLog->INFO("protocol [%d]", stTraceRequest.protocol);
    menu["protocol"]    = stTraceRequest.protocol;
    g_pcLog->INFO("search_mode [%d]", stTraceRequest.search_mode);
    menu["search_mode"]    = stTraceRequest.search_mode;
    g_pcLog->INFO("keyword [%s]", stTraceRequest.keyword);
    menu["keyword"]    = stTraceRequest.keyword;
    g_pcLog->INFO("run_mode [%d]", stTraceRequest.run_mode);
    menu["run_mode"]    = stTraceRequest.run_mode;
    g_pcLog->INFO("user_id [%s]", stTraceRequest.user_id);
    menu["user_id"]    = stTraceRequest.user_id;
    g_pcLog->INFO("start_date [%s]", stTraceRequest.start_date);
    menu["start_date"]    = stTraceRequest.start_date;
    g_pcLog->INFO("end_date [%s]", stTraceRequest.end_date);
    menu["end_date"]    = stTraceRequest.end_date;

    g_pcLog->INFO("proc_no [%d]", stTraceRequest.proc_no);
    menu["proc_no"]    = stTraceRequest.proc_no;
    g_pcLog->INFO("cmd [%s]", stTraceRequest.cmd);
    menu["cmd"]    = stTraceRequest.cmd;

#ifdef TRM_DEBUG
    cout << m_root.str() << endl;
#endif	
	m_sock.SetPayload(m_root.str());

	return TRM_OK;
}

// init response
int CNMSession::SendInitResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest)
{
	int 		nret = 0;

	ST_TRACE_REQUEST	st_batchrequest;
	st_batchrequest = *a_tracerequest;

	m_sock.SetCommand(ATOM_PROC_CTL);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->nm_proc_no);             // Src NM
    m_sock.SetFlagResponse();			// response

    g_pcLog->INFO("src : id_snode : %d, id_sproc : %d, dst : id_dnode : %d, ip_dproc : %d",
	          a_coworkinfo->node_no, a_coworkinfo->trm_proc_no, a_coworkinfo->node_no, a_coworkinfo->nm_proc_no);

	rabbit::object  	objRecvRoot;
    rabbit::object  menu = objRecvRoot["BODY"];

    menu["code"]    = 0;

	if(ret == TRM_NOK)
	{
	    menu["success"]    = false;
	}
	else
	{
	    menu["success"]    = true;
	}

	m_sock.SetPayload(objRecvRoot.str());

	g_pcLog->INFO("CNMSession Send Init Response Msg");
	nret = m_sock.SendMesg();
    if (nret < 0) {
        g_pcLog->DEBUG("CNMSession SendInitResponseMsg, Fail to Send Message : %s", m_sock.CSocket::m_strErrorMsg.c_str());
		m_sock.Clear();
        return TRM_NOK;
    }
	
	return TRM_OK;
}

int CNMSession::SendWSMResponseMg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest, int a_nOffData, int a_nRet)
{
	ST_TRACE_REQUEST st_TraceRequest;
	st_TraceRequest = *a_tracerequest;

	if (a_nOffData == 0)
		m_sock.SetCommand(TRM_TRACE_ON);
	else if(a_nOffData == 1)
		m_sock.SetCommand(TRM_TRACE_OFF);
	else
		m_sock.SetCommand(TRM_TRACE_DATA);

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);                // WSM
    m_sock.SetFlagResponse();
	m_sock.SetSequence(st_TraceRequest.sequence);

	rabbit::object  	objRecvRoot;

    rabbit::object  menu = objRecvRoot["BODY"];

    menu["oper_no"]    	= st_TraceRequest.oper_no;
    menu["trace"]    	= st_TraceRequest.trace;
    menu["status"]  	=  1;
    menu["start_date"] 	= st_TraceRequest.start_date;
    menu["end_date"] 	= st_TraceRequest.end_date;
	menu["msg"]	    	= "";

    if(a_nRet == TRM_NOK)
    {
        menu["success"]    = false;
    }
    else
    {
        menu["success"]    = true;
    }

#ifdef TRM_DEBUG
    cout << objRecvRoot.str() << endl;
#endif    
	m_sock.SetPayload(objRecvRoot.str());

	g_pcLog->INFO("CNMSession Send Response Msg to WSM");
    if (m_sock.SendMesg() < 0) {
        g_pcLog->ERROR("CNMSession, SendWSMResponseMg, Fail to Send Message : %s", m_sock.CSocket::m_strErrorMsg.c_str());
		m_sock.Clear();
        return TRM_NOK;
    }

    m_sock.Clear();

	return TRM_OK;
}

int CNMSession::SendWSMResponseMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_RESPONSE *a_traceresponse, int a_nOffData, int a_nRet)
{
	ST_TRACE_RESPONSE st_TraceResponse;
	st_TraceResponse = *a_traceresponse;

	g_pcLog->DEBUG("a_nOffData =[%d]", a_nOffData);

	if (a_nOffData == 0)
		m_sock.SetCommand(TRM_TRACE_ON);
	else if(a_nOffData == 1)
		m_sock.SetCommand(TRM_TRACE_OFF);
	else
		m_sock.SetCommand(TRM_TRACE_DATA);
#if 1
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);                // WSM
#else
    m_sock.SetSource(1, 10);				// Src Node_No, Src Process_No
    m_sock.SetDestination(1, 46);				// WSM Node_No, WSM Process_No
#endif
    m_sock.SetFlagResponse();			// response
    m_sock.SetSequence(m_stTraceRequest.sequence);

	rabbit::object  	objRecvRoot;

    rabbit::object  menu = objRecvRoot["BODY"];

    menu["oper_no"]    	= st_TraceResponse.oper_no;
    menu["trace"]    	= st_TraceResponse.trace;
    menu["status"]  	= st_TraceResponse.status;
    menu["start_date"] 	= st_TraceResponse.start_date;
    menu["end_date"] 	= st_TraceResponse.end_date;
	menu["msg"]	    	= st_TraceResponse.msg;

    if(a_nRet == TRM_NOK)
    {
        menu["success"]    = false;
    }
    else
    {
        menu["success"]    = true;
    }

#ifdef TRM_DEBUG
    cout << objRecvRoot.str() << endl;
#endif
    
	m_sock.SetPayload(objRecvRoot.str());

	g_pcLog->INFO("CNMSession Send Response Msg to WSM");
    if (m_sock.SendMesg() < 0) {
        g_pcLog->INFO("CNMSession Send message failed : %s", m_sock.CSocket::m_strErrorMsg.c_str());
		m_sock.Clear();
        return TRM_NOK;
    }

    m_sock.Clear();

	return TRM_OK;
}

int CNMSession::SendRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest)
{
	string strPayload;

	ST_TRACE_REQUEST 	stTraceRequest;
	stTraceRequest 		= *a_tracerequest;

    m_sock.SetCommand(TRM_TRACE_ON);
    m_sock.SetFlagRequest();
	m_sock.SetSequence(stTraceRequest.sequence);

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
    m_sock.SetDestination(stTraceRequest.node_no, stTraceRequest.proc_no);		// Dst Node_No, Dst Process_No

	if(&stTraceRequest.pkg_name != NULL && &stTraceRequest.oper_no != 0)
	{
		if( SetRequestMsg(&stTraceRequest) == TRM_OK)
		{
			strPayload = m_sock.GetPayload();
	
			g_pcLog->INFO("CNMSession SendMsg strPayload [%s]", strPayload.c_str());
		    if (m_sock.SendMesg() < 0) 
			{
	        	g_pcLog->INFO("Fail to Send Message : %s", m_sock.CSocket::m_strErrorMsg.c_str());
	            m_sock.Clear();
		        return TRM_NOK;
	    	}
			m_root.clear();
		}
	}

	m_sock.Clear();

	return TRM_OK;
}

#ifdef TRM_DEBUG
// for Web Test
int CNMSession::TestWebGetTraceData(double a_oper_no, char* start_date, char* end_date, ST_TRACE_RESPONSE *a_traceresponse)
{
    cout << "----- READ Trace Data" << endl;
	char 						imsi[255];
	std::string					strTraceResponse;

	CheckRecvMsg();

    g_pcLog->INFO("GetTraceData strTraceResponse : [%s]", strTraceResponse.c_str());

    try {
        rabbit::document    doc;

		a_traceresponse->oper_no = a_oper_no;
        cout << "[BODY][oper_no]    :" << "[" << a_traceresponse->oper_no << "]" << endl;

        string value3 = "on";
		strncpy(a_traceresponse->trace, value3.c_str(), sizeof(value3));
        cout << "[BODY][trace]    :" << "[" << a_traceresponse->trace << "]" << endl;

		string status = "process";
		strncpy(a_traceresponse->status, status.c_str(), sizeof(status));
        cout << "[BODY][status]    :" << "[" << a_traceresponse->status << "]" << endl;

		strncpy(imsi, start_date, sizeof(start_date) + 6);
		strncpy(a_traceresponse->start_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][start_date]    :" << "[" << a_traceresponse->start_date << "]" << endl;

		strncpy(imsi, end_date, sizeof(end_date) + 6);
		strncpy(a_traceresponse->end_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][end_date]    :" << "[" << a_traceresponse->end_date << "]" << endl;

        bool success_yn = true;
		a_traceresponse->success = success_yn;
        cout << "[BODY][success]    :" << "[" << a_traceresponse->success << "]" << endl;

		string msg = "Kimbo Slice wasn't a great professional fighter. He appeared in only one official UFC fight and was knocked out in the second round. But technical skill isn't why you know his name or why we're remembering him following his death on Monday. Fighting record aside, Slice was a cult hero, and no one appreciated his uncompromising style and unconventional path to the top more than the hip-hop community.";
  	  	strncpy(a_traceresponse->msg, msg.c_str(), msg.length());
        cout << "[BODY][msg]    :" << "[" << a_traceresponse->msg << "]" << endl;

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("Unkown Error");
        m_sock.Clear();
    }

	return TRM_OK;
}
#endif

int CNMSession::GetTraceData(ST_TRACE_RESPONSE *a_traceresponse)
{
	char		imsi[255];
	string		strTraceResponse;
	double 		oper_no = 0;
	string 		trace;
	string 		status;
	string 		start_date;
	string 		end_date;
	bool 		success_yn = false;
	string 		msg;

#ifdef TRM_DEBUG	
    cout << "----- READ Trace Data" << endl;
	CheckRecvMsg();
#endif

	a_traceresponse->sequence = m_sock.GetSequence();

	strTraceResponse = m_sock.GetPayload();

    g_pcLog->INFO("GetTraceData strTraceResponse : [%s]", strTraceResponse.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceResponse.c_str());

        oper_no = doc["BODY"]["oper_no"].as_int64();
		a_traceresponse->oper_no = oper_no;

        trace = string(doc["BODY"]["trace"].as_string());
		strncpy(a_traceresponse->trace, trace.c_str(), sizeof(trace));

		status = string(doc["BODY"]["status"].as_string());
		strncpy(a_traceresponse->status, status.c_str(), sizeof(status));

        start_date = string(doc["BODY"]["start_date"].as_string());
		strncpy(imsi, start_date.c_str(), sizeof(start_date) + 6);
		strncpy(a_traceresponse->start_date, space_remove(imsi), sizeof(imsi));

        end_date = string(doc["BODY"]["end_date"].as_string());
		strncpy(imsi, end_date.c_str(), sizeof(end_date) + 6);
		strncpy(a_traceresponse->end_date, space_remove(imsi), sizeof(imsi));

        success_yn = doc["BODY"]["success"].as_bool();
		a_traceresponse->success = success_yn;

        msg = string(doc["BODY"]["msg"].as_string());
		strncpy(a_traceresponse->msg, msg.c_str(), msg.length());

#ifdef TRM_DEBUG	
        cout << "[BODY][oper_no]    :" << "[" << a_traceresponse->oper_no << "]" << endl;
        cout << "[BODY][trace]    :" << "[" << a_traceresponse->trace << "]" << endl;
        cout << "[BODY][status]    :" << "[" << a_traceresponse->status << "]" << endl;
        cout << "[BODY][start_date]    :" << "[" << a_traceresponse->start_date << "]" << endl;
        cout << "[BODY][end_date]    :" << "[" << a_traceresponse->end_date << "]" << endl;
        cout << "[BODY][success]    :" << "[" << a_traceresponse->success << "]" << endl;
        cout << "[BODY][msg]    :" << "[" << a_traceresponse->msg << "]" << endl;
#endif

        m_sock.Clear();

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("Unkown Error");
        m_sock.Clear();
    }

	return TRM_OK;
}

int CNMSession::GetResponseMsg(ST_TRACE_RESPONSE *a_traceresponse)
{
    cout << "----- READ Response" << endl;
	char 						imsi[255];
	std::string					strTraceResponse;

#ifdef TRM_DEBUG
	CheckRecvMsg();
#endif

	a_traceresponse->sequence = m_sock.GetSequence();
	strTraceResponse = m_sock.GetPayload();

    g_pcLog->INFO("strTraceResponse : [%s]", strTraceResponse.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceResponse.c_str());

        double value = doc["BODY"]["oper_no"].as_int64();
		a_traceresponse->oper_no = value;

        string value3 = string(doc["BODY"]["trace"].as_string());
		strncpy(a_traceresponse->trace, value3.c_str(), sizeof(value3));

		string status = string(doc["BODY"]["status"].as_string());
		strncpy(a_traceresponse->status, status.c_str(), sizeof(status));

        string value9 = string(doc["BODY"]["start_date"].as_string());
		strncpy(imsi, value9.c_str(), sizeof(value9) + 6);
		strncpy(a_traceresponse->start_date, space_remove(imsi), sizeof(imsi));

        string value10 = string(doc["BODY"]["end_date"].as_string());
        strncpy(imsi, value10.c_str(), sizeof(value10) + 6);
        strncpy(a_traceresponse->end_date, space_remove(imsi), sizeof(imsi));

        bool success_yn = doc["BODY"]["success"].as_bool();
		a_traceresponse->success = success_yn;

        string msg = string(doc["BODY"]["msg"].as_string());
		strncpy(a_traceresponse->msg, msg.c_str(), msg.length());

#ifdef TRM_DEBUG
        cout << "[BODY][oper_no]    :" << "[" << a_traceresponse->oper_no << "]" << endl;
        cout << "[BODY][trace]    :" << "[" << a_traceresponse->trace << "]" << endl;
        cout << "[BODY][status]    :" << "[" << a_traceresponse->status << "]" << endl;
        cout << "[BODY][start_date]    :" << "[" << a_traceresponse->start_date << "]" << endl;
        cout << "[BODY][end_date]    :" << "[" << a_traceresponse->end_date << "]" << endl;
        cout << "[BODY][success]    :" << "[" << a_traceresponse->success << "]" << endl;
        cout << "[BODY][msg]    :" << "[" << a_traceresponse->msg << "]" << endl;
#endif

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("Unkown Error");
        m_sock.Clear();
    }

	return TRM_OK;
}

int CNMSession::GetRequestMsg(int nOnOff, ST_TRACE_REQUEST *a_tracerequest)
{
    cout << "----- READ Request" << endl;
	char 				imsi[255];
	std::string			strTraceRequest;
	long long 			nOper_no = 0;
	string 				strPkg_name;
	int 				nNode_no = 0;
	string				strTrace;
	int	 				nProtocol = 0;
	int 				nSearchMode = 0;
	string 				strKeyword;
	int					nRunMode = 0;
	string 				nUserId;
	string 				strStartDate;
	string 				strEndDate;
	string				strUserId;

#ifdef TRM_DEBUG
	CheckRecvMsg();
#endif

	a_tracerequest->sequence = m_sock.GetSequence();
	strTraceRequest = m_sock.GetPayload();

    g_pcLog->INFO("strTraceRequest: [%s]", strTraceRequest.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceRequest.c_str());

        nOper_no = doc["BODY"]["oper_no"].as_int64();
		a_tracerequest->oper_no = nOper_no;
        cout << "[BODY][oper_no]    :" << "[" << a_tracerequest->oper_no << "]" << endl;

        strPkg_name = string(doc["BODY"]["pkg_name"].as_string());
		strncpy(a_tracerequest->pkg_name, strPkg_name.c_str(), strPkg_name.length());
        cout << "[BODY][pkg_name]    :" << "[" << a_tracerequest->pkg_name << "]" << endl;

        nNode_no = doc["BODY"]["node_no"].as_int();
		a_tracerequest->node_no = nNode_no;
        cout << "[BODY][node_no]    :" << "[" << a_tracerequest->node_no << "]" << endl;

        strTrace = string(doc["BODY"]["trace"].as_string());
		strncpy(a_tracerequest->trace, strTrace.c_str(), strTrace.length());
        cout << "[BODY][trace]    :" << "[" << a_tracerequest->trace << "]" << endl;

        nProtocol = doc["BODY"]["protocol"].as_int();
		a_tracerequest->protocol = nProtocol;
        cout << "[BODY][protocol]    :" << "[" << a_tracerequest->protocol << "]" << endl;

        nSearchMode = doc["BODY"]["search_mode"].as_int();
		a_tracerequest->search_mode = nSearchMode;
        cout << "[BODY][search_mode    :" << "[" << a_tracerequest->search_mode << "]" << endl;

        strKeyword = string(doc["BODY"]["keyword"].as_string());
		strncpy(a_tracerequest->keyword, strKeyword.c_str(), sizeof(strKeyword) + 12);
        cout << "[BODY][keyword]    :" << "[" << a_tracerequest->keyword << "]" << endl;

        nRunMode = doc["BODY"]["run_mode"].as_int();
		a_tracerequest->run_mode = nRunMode;
        cout << "[BODY][run_mode]    :" << "[" << a_tracerequest->run_mode << "]" << endl;

        strUserId = string(doc["BODY"]["user_id"].as_string());
		strncpy(a_tracerequest->user_id, strUserId.c_str(), strUserId.length());
        cout << "[BODY][user_id]    :" << "[" << a_tracerequest->user_id << "]" << endl;

        strStartDate = string(doc["BODY"]["start_date"].as_string());
		strncpy(imsi, strStartDate.c_str(), sizeof(strStartDate) + 6);
		strncpy(a_tracerequest->start_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][start_date]    :" << "[" << a_tracerequest->start_date << "]" << endl;

        if(nOnOff == 1)
        {
            strEndDate = string(doc["BODY"]["end_date"].as_string());
	    	strncpy(imsi, strEndDate.c_str(), sizeof(strEndDate) + 6);
		    strncpy(a_tracerequest->end_date, space_remove(imsi), sizeof(imsi));
            cout << "[BODY][end_date]    :" << "[" << a_tracerequest->end_date << "]" << endl;
        }

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("Unkown Error");
        m_sock.Clear();
    }
	
	return TRM_OK;
}

int CNMSession::GetRegistRouteInfo(ST_TRACE_ROUTE *a_traceroute)
{
    cout << "----- READ RegistRoutInfo" << endl;
	
	std::string					strTraceRoute;

#ifdef TRM_DEBUG	
	CheckRecvMsg();
#endif

	a_traceroute->sequence = m_sock.GetSequence();
	strTraceRoute = m_sock.GetPayload();

    g_pcLog->INFO("strTraceRoute : [%s]", strTraceRoute.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceRoute.c_str());

        string pkg_name = string(doc["BODY"]["pkg_name"].as_string());
		strncpy(a_traceroute->pkg_name, pkg_name.c_str(), pkg_name.length());
        cout << "[BODY][pkg_name]    :" << "[" << a_traceroute->pkg_name << "]" << endl;

		string cmd = string(doc["BODY"]["cmd"].as_string());
		strncpy(a_traceroute->cmd, cmd.c_str(), cmd.length());
        cout << "[BODY][cmd]    :" << "[" << a_traceroute->cmd << "]" << endl;

        int node_no = doc["BODY"]["node_no"].as_int();
		a_traceroute->node_no = node_no;
        cout << "[BODY][node_no]    :" << "[" << a_traceroute->node_no << "]" << endl;

        string node_name = string(doc["BODY"]["node_name"].as_string());
		strncpy(a_traceroute->node_name, node_name.c_str(), node_name.length());
        cout << "[BODY][node_name]    :" << "[" << a_traceroute->node_name << "]" << endl;

        int proc_no = doc["BODY"]["proc_no"].as_int();
		a_traceroute->proc_no = proc_no;
        cout << "[BODY][proc_no]    :" << "[" << a_traceroute->proc_no << "]" << endl;

        string node_type = string(doc["BODY"]["node_type"].as_string());
		strncpy(a_traceroute->node_type, node_type.c_str(), node_type.length());
        cout << "[BODY][node_type]    :" << "[" << a_traceroute->node_type << "]" << endl;

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("Unkown Error");
        m_sock.Clear();
    }

	return TRM_OK;
}

int CNMSession::IsRequest()
{
    char flag = m_sock.GetFlag();

    if (flag & CProtocol::FLAG_REQUEST)
		return 0;
    if (flag & CProtocol::FLAG_RESPONSE)
		return 1;
    if (flag & CProtocol::FLAG_NOTIFY)
		return 2;
    if (flag & CProtocol::FLAG_RETRNS)
		return 3;
    if (flag & CProtocol::FLAG_BROAD)
		return 4;
    if (flag & CProtocol::FLAG_ERROR)
		return 5;

	return -1;
}

#ifdef TRM_DEBUG
int CNMSession::CheckRecvMsg()
{
	g_pcLog->INFO("CNMSession CheckRecvMsg");

    int is_node = 0, is_proc = 0;
    int id_node = 0, id_proc = 0;
	int irecvlength = 0;
	int isequence = 0;
    std::string strBuf;

	// header
    g_pcLog->INFO("Version: %d", m_sock.GetVersion());
    m_sock.GetCommand(strBuf);
    g_pcLog->INFO("Command: %s", strBuf.c_str());

    g_pcLog->INFO("Response, %x", m_sock.GetFlag());
	m_sock.GetSource(is_node, is_proc);
	g_pcLog->INFO("Source Node : %d", is_node);
	g_pcLog->INFO("Source Proc : %d", is_proc);
	m_sock.GetDestination(id_node, id_proc);
	g_pcLog->INFO("Dest   Node : %d", id_node);
	g_pcLog->INFO("Dest   Proc : %d", id_proc);
	isequence = m_sock.GetSequence();
	g_pcLog->INFO("Sequence    : %d", isequence);
	irecvlength = m_sock.GetLength();
	g_pcLog->INFO("Length      : %d", irecvlength);

	return TRM_OK;
}
#endif

int CNMSession::Final()
{
    if(m_pDB != NULL)
    {
        delete m_pDB;   
        m_pDB = NULL;   
    }

	m_sock.Close();

	delete g_pcLog;
	g_pcLog = NULL;

	return TRM_OK;
}

char* CNMSession::space_remove(char *a_str)
{
  // change BUF_SIZE depending on the length of your string
    char buf[SPACE_REMOVE_BUF_SIZE] = { 0 };
    char *p;

    p = strtok(a_str, "\r\n\t ");

    strcat(buf, p);

    while(p!=NULL)
	{
        p = strtok(NULL, "\r\n\t ");

        if(p!=NULL) 
		{
            strcat(buf, " ");
            strcat(buf, p);
        }
    }

    memset(a_str, '\0', SPACE_REMOVE_BUF_SIZE);
    strcpy(a_str, buf);
    return a_str;
}

