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

    if(m_pDB != NULL)
    {
        delete m_pDB;   
        m_pDB = NULL;   
    }

	m_sock.Close();
	delete g_pcLog;
	g_pcLog = NULL;
}

int CNMSession::Initial()
{
	// Connect to MN
    if(m_sock.Connect(g_pcCFG.NM.m_strNMAddr.c_str(), g_pcCFG.NM.m_nNMPort) == false)
    {	
        printf("Connection IP : %s, Port : %d Failed\n", g_pcCFG.NM.m_strNMAddr.c_str(), g_pcCFG.NM.m_nNMPort );
        return TRM_NOK;
    }

	return TRM_OK;
}
int CNMSession::Regist(ST_COWORK_INFO *a_coworkinfo)
{
	int nret;

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
//    m_sock.SetSource(1, 11);				// Src TRM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->nm_proc_no);             // Src NM
//    m_sock.SetDestination(1, 8);				// Src NM
    m_sock.SetFlagRequest();			// response
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
        g_pcLog->INFO("message send failed : %s", m_sock.CSocket::m_strErrorMsg.c_str());
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

// which Message is ?
int CNMSession::RecvMsg()
{
    int  nRecvFlag = 0;
	int  nRetRecv = 0;
//	char imsi[256];
//	int  nLength = 0;
//	string strimsi;
//	string strpayload;
    string strBuf;

    if( m_sock.IsConnected() == false)
    {
        Initial();
    }

	nRecvFlag = m_sock.RecvMesg(NULL, -1);
	if(nRecvFlag < 0)
	{
        g_pcLog->INFO("message receive failed errorno=%d, socket [%s]", 
									errno, m_sock.CSocket::m_strErrorMsg.c_str());
        printf("socket, %s\n", m_sock.CSocket::m_strErrorMsg.c_str());

		m_sock.Clear();

		return TRM_RECV_ERROR;
	}

	// header
//    g_pcLog->INFO("Version: %d", m_sock.GetVersion());
    m_sock.GetCommand(strBuf);
//    g_pcLog->INFO("Command: %s", strBuf.c_str());

	// Debug 
//	strpayload = m_sock.GetPayload();
//    g_pcLog->INFO("payload: [%s]", strpayload.c_str());
//	strimsi = m_sock.GetFlag();
//    g_pcLog->INFO("flag: %s", strimsi.c_str());
//	nLength = m_sock.GetLength();
//    g_pcLog->INFO("Length: %d", nLength);
//	strcpy(imsi, strBuf.c_str());
	
    // Receive
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
// 		g_pcLog->INFO("IsFlagRequest Other");
		m_sock.Clear();
		return TRM_NOK;
	}

	return TRM_OK;
}

// Process Control의 구분
int CNMSession::ProcessCtrl()
{
    cout << "----- READ 2 " << endl;
    std::string                 strLogPayload;

    strLogPayload = m_sock.GetPayload();

    g_pcLog->INFO("request: [%s]", strLogPayload.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strLogPayload.c_str());

        string strAction = string(doc["BODY"]["action"].as_string());
        cout << "[BODY][action]    :" << "[" << strAction << "]" << endl;

        int nLogLevel = doc["BODY"]["loglevel"].as_int();
        cout << "[BODY][LogLevel]    :" << "[" << nLogLevel << "]" << endl;

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

// 추가 필요 데이터
    g_pcLog->INFO("proc_no [%d]", stTraceRequest.proc_no);
    menu["proc_no"]    = stTraceRequest.proc_no;
    g_pcLog->INFO("cmd [%s]", stTraceRequest.cmd);
    menu["cmd"]    = stTraceRequest.cmd;

    cout << m_root.str() << endl;
	m_sock.SetPayload(m_root.str());

	return TRM_OK;
}

// init response
int CNMSession::SendInitResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest)
{
	int nret = 0;

	ST_TRACE_REQUEST	st_batchrequest;
	st_batchrequest = *a_tracerequest;

//    g_pcLog->INFO("src : id_snode : %d, id_sproc : %d, dst : id_dnode : %d, ip_dproc : %d",
//	          st_batchrequest.id_snode, st_batchrequest.id_sproc, st_batchrequest.id_dnode, st_batchrequest.id_dproc);
	m_sock.SetCommand(ATOM_PROC_CTL);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
//    m_sock.SetSource(1, 10);				// Src TRM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->nm_proc_no);             // Src NM
//    m_sock.SetDestination(36, 23);				// Src NM
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

//    cout << objRecvRoot.str() << endl;
	m_sock.SetPayload(objRecvRoot.str());

	nret = m_sock.SendMesg();

	g_pcLog->INFO("CNMSession Send Init Response Msg");
    if (nret < 0) {
        g_pcLog->INFO("message send failed : %s", m_sock.CSocket::m_strErrorMsg.c_str());
		m_sock.Clear();
        return TRM_NOK;
    }
	
	return TRM_OK;
}

int CNMSession::SendWSMResponseMg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest, int ret)
{
	ST_TRACE_REQUEST st_TraceRequest;
	st_TraceRequest = *a_tracerequest;

	m_sock.SetCommand(TRM_TRACE_ON);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
//    m_sock.SetSource(1, 11);				// Src Node_No, Src Process_No
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);                // WSM
//    m_sock.SetDestination(1, 46);				// WSM Node_No, WSM Process_No
    m_sock.SetFlagResponse();			// response
//    m_sock.GetSequence();
	m_sock.SetSequence(st_TraceRequest.sequence);

	rabbit::object  	objRecvRoot;

    rabbit::object  menu = objRecvRoot["BODY"];

    menu["oper_no"]    	= st_TraceRequest.oper_no;
    menu["trace"]    	= st_TraceRequest.trace;
    menu["status"]  	=  1;
    menu["start_date"] 	= st_TraceRequest.start_date;
    menu["end_date"] 	= st_TraceRequest.end_date;
	menu["msg"]	    	= "";

    if(ret == TRM_NOK)
    {
        menu["success"]    = false;
    }
    else
    {
        menu["success"]    = true;
    }

    cout << objRecvRoot.str() << endl;
    
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

int CNMSession::SendWSMResponseMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_RESPONSE *a_traceresponse, int ret)
{
	ST_TRACE_RESPONSE st_TraceResponse;
	st_TraceResponse = *a_traceresponse;

	m_sock.SetCommand(TRM_TRACE_ON);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
//    m_sock.SetSource(1, 10);				// Src Node_No, Src Process_No
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);                // WSM
//    m_sock.SetDestination(1, 46);				// WSM Node_No, WSM Process_No
    m_sock.SetFlagResponse();			// response
    m_sock.SetSequence(st_TraceResponse.sequence);

	rabbit::object  	objRecvRoot;

    rabbit::object  menu = objRecvRoot["BODY"];

    menu["oper_no"]    	= st_TraceResponse.oper_no;
    menu["trace"]    	= st_TraceResponse.trace;
    menu["status"]  	= st_TraceResponse.status;
    menu["start_date"] 	= st_TraceResponse.start_date;
    menu["end_date"] 	= st_TraceResponse.end_date;
	menu["msg"]	    	= st_TraceResponse.msg;

    if(ret == TRM_NOK)
    {
        menu["success"]    = false;
    }
    else
    {
        menu["success"]    = true;
    }

    cout << objRecvRoot.str() << endl;
    
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

    m_sock.SetCommand(TRM_TRACE_ON);	// TRM_TRACE_ON
    m_sock.SetFlagRequest();			// request
	m_sock.SetSequence(stTraceRequest.sequence);

//	nnode_id = atoi(getenv("NODEID"));
//	nproc_id = atoi(getenv("PROCID"));

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->trm_proc_no);             // Src TRM
    m_sock.SetDestination(stTraceRequest.node_no, stTraceRequest.proc_no);		// Dst Node_No, Dst Process_No
//    m_sock.SetDestination(36, 23);		// Dst Node_No, Dst Process_No
//    nSendSeq = m_sock.SetSequence();

	if(&stTraceRequest.pkg_name != NULL && &stTraceRequest.oper_no != 0)
	{
		if( SetRequestMsg(&stTraceRequest) == TRM_OK)
		{
			strPayload = m_sock.GetPayload();
	
			g_pcLog->INFO("CNMSession SendMsg strPayload [%s]", strPayload.c_str());
		    if (m_sock.SendMesg() < 0) 
			{
	        	g_pcLog->INFO("message send failed : %s", m_sock.CSocket::m_strErrorMsg.c_str());
	            m_sock.Clear();
		        return TRM_NOK;
	    	}
			m_root.clear();
		}
	}

	m_sock.Clear();

	return TRM_OK;
}

// for Web Test
int CNMSession::TestWebGetTraceData(double a_oper_no, char* start_date, char* end_date, ST_TRACE_RESPONSE *a_traceresponse)
{
    cout << "----- READ Trace Data" << endl;
	char 						imsi[255];
	std::string					strTraceResponse;

//	CheckRecvMsg();

//	a_traceresponse->sequence = m_sock.GetSequence();

//	strTraceResponse = m_sock.GetPayload();

    g_pcLog->INFO("GetTraceData strTraceResponse : [%s]", strTraceResponse.c_str());

//    try {
        rabbit::document    doc;
//        doc.parse(strTraceResponse.c_str());

//        double value = doc["BODY"]["oper_no"].as_int64();
		a_traceresponse->oper_no = a_oper_no;
        cout << "[BODY][oper_no]    :" << "[" << a_traceresponse->oper_no << "]" << endl;

        string value3 = "on"; //string(doc["BODY"]["trace"].as_string());
		strncpy(a_traceresponse->trace, value3.c_str(), sizeof(value3));
        cout << "[BODY][trace]    :" << "[" << a_traceresponse->trace << "]" << endl;

		string status = "process"; //string(doc["BODY"]["status"].as_string());
		strncpy(a_traceresponse->status, status.c_str(), sizeof(status));
        cout << "[BODY][status]    :" << "[" << a_traceresponse->status << "]" << endl;

//        string value9 = string(doc["BODY"]["start_date"].as_string());
//		strncpy(imsi, value9.c_str(), sizeof(value9) + 6);
		strncpy(imsi, start_date, sizeof(start_date) + 6);
		strncpy(a_traceresponse->start_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][start_date]    :" << "[" << a_traceresponse->start_date << "]" << endl;

//        string end_date = string(doc["BODY"]["end_date"].as_string());
//		strncpy(imsi, end_date.c_str(), sizeof(end_date) + 6);
		strncpy(imsi, end_date, sizeof(end_date) + 6);
		strncpy(a_traceresponse->end_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][end_date]    :" << "[" << a_traceresponse->end_date << "]" << endl;

        bool success_yn = true; //doc["BODY"]["success"].as_bool();
		a_traceresponse->success = success_yn;
        cout << "[BODY][success]    :" << "[" << a_traceresponse->success << "]" << endl;

        string msg = "인공지능 컴퓨터의 세계 양대 산맥은 구글의 알파고와 IBM의 왓슨입니다. 미국 퀴즈쇼에서 우승하면서 유명세를 탄 왓슨은 특>히 언어를 인식하는데 뛰어납니다.IBM이 이번에는 왓슨 기술을 바탕으로 사람의 성격을 분석할 수 있는 새 로봇나오미를 내놨습니다. 박소정 기자입니다.[기자]2015년 12월생인 인공지능 로봇 나오미.질문에 대답도 하고 춤도 춥니다.이뿐 아니라 이마에 달린 카메라로 사물이나 사>람을 인식해 성격도 분석합니다.IBM이 인공지능 컴퓨터 왓슨의 기술을 탑재해 내놓은 새로운 로봇입니다.왓슨은 과거 체스 대회에서 인간을 꺾은 딥블루의 기반이자 퀴즈쇼에서 우승해 알파고보다 먼저 유명해진 인공지능입니다.IBM은 인공지능이라는 말 대신 사람과 상호작용을 할 수 있다는 뜻으로 코그너티브 컴퓨팅, 즉 인지 기술이라고 설명합니다.[김연주 / 한국IBM 왓슨 총괄 상무 : 법률 전문가로서 서비스해줄 수>도 있고, 금융 분야로 가면 금융 전문가가 되고, 인간의 전문성을 도와서 증폭시키고 서비스를 개선하는 데 활용되는 모델입니다.] 사람처럼 인지하고 추론하는 능력을 갖춘 왓슨 기술은 로봇으로 구현되는 기술뿐 아니라 금융이나 의료, 패션 등 사회의 다양한 산업으로 펼쳐지고 >있습니다.최근 국립환경과학원은 미세먼지 예보에 왓슨을 도입하려고 논의 중입니다.아이들과 놀아주는 장난감뿐 아니라 자산 관리 상담 등>에도 널리 쓰이고 있습니다.[안태규 / 한국IBM 차장 : 고객의 성향을 파악하는 것과 동시에 고객의 자산 현황을 파악해 그 사람에게 상품을 추천해주는 것을 인공지능 기술을 사용해서…] IBM은 왓슨이 올해 말에는 자연스럽게 한국어로 대화하는 능력도 갖추게 될 것이라고 밝혔습니다. 또, 알파고를 제치고 전 세계 모든 산업의 기반 기술로 활용될 것이라는 자신감을내비쳤습니.dsfasdfdsfdsaffsafdfaffasfdas~~~~~~~~~~~~~~~~~~~~";	 //string(doc["BODY"]["msg"].as_string());
		strncpy(a_traceresponse->msg, msg.c_str(), msg.length());
        cout << "[BODY][msg]    :" << "[" << a_traceresponse->msg << "]" << endl;

/*    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("Unkown Error");
        m_sock.Clear();
    }
*/
	return TRM_OK;
}


int CNMSession::GetTraceData(ST_TRACE_RESPONSE *a_traceresponse)
{
    cout << "----- READ Trace Data" << endl;
	char 						imsi[255];
	std::string					strTraceResponse;

	CheckRecvMsg();

	a_traceresponse->sequence = m_sock.GetSequence();

	strTraceResponse = m_sock.GetPayload();

    g_pcLog->INFO("GetTraceData strTraceResponse : [%s]", strTraceResponse.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceResponse.c_str());

        double value = doc["BODY"]["oper_no"].as_int64();
		a_traceresponse->oper_no = value;
        cout << "[BODY][oper_no]    :" << "[" << a_traceresponse->oper_no << "]" << endl;

        string value3 = string(doc["BODY"]["trace"].as_string());
		strncpy(a_traceresponse->trace, value3.c_str(), sizeof(value3));
        cout << "[BODY][trace]    :" << "[" << a_traceresponse->trace << "]" << endl;

		string status = string(doc["BODY"]["status"].as_string());
		strncpy(a_traceresponse->status, status.c_str(), sizeof(status));
        cout << "[BODY][status]    :" << "[" << a_traceresponse->status << "]" << endl;

        string value9 = string(doc["BODY"]["start_date"].as_string());
		strncpy(imsi, value9.c_str(), sizeof(value9) + 6);
		strncpy(a_traceresponse->start_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][start_date]    :" << "[" << a_traceresponse->start_date << "]" << endl;

        string end_date = string(doc["BODY"]["end_date"].as_string());
		strncpy(imsi, end_date.c_str(), sizeof(end_date) + 6);
		strncpy(a_traceresponse->end_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][end_date]    :" << "[" << a_traceresponse->end_date << "]" << endl;

        bool success_yn = doc["BODY"]["success"].as_bool();
		a_traceresponse->success = success_yn;
        cout << "[BODY][success]    :" << "[" << a_traceresponse->success << "]" << endl;

        string msg = string(doc["BODY"]["msg"].as_string());
		strncpy(a_traceresponse->msg, msg.c_str(), msg.length());
        cout << "[BODY][msg]    :" << "[" << a_traceresponse->msg << "]" << endl;

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

	CheckRecvMsg();

	a_traceresponse->sequence = m_sock.GetSequence();
	strTraceResponse = m_sock.GetPayload();

    g_pcLog->INFO("strTraceResponse : [%s]", strTraceResponse.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceResponse.c_str());

        double value = doc["BODY"]["oper_no"].as_int64();
		a_traceresponse->oper_no = value;
        cout << "[BODY][oper_no]    :" << "[" << a_traceresponse->oper_no << "]" << endl;

        string value3 = string(doc["BODY"]["trace"].as_string());
		strncpy(a_traceresponse->trace, value3.c_str(), sizeof(value3));
        cout << "[BODY][trace]    :" << "[" << a_traceresponse->trace << "]" << endl;

		string status = string(doc["BODY"]["status"].as_string());
		strncpy(a_traceresponse->status, status.c_str(), sizeof(status));
        cout << "[BODY][status]    :" << "[" << a_traceresponse->status << "]" << endl;

        string value9 = string(doc["BODY"]["start_date"].as_string());
		strncpy(imsi, value9.c_str(), sizeof(value9) + 6);
		strncpy(a_traceresponse->start_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][start_date]    :" << "[" << a_traceresponse->start_date << "]" << endl;

        string value10 = string(doc["BODY"]["end_date"].as_string());
        strncpy(imsi, value10.c_str(), sizeof(value10) + 6);
        strncpy(a_traceresponse->end_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][end_date]    :" << "[" << a_traceresponse->end_date << "]" << endl;

        bool success_yn = doc["BODY"]["success"].as_bool();
		a_traceresponse->success = success_yn;
        cout << "[BODY][success]    :" << "[" << a_traceresponse->success << "]" << endl;

        string msg = string(doc["BODY"]["msg"].as_string());
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

int CNMSession::GetRequestMsg(int nOnOff, ST_TRACE_REQUEST *a_tracerequest)
{
    cout << "----- READ Request" << endl;
	char 						imsi[255];
	std::string					strTraceRequest;

	CheckRecvMsg();
	a_tracerequest->sequence = m_sock.GetSequence();
	strTraceRequest = m_sock.GetPayload();

    g_pcLog->INFO("strTraceRequest: [%s]", strTraceRequest.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strTraceRequest.c_str());

        long long value = doc["BODY"]["oper_no"].as_int64();
		a_tracerequest->oper_no = value;
        cout << "[BODY][oper_no]    :" << "[" << a_tracerequest->oper_no << "]" << endl;

        string value1 = string(doc["BODY"]["pkg_name"].as_string());
		strncpy(a_tracerequest->pkg_name, value1.c_str(), sizeof(value1));
        cout << "[BODY][pkg_name]    :" << "[" << a_tracerequest->pkg_name << "]" << endl;

        int value2 = doc["BODY"]["node_no"].as_int();
		a_tracerequest->node_no = value2;
        cout << "[BODY][node_no]    :" << "[" << a_tracerequest->node_no << "]" << endl;

        string value3 = string(doc["BODY"]["trace"].as_string());
		strncpy(a_tracerequest->trace, value3.c_str(), sizeof(value3));
        cout << "[BODY][trace]    :" << "[" << a_tracerequest->trace << "]" << endl;

        int value4 = doc["BODY"]["protocol"].as_int();
		a_tracerequest->protocol = value4;
        cout << "[BODY][protocol]    :" << "[" << a_tracerequest->protocol << "]" << endl;

        int value5 = doc["BODY"]["search_mode"].as_int();
		a_tracerequest->search_mode = value5;
        cout << "[BODY][search_mode    :" << "[" << a_tracerequest->search_mode << "]" << endl;

        string value6 = string(doc["BODY"]["keyword"].as_string());
		strncpy(a_tracerequest->keyword, value6.c_str(), sizeof(value6) + 12);
        cout << "[BODY][keyword]    :" << "[" << a_tracerequest->keyword << "]" << endl;

        int value7 = doc["BODY"]["run_mode"].as_int();
		a_tracerequest->run_mode = value7;
        cout << "[BODY][run_mode]    :" << "[" << a_tracerequest->run_mode << "]" << endl;

        string value8 = string(doc["BODY"]["user_id"].as_string());
		strncpy(a_tracerequest->user_id, value8.c_str(), sizeof(value8));
        cout << "[BODY][user_id]    :" << "[" << a_tracerequest->user_id << "]" << endl;

        string value9 = string(doc["BODY"]["start_date"].as_string());
		strncpy(imsi, value9.c_str(), sizeof(value9) + 6);
		strncpy(a_tracerequest->start_date, space_remove(imsi), sizeof(imsi));
        cout << "[BODY][start_date]    :" << "[" << a_tracerequest->start_date << "]" << endl;

        if(nOnOff == 1)
        {
            string end_date = string(doc["BODY"]["end_date"].as_string());
	    	strncpy(imsi, end_date.c_str(), sizeof(end_date) + 6);
		    strncpy(a_tracerequest->end_date, space_remove(imsi), sizeof(imsi));
            cout << "[BODY][end_date]    :" << "[" << a_tracerequest->end_date << "]" << endl;
        }
/*        cout << "---- PROC_NO" << endl;
        int value11 = doc["BODY"]["proc_no"].as_int();
		a_tracerequest->proc_no = value11;
        cout << "[BODY][proc_no]    :" << "[" << a_tracerequest->proc_no << "]" << endl;

        cout << "---- CMD" << endl;
        string value12 = string(doc["BODY"]["cmd"].as_string());
		strncpy(a_tracerequest->cmd, value12.c_str(), sizeof(value12));
        cout << "[BODY][cmd]    :" << "[" << a_tracerequest->cmd << "]" << endl;
*/
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

	CheckRecvMsg();

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

	// Request 메시지이면 
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

int CNMSession::Final()
{
	m_sock.Close();

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

