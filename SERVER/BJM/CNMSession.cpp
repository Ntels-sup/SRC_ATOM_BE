#include "CMain.hpp"
#include "BJM_Define.hpp"
#include "CScheduler.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <string>
#include <cerrno>
#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>

using namespace std;
using namespace rapidjson;
using std::cout;
using std::endl;
using std::string;
using std::string;
using std::vector;

//extern symbol
extern CFileLog* g_pcLog;
extern CConfigBJM g_pcCFG;

CNMSession::CNMSession()
{
	m_pDB 		 	= NULL;
	m_nStartFlag 	= BJM_NOK;
	m_nResponseSeq 	= 0;

	if(Initial() == BJM_OK)
	{
		m_nStartFlag = BJM_OK;
	}
	else
	{
		m_nStartFlag = BJM_NOK;
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

	delete m_stbatchrequest;
}

int CNMSession::Initial()
{
	// MN과의 통신 연결
    if(m_sock.Connect(g_pcCFG.NM.m_strNMAddr.c_str(), g_pcCFG.NM.m_nNMPort) == false)
    {	
        printf("Connection IP : %s, Port : %d Failed\n", g_pcCFG.NM.m_strNMAddr.c_str(), g_pcCFG.NM.m_nNMPort );
        return BJM_NOK;
    }

	return BJM_OK;
}

int CNMSession::MsgSend(string a_strPayload)
{
	int nret = 0;
	string strPayload;

	strPayload = a_strPayload;

	m_sock.SetPayload(strPayload.c_str());

	nret = m_sock.SendMesg();
	
    if (nret < 0) {
        g_pcLog->ERROR("message send failed : %s", m_sock.CSocket::m_strErrorMsg.c_str());
        return BJM_NOK;
    }

	m_sock.Clear();

	return BJM_OK;
}

int CNMSession::Regist(ST_COWORK_INFO *a_coworkinfo)
{

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);             // Src BJM
//    m_sock.SetSource(1, 10);				// Src BJM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->nm_proc_no);				// Src NM
//    m_sock.SetDestination(1, 8);				// Src NM
    m_sock.SetFlagRequest();			// response
	m_sock.SetCommand(BJM_REGIST);

    rabbit::object  menu = m_root["BODY"];

    menu["pkgname"]    = a_coworkinfo->pkg_name;
    menu["nodetype"]    = a_coworkinfo->node_type;
    menu["procname"]    = g_pcCFG.LOG.m_strProcName;
    menu["procno"]    = a_coworkinfo->bjm_proc_no;

	if(MsgSend(m_root.str()) == BJM_NOK)
		return BJM_NOK;

	return BJM_OK;
}

int CNMSession::Run()
{
	g_pcLog->INFO("CNMSession Run");

	if(m_nStartFlag == BJM_NOK)
		return BJM_NOK;

	return BJM_OK;
}

int CNMSession::RecvMsg(DB * a_pDB, ST_BatchRequest *a_batchrequest, ST_BatchResponse *a_batchresponse)
{
    int  nRecvFlag = 0;
	int  nRetRecv = 0;
    string strBuf;

	m_pDB = a_pDB;

	if( m_sock.IsConnected() == false)
	{
		Initial();
	}

	nRecvFlag = m_sock.RecvMesg(NULL, -1);
	if(nRecvFlag < 0)
	{
        g_pcLog->ERROR("message receive failed errorno=%d", errno);
//       	printf("socket, %s\n", m_sock.CSocket::m_strErrorMsg.c_str());
        g_pcLog->ERROR("socket, %s", m_sock.CSocket::m_strErrorMsg.c_str());
		m_sock.Clear();
		return BJM_RECV_ERROR;
	}

    m_sock.GetCommand(strBuf);

    // Receive
	if(strncmp(BJM_BATCH_RECONFIG, strBuf.c_str(), strBuf.length()) == 0)	// INIT
	{
		return BJM_RECV_RECONFIG;
	}
	else if(strncmp(BJM_BATCH_START_WSM, strBuf.c_str(), strBuf.length()) == 0)	// WSM_START
	{
		if(m_sock.IsFlagRequest())
		{
			if(GetRequestMsg(a_batchrequest) == BJM_NOK)
			{
    			g_pcLog->ERROR("GetRequestMsg, Error");
				m_sock.Clear();

				return BJM_NOK;
			}
		}
		
		m_sock.Clear();
		return BJM_RECV_WSM_START;
	}
	else if(strncmp(BJM_BATCH_START, strBuf.c_str(), strBuf.length()) == 0)	// START
	{
       	g_pcLog->DEBUG("BJM_RECV_START");
		return BJM_RECV_START;
	}
	else if(strncmp(BJM_REGIST, strBuf.c_str(), strBuf.length()) == 0)	// REGIST
	{
       	g_pcLog->INFO("BJM_RECV_REGIST");
		m_sock.Clear();
		return BJM_RECV_REGIST;
	}
	else if(strncmp(ATOM_PROC_CTL, strBuf.c_str(), strBuf.length()) == 0)	// PROC_CTRL
	{
       	g_pcLog->INFO("BJM_RECV_PROC_CTRL");
		nRetRecv = ProcessCtrl();
		return nRetRecv;
	}
	else if(strncmp(BJM_BACKUP, strBuf.c_str(), strBuf.length()) == 0)	// BACKUP
	{
		return BJM_RECV_BACKUP;
	}
	else if(strncmp(BJM_RESTORE, strBuf.c_str(), strBuf.length()) == 0)	// RESTORE
	{
		return BJM_RECV_RESTORE;
	}
	else if(strncmp(NODE_SCALE_IN, strBuf.c_str(), strBuf.length()) == 0)	// SCALE_IN
	{
		return BJM_RECV_SCALE_IN;
	}
	else if(strncmp(NODE_SCALE_OUT, strBuf.c_str(), strBuf.length()) == 0)	// SCALE_OUT
	{
		return BJM_RECV_SCALE_OUT;
	}
	else
	{
        g_pcLog->DEBUG("Check Action Message [%s]", strBuf.c_str());
		m_sock.Clear();
		return BJM_NOK;
	}
    
	return BJM_OK;
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

// Process Control의 구분
int CNMSession::ProcessCtrl()
{
	cout << "----- READ 2 " << endl;
    string              strLogPayload;
	string				strAction;
	int					nLogLevel = 0;				

    strLogPayload = m_sock.GetPayload();

    g_pcLog->INFO("request: [%s]", strLogPayload.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strLogPayload.c_str());

        strAction = string(doc["BODY"]["action"].as_string());
        cout << "[BODY][action]    :" << "[" << strAction << "]" << endl;

        nLogLevel = doc["BODY"]["loglevel"].as_int();
        cout << "[BODY][LogLevel]    :" << "[" << nLogLevel << "]" << endl;

		if(strAction.compare(BJM_ACTION_LOGLEVEL) == 0)	
		{
			g_pcLog->SetLogLevel(nLogLevel);
			g_pcLog->INFO("Cur Log Level %d", g_pcLog->GetLogLevel());
			m_sock.Clear();

			return BJM_OK;
		}
		else if(strAction.compare(BJM_ACTION_START) == 0)
		{
			m_sock.Clear();
			return BJM_RECV_PROC_START;
		}
		else if(strAction.compare(BJM_ACTION_STOP) == 0)
		{
			m_sock.Clear();
			return BJM_RECV_PROC_STOP;
		}
		else if(strAction.compare(BJM_ACTION_INIT) == 0)
		{
			m_sock.Clear();
			return BJM_RECV_RECONFIG;
		}
		else
		{
        	g_pcLog->INFO("ProcessCtrl, Check Action Message ");
		}

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(...) {
		m_sock.Clear();
        g_pcLog->ERROR("JSon Parsing Error");
		return BJM_NOK;
    }

	return BJM_NOK;
}

int CNMSession::SetRequestMsg(ST_BatchRequest *a_batchrequest)
{
	ST_BatchRequest 	stbatchrequest;
	stbatchrequest 		= *a_batchrequest;

    rabbit::object  menu = m_root["BODY"];

    g_pcLog->DEBUG("group_name [%s]", stbatchrequest.group_name);
    menu["group_name"]  = stbatchrequest.group_name;
    g_pcLog->DEBUG("job_name [%s]", stbatchrequest.job_name);
    menu["job_name"]    = stbatchrequest.job_name;
    g_pcLog->DEBUG("pkg_name [%s]", stbatchrequest.pkg_name);
    menu["pkg_name"]    = stbatchrequest.pkg_name;
    g_pcLog->DEBUG("proc_no [%d]", stbatchrequest.proc_no);
    menu["proc_no"]    = stbatchrequest.proc_no;
    g_pcLog->DEBUG("prc_date [%s]", stbatchrequest.prc_date);
    menu["prc_date"]    = stbatchrequest.prc_date;
    g_pcLog->DEBUG("exec_bin [%s]", stbatchrequest.exec_bin);
    menu["exec_bin"]    = stbatchrequest.exec_bin;
    g_pcLog->DEBUG("exec_arg [%s]", stbatchrequest.exec_arg);
    menu["exec_arg"]    = stbatchrequest.exec_arg;
    g_pcLog->DEBUG("exec_env [%s]", stbatchrequest.exec_env);
    menu["exec_env"]    = stbatchrequest.exec_env;

	if(MsgSend(m_root.str()) == BJM_NOK)
	{
        g_pcLog->ERROR("SetRequestMsg, Msg Send Error");
		return BJM_NOK;
	}

	return BJM_OK;
}

int CNMSession::SendInitResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_BatchRequest *a_batchrequest)
{
	ST_BatchRequest	st_batchrequest;
	st_batchrequest = *a_batchrequest;

	a_batchrequest->sequence = m_sock.GetSequence();
	m_sock.SetSequence(a_batchrequest->sequence);
	m_sock.SetCommand(BJM_BATCH_RECONFIG);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);				// Src BJM
    m_sock.SetDestination(a_coworkinfo->node_no,a_coworkinfo->wsm_proc_no);				// Src NM
    m_sock.SetFlagResponse();

    g_pcLog->DEBUG("src : id_snode : %d, id_sproc : %d, dst : id_dnode : %d, ip_dproc : %d",
	          st_batchrequest.id_snode, st_batchrequest.id_sproc, st_batchrequest.id_dnode, st_batchrequest.id_dproc);

	rabbit::object  	objSendRoot;

    rabbit::object  menu = objSendRoot["BODY"];

    menu["code"]    = 0;

	if(ret == BJM_NOK)
	{
	    menu["success"]    = false;
	}
	else
	{
	    menu["success"]    = true;
	}

	if(MsgSend(objSendRoot.str()) == BJM_NOK)
	{
        g_pcLog->ERROR("SendInitResponseMsg, Msg Send Error");
		return BJM_NOK;
	}

	return BJM_OK;
}


int CNMSession::SendWSMResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_BatchRequest *a_batchrequest, ST_BatchResponse *a_batchresponse)
{
	ST_BatchRequest	st_batchrequest;
	st_batchrequest = *a_batchrequest;
	ST_BatchResponse st_batchresponse;
	st_batchresponse = *a_batchresponse;

	a_batchresponse->sequence = m_sock.GetSequence();
	m_sock.SetSequence(a_batchresponse->sequence);

	m_sock.SetCommand(BJM_BATCH_START_WSM);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);				// Src BJM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);				// WSM
    m_sock.SetFlagResponse();			// response

	rabbit::object  	objSendRoot;

    rabbit::object  menu = objSendRoot["BODY"];

	menu["group_name"] = st_batchresponse.group_name;
	menu["job_name"] = st_batchresponse.job_name;
	menu["prc_date"] = st_batchresponse.prc_date;
    menu["code"]    = 0;
    menu["status"]  = st_batchresponse.status;
    menu["msg"]     = st_batchresponse.msg;

	if(ret == 0)
	{
	    menu["success"]    = false;
	}
	else
	{
	    menu["success"]    = true;
	}

	if(MsgSend(objSendRoot.str()) == BJM_NOK)
	{
        g_pcLog->ERROR("SetRequestMsg, Msg Send Error");
		return BJM_NOK;
	}

	return BJM_OK;
}

int CNMSession::SendRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_BatchJob *batchJobInfo_, ST_BatchRequest *a_batchrequest)
{
	string imsi;
	int nSendSeq = 0; 

	ST_BatchJob     	stbatchjob;
	ST_BatchRequest     *m_stbatchrequest = new ST_BatchRequest;
	memset(m_stbatchrequest, 0x00, sizeof(ST_BatchRequest));

	stbatchjob 			= *batchJobInfo_;
	m_stbatchrequest 	= a_batchrequest;

    m_sock.SetCommand(BJM_BATCH_START);	// BATCH_START 
    m_sock.SetFlagRequest();			// request

    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);				// Src BJM
    m_sock.SetDestination(stbatchjob.node_no, PROCID_ATOM_NA_PRA);		// Dst Node_No, Dst Process_No
	nSendSeq = m_sock.SetSequence();
	m_stbatchrequest->sequence = nSendSeq;
	m_nResponseSeq = nSendSeq;

	if(m_stbatchrequest->job_name != NULL && SetRequestMsg(m_stbatchrequest) == BJM_OK)
	{
		m_root.clear();
	}
	
	m_vBatchRequest.push_back(m_stbatchrequest);

	m_sock.Clear();

	return BJM_OK;
}

int CNMSession::GetRequestMsg(ST_BatchRequest *a_batchrequest)
{
	char				imsi[255];
	std::string			strbatchrequest;

	string 				group_name;
	string 				job_name;
	string 				pkg_name;
	string 				prc_date;
	string 				exec_bin;
	string 				exec_arg;
	string 				exec_env;
	string 				wsm_yn;

	strbatchrequest = m_sock.GetPayload();

    g_pcLog->DEBUG("strbatchrequest: [%s]", strbatchrequest.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strbatchrequest.c_str());

        group_name = string(doc["BODY"]["group_name"].as_string());
		strncpy(a_batchrequest->group_name, group_name.c_str(), group_name.length());

        job_name = string(doc["BODY"]["job_name"].as_string());
		strncpy(a_batchrequest->job_name, job_name.c_str(), job_name.length());

        pkg_name = string(doc["BODY"]["pkg_name"].as_string());
		strncpy(a_batchrequest->pkg_name, pkg_name.c_str(), pkg_name.length());

        prc_date = string(doc["BODY"]["prc_date"].as_string());
		strncpy(imsi, prc_date.c_str(), prc_date.length());
		strncpy(a_batchrequest->prc_date, space_remove(imsi), sizeof(imsi));

        exec_bin = string(doc["BODY"]["exec_bin"].as_string());
		strncpy(a_batchrequest->exec_bin, exec_bin.c_str(), exec_bin.length());

        exec_arg = string(doc["BODY"]["exec_arg"].as_string());
		strncpy(a_batchrequest->exec_arg, exec_arg.c_str(), exec_arg.length());

        exec_env = string(doc["BODY"]["exec_env"].as_string());
		strncpy(a_batchrequest->exec_env, exec_env.c_str(), exec_env.length());

        wsm_yn = string(doc["BODY"]["wsm_yn"].as_string());
		strncpy(a_batchrequest->wsm_yn, wsm_yn.c_str(), wsm_yn.length());

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(...) {
        g_pcLog->ERROR("GetRequestMsg, JSon Parsing Unkown Error");
		m_sock.Clear();
		return BJM_NOK;
    }

	return BJM_OK;
}

int CNMSession::GetResponseMsg(ST_BatchResponse *a_batchresponse, vector<ST_BatchRequest *> a_vBatchRequest)
{
	int				i = 0 ;
	int 			nResponseSeq = 0;
	string			strbatchresponse;
	string			end_date;
	string			status;
	int 			nNumber = 0;

	ST_BATCHHIST 	stbatchhist;
	
	nResponseSeq = m_sock.GetSequence();
	m_vBatchRequest = a_vBatchRequest;
	
	for(i = 0; i < (int)m_vBatchRequest.size(); i++)
	{

		if(m_vBatchRequest[i]->sequence == nResponseSeq)
		{
			g_pcLog->DEBUG("m_vBatchRequest[%d]->sequence %d, sequence  %d",i,  m_vBatchRequest[i]->sequence, nResponseSeq);

			strncpy(a_batchresponse->group_name, m_vBatchRequest[i]->group_name, sizeof(m_vBatchRequest[i]->group_name));
			strncpy(a_batchresponse->job_name, m_vBatchRequest[i]->job_name, sizeof(m_vBatchRequest[i]->job_name));
			strncpy(a_batchresponse->prc_date, m_vBatchRequest[i]->prc_date, sizeof(m_vBatchRequest[i]->prc_date));

			strncpy(stbatchhist.group_name, m_vBatchRequest[i]->group_name, sizeof(m_vBatchRequest[i]->group_name));
			strncpy(stbatchhist.job_name, m_vBatchRequest[i]->job_name, sizeof(m_vBatchRequest[i]->job_name));
			strncpy(stbatchhist.prc_date, m_vBatchRequest[i]->prc_date, sizeof(m_vBatchRequest[i]->prc_date));

			strncpy(a_batchresponse->wsm_yn, m_vBatchRequest[i]->wsm_yn, sizeof(m_vBatchRequest[i]->wsm_yn));
			
			strbatchresponse = m_sock.GetPayload();

    		try {
				rabbit::document    doc;
			    doc.parse(strbatchresponse.c_str());

				end_date.clear();
			    end_date = string(doc["BODY"]["end_date"].as_string());
				strncpy(stbatchhist.end_date, end_date.c_str(), end_date.length());
				strncpy(a_batchresponse->end_date, end_date.c_str(), end_date.length());
			    cout << "[BODY][end_date]    :" << "[" << stbatchhist.end_date << "]" << endl;

			    nNumber = doc["BODY"]["exit_cd"].as_int();
				stbatchhist.exit_cd = nNumber;
				a_batchresponse->exit_cd = nNumber;
			    cout << "[BODY][exit_cd]    :" << "[" << nNumber << "]" << endl;
			    
				status.clear();
				strcpy(a_batchresponse->status, "");
				status = string(doc["BODY"]["status"].as_string());
				strncpy(a_batchresponse->status, status.c_str(), status.length());
			    cout << "[BODY][status]    :" << "[" << a_batchresponse->status << "]" << endl;

				if(strncmp(a_batchresponse->status, "failed", 6) == 0 || strncmp(a_batchresponse->status, "pending", 7) == 0)
				{
					strncpy(stbatchhist.success_yn, "N", sizeof(char));
				}
				else
				{
					strncpy(stbatchhist.success_yn, "Y", sizeof(char));
				}

				m_sock.Clear();

		    } catch(rabbit::type_mismatch e) {
		        cout << e.what() << endl;
				m_sock.Clear();
				return BJM_NOK;
		    } catch(rabbit::parse_error e) {
		        cout << e.what() << endl;
				m_sock.Clear();
				return BJM_NOK;
		    } catch(...) {
		        g_pcLog->ERROR("GetResponseMsg, Unkown Error");
				return BJM_NOK;
		    }

		    if(m_BatchHist.HistUpdate(m_pDB, &stbatchhist) == BJM_NOK)
			{
		       	g_pcLog->ERROR("GetResponseMsg, HistUpdate Failure");
				return BJM_NOK;
			}
		}
	}

	m_vBatchRequest.erase(m_vBatchRequest.begin() + i -1);

	return BJM_OK;
}

int CNMSession::GetBackupRequest(ST_BACKUP_REQUEST *a_backuprequest)
{
    cout << "----- READ Backup Request" << endl;
	string					strbackuprequest;
	string					exec_bin;
	string					category;

    a_backuprequest->sequence = m_sock.GetSequence();

	strbackuprequest = m_sock.GetPayload();

    g_pcLog->INFO("strbackuprequest: [%s]", strbackuprequest.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strbackuprequest.c_str());

        exec_bin = string(doc["BODY"]["exec_bin"].as_string());
		a_backuprequest->exec_bin = exec_bin;
        cout << "[BODY][exec_bin]    :" << "[" << a_backuprequest->exec_bin << "]" << endl;

		rabbit::array a = doc["BODY"]["category"];
		rabbit::value v;
		for(auto nLoop = 0u; nLoop < a.size(); ++nLoop)
		{
			v= a.at(nLoop);
			category = v[nLoop].as_string();
			a_backuprequest->vCategory.push_back(category);
		}

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(...) {
        g_pcLog->ERROR("GetBackupRequest, Unkown Error");
		m_sock.Clear();
		return BJM_NOK;
    }

	return BJM_OK;
}

int CNMSession::SendBackupRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_BatchJob *batchJobInfo_, ST_BACKUP_REQUEST *a_backuprequest)
{
    string imsi;

    ST_BatchJob         stbatchjob;
    ST_BACKUP_REQUEST   stbackuprequest;

    stbatchjob          = *batchJobInfo_;
    stbackuprequest     = *a_backuprequest;
    imsi = m_sock.GetPayload();
    g_pcLog->INFO("CNMSession SendMsg [%s]", imsi.c_str());
    stbackuprequest.sequence = m_sock.GetSequence();

    m_sock.SetCommand(BJM_BACKUP);
    m_sock.SetFlagRequest();
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);             // Src BJM
    m_sock.SetDestination(stbatchjob.node_no, PROCID_ATOM_NA_PRA);      // Dst Node_No, Dst Process_No
	m_sock.SetSequence(stbackuprequest.sequence);

	if(MsgSend(imsi.c_str()) == BJM_NOK)
	{
        g_pcLog->ERROR("SetBackupRequestMsg, Msg Send Error");
		return BJM_NOK;
	}

    return BJM_OK;
}

int CNMSession::SetBackupResponse(ST_COWORK_INFO *a_coworkinfo, ST_BACKUP_REQUEST *a_backuprequest, bool a_ret)
{
    m_sock.SetSequence(a_backuprequest->sequence);
    m_sock.SetCommand(BJM_BACKUP);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);             // Src BJM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);                // WSM
    m_sock.SetFlagResponse();           // response

    rabbit::object  objSendRoot;
    rabbit::object  menu = objSendRoot["BODY"];

	for(int i = 0; i <  (int)a_backuprequest->vCategory.size(); i++)
	{
		menu["%s"] = a_backuprequest->vCategory[i];
	
		if(a_ret == false)
    	{
	        menu["success"] = false;
    	}
	    else
    	{
	        menu["success"] = true;
    	}
	}
//    cout << objSendRoot.str() << endl;

	if(MsgSend(objSendRoot.str()) == BJM_NOK)
	{
        g_pcLog->ERROR("SetBackupResponse, Msg Send Error");
		return BJM_NOK;
	}

	return BJM_OK;
}

int CNMSession::GetRestoreRequest(ST_RESTORE_REQUEST *a_restorerequest)
{
    cout << "----- READ Restore Request" << endl;
	std::string					strRestoreRequest;

	string restore_file;
	string restore_path;

    a_restorerequest->sequence = m_sock.GetSequence();

	strRestoreRequest = m_sock.GetPayload();

    g_pcLog->INFO("strRestoreRequest: [%s]", strRestoreRequest.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strRestoreRequest.c_str());

        restore_file = string(doc["BODY"]["restore_file"].as_string());
		a_restorerequest->restore_file = restore_file;
        cout << "[BODY][restore_file]    :" << "[" << a_restorerequest->restore_file << "]" << endl;

        restore_path = string(doc["BODY"]["restore_path"].as_string());
		a_restorerequest->restore_path = restore_path;
        cout << "[BODY][restore_path]    :" << "[" << a_restorerequest->restore_path << "]" << endl;

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
		m_sock.Clear();
		return BJM_NOK;
    } catch(...) {
        g_pcLog->ERROR("GetRestoreRequest, Unkown Error");
		m_sock.Clear();
		return BJM_NOK;
    }

	return BJM_OK;
}

int CNMSession::SendRestoreRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_BatchJob *batchJobInfo_, ST_RESTORE_REQUEST *a_restorerequest)
{
    string imsi;

    ST_BatchJob         stbatchjob;
    ST_RESTORE_REQUEST   strestorerequest;

    stbatchjob          = *batchJobInfo_;
    strestorerequest     = *a_restorerequest;
    imsi = m_sock.GetPayload();
    g_pcLog->INFO("CNMSession SendMsg [%s]", imsi.c_str());
    strestorerequest.sequence = m_sock.GetSequence();

    m_sock.SetCommand(BJM_RESTORE);
    m_sock.SetFlagRequest();            // request
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);             // Src BJM
    m_sock.SetDestination(stbatchjob.node_no, PROCID_ATOM_NA_PRA);      // Dst Node_No, Dst Process_No
	m_sock.SetSequence(strestorerequest.sequence);

	if(MsgSend(imsi.c_str()) == BJM_NOK)		
	{
        g_pcLog->ERROR("SetRestoreRequestMsg, Msg Send Error");
		return BJM_NOK;
	}

    return BJM_OK;
}

int CNMSession::GetBackupResponse(ST_COWORK_INFO *a_coworkinfo, ST_BACKUP_REQUEST *a_backuprequest)
{
	int					nSequence = 0;
    string              strBackupResponse;
    string 				strSuccess;
	string				strCategory;

    nSequence = m_sock.GetSequence();
    strBackupResponse = m_sock.GetPayload();

    g_pcLog->INFO("strBackupResponse : [%s]", strBackupResponse.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strBackupResponse.c_str());

		rabbit::array a = doc["BODY"];
		rabbit::value v;
		for(auto nLoop=0u; nLoop < a.size(); ++nLoop)
		{
			v = a.at(nLoop);
			strCategory = v["category"].as_string();
			strSuccess = v["success"].as_string();
		}

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("GetBackupResponse Unkown Error");
        m_sock.Clear();
    }

	if(nSequence == a_backuprequest->sequence)
		return BJM_OK;
	else
		return BJM_NOK;
}

int CNMSession::GetRestoreResponse(ST_COWORK_INFO *a_coworkinfo, ST_RESTORE_REQUEST *a_restorerequest)
{
	int					nSequence = 0;
    string              strRestoreResponse;
    string 				strSuccess;	

    nSequence = m_sock.GetSequence();
    strRestoreResponse = m_sock.GetPayload();

    g_pcLog->INFO("strRestoreResponse : [%s]", strRestoreResponse.c_str());

    try {
        rabbit::document    doc;
        doc.parse(strRestoreResponse.c_str());

        strSuccess = string(doc["BODY"]["success"].as_string());
        cout << "[BODY][success]    :" << "[" << strSuccess << "]" << endl;

    } catch(rabbit::type_mismatch   e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(rabbit::parse_error e) {
        cout << e.what() << endl;
        m_sock.Clear();
    } catch(...) {
        g_pcLog->INFO("GetRestoreResponse Unkown Error");
        m_sock.Clear();
    }

	if(nSequence == a_restorerequest->sequence && strSuccess.find("true") != string::npos) 
		return BJM_OK;
	else
		return BJM_NOK;
}

int CNMSession::SetRestoreResponse(ST_COWORK_INFO *a_coworkinfo, ST_RESTORE_REQUEST *a_restorerequest, bool a_ret)
{
    m_sock.SetSequence(a_restorerequest->sequence);

    m_sock.SetCommand(BJM_RESTORE);
    m_sock.SetSource(a_coworkinfo->node_no, a_coworkinfo->bjm_proc_no);             // Src BJM
    m_sock.SetDestination(a_coworkinfo->node_no, a_coworkinfo->wsm_proc_no);                // WSM
    m_sock.SetFlagResponse();           // response

    rabbit::object  objSendRoot;
    rabbit::object  menu = objSendRoot["BODY"];

    if(a_ret == false)
    {
        menu["success"]    = false;
    }
    else
    {
        menu["success"]    = true;
    }

//    cout << objSendRoot.str() << endl;
    m_sock.SetPayload(objSendRoot.str());

	if(MsgSend(objSendRoot.str()) == BJM_NOK)
	{
        g_pcLog->ERROR("SetRestoreResponse, Msg Send Error");
		return BJM_NOK;
	}

	return BJM_OK;
}

int CNMSession::Final()
{
	m_sock.Close();

	return BJM_OK;
}

int CNMSession::GetSequence()
{
	return m_nResponseSeq;
}

char* CNMSession::space_remove(char *a_str)
{
  // change SPACE_REMOVE_BUF_SIZE depending on the length of your string
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
