#include "CTraceAPI.hpp"
#include "CProtocol.hpp"
#include "TRM_Define.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <string>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <vector>

using namespace std;

CTraceAPI::CTraceAPI()
{
	m_nRunMode = 0;
	m_nTraceCount = 0;
}

CTraceAPI::~CTraceAPI()
{

}

int CTraceAPI::GetTraceRequest(CProtocol *a_pclsReqProtocol, CProtocol *a_pclsResProtocol)
{
	//Trace On 메시지의 Request 메시지 수신
    char                        imsi[255];
	string						strCommand;
    std::string                 strTraceRequest;

	ST_TRACE_REQUEST *m_stTraceRequest = new ST_TRACE_REQUEST;
	memset(m_stTraceRequest, 0x00, sizeof(ST_TRACE_REQUEST));

	strCommand = a_pclsReqProtocol->GetCommand();
	strTraceRequest = a_pclsReqProtocol->GetPayload();
	m_stTraceRequest->sequence = a_pclsReqProtocol->GetSequence();

	try {
    	rabbit::document    doc;
        doc.parse(strTraceRequest.c_str());

	    long int oper_no = doc["BODY"]["oper_no"].as_int64();
    	m_stTraceRequest->oper_no = oper_no;

	    string pkg_name = string(doc["BODY"]["pkg_name"].as_string());
    	strncpy(m_stTraceRequest->pkg_name, pkg_name.c_str(), pkg_name.length());

	    int node_no = doc["BODY"]["node_no"].as_int();
    	m_stTraceRequest->node_no = node_no;

	    string trace = string(doc["BODY"]["trace"].as_string());
    	strncpy(m_stTraceRequest->trace, trace.c_str(), trace.length());

	    int protocol = doc["BODY"]["protocol"].as_int();
    	m_stTraceRequest->protocol = protocol;

	    int search_mode = doc["BODY"]["search_mode"].as_int();
    	m_stTraceRequest->search_mode = search_mode;

	    string keyword = string(doc["BODY"]["keyword"].as_string());
    	strncpy(m_stTraceRequest->keyword, keyword.c_str(), keyword.length());

	    int run_mode = doc["BODY"]["run_mode"].as_int();
    	m_stTraceRequest->run_mode = run_mode;

	    string user_id = string(doc["BODY"]["user_id"].as_string());
    	strncpy(m_stTraceRequest->user_id, user_id.c_str(), user_id.length());

	    string start_date = string(doc["BODY"]["start_date"].as_string());
    	strncpy(imsi, start_date.c_str(), start_date.length());
        strncpy(m_stTraceRequest->start_date, space_remove(imsi), sizeof(imsi));

    	if(strCommand.find(TRM_TRACE_OFF) != string::npos)		// Trace OFF
        {
			m_stTraceRequest->bTraceOff = true;
	        string end_date = string(doc["BODY"]["end_date"].as_string());
    	    strncpy(imsi, end_date.c_str(), end_date.length());
            strncpy(m_stTraceRequest->end_date, space_remove(imsi), sizeof(imsi));
		}

		m_vTraceRequest.push_back(m_stTraceRequest);
		m_nTraceCount++;	
	
		SetTraceResponse(a_pclsResProtocol, m_stTraceRequest, true);
	} catch(rabbit::type_mismatch   e) {
    	    cout << e.what() << endl;
			return -1;
	} catch(rabbit::parse_error e) {
        	cout << e.what() << endl;
			return -1;
	} catch(...) {
    	    fprintf(stderr, "Unkown Error\n");
			return -1;
	}

	return 0;
}

bool CTraceAPI::IsOn()
{
	if (m_nTraceCount == 0)
		return false;
	else
		return true;
}

int CTraceAPI::CheckIMSI(string a_TargetString)
{
	unsigned int unRet = 0;

	unRet = CheckString(1, a_TargetString);

	return unRet;
}

int CTraceAPI::CheckMSISDN(string a_TargetString)
{
	unsigned int unRet = 0;

	unRet = CheckString(2, a_TargetString);
	
	return unRet;
}

int CTraceAPI::CheckCallingIP(string a_TargetString)
{
	unsigned int unRet = 0;

	unRet = CheckString(3, a_TargetString);

	return unRet;
}

int CTraceAPI::CheckCalledIP(string a_TargetString)
{
	unsigned int unRet = 0;

	unRet = CheckString(4, a_TargetString);

	return unRet;
}

int CTraceAPI::CheckString(int a_nMode, string a_TargetString)
{
	unsigned int unRet = 0;
	size_t found;

	for (unRet = 0; unRet < m_vTraceRequest.size();unRet++)
	{
		if(a_nMode == m_vTraceRequest[unRet]->search_mode)
		{
			found = a_TargetString.find(m_vTraceRequest[unRet]->keyword);
			if(found == string::npos)
				return -1;
			else
				return unRet;
		}
	}

	return -1;
}

void CTraceAPI::SetTraceData(CProtocol *a_pclsResProtocol, int a_nIdx, bool a_bRet, string a_strData)
{
	ST_TRACE_REQUEST *pstTraceReq = m_vTraceRequest[a_nIdx];
	char  curTime[16];
	time_t now;

	time(&now);

	struct tm today;
	today = *localtime(&now);
	
	sprintf(curTime, "%04d%02d%02d%02d%02d%02d", today.tm_year+1900, today.tm_mon+1, today.tm_mday, today.tm_hour, today.tm_min, today.tm_sec);
    rabbit::object      objRecvRoot;

	if(pstTraceReq->oper_no > 0)
	{		
	    a_pclsResProtocol->SetFlagResponse();           // response
    	a_pclsResProtocol->SetSequence(pstTraceReq->sequence);
	    a_pclsResProtocol->SetCommand(TRM_TRACE_ON);

	    rabbit::object  menu = objRecvRoot["BODY"];

	    menu["oper_no"]     = pstTraceReq->oper_no;
	    menu["trace"]       = "on";
		menu["status"]      = "process";
	    menu["start_date"]  = pstTraceReq->start_date;
		menu["end_date"]    = curTime;
	    menu["msg"]         = a_strData.c_str();

	    if(a_bRet == false)
    	{
        	menu["success"]    = false;
	    }
    	else
	    {
    	    menu["success"]    = true;
	    }
//    	cout << objRecvRoot.str() << endl;
	}
	
	a_pclsResProtocol->SetPayload(objRecvRoot.str());
//		SendMessage(objRecvRooot);
	return ;
}

int CTraceAPI::SetTraceResponse(CProtocol *a_pclsResProtocol, ST_TRACE_REQUEST *a_tracerequest, bool a_bRet)
{
	unsigned int 				nRet = 0;
	a_pclsResProtocol->SetFlagResponse();
    rabbit::object      objRecvRoot;

    rabbit::object  menu = objRecvRoot["BODY"];

    menu["oper_no"]     = a_tracerequest->oper_no;
    menu["trace"]       = a_tracerequest->trace;
    menu["start_date"]  = a_tracerequest->start_date;
    menu["end_date"]    = a_tracerequest->end_date;
    menu["msg"]         = "";

    if(a_bRet == false)
    {
        menu["success"]    = false;
    }
    else
    {
        menu["success"]    = true;
    }

	if(a_tracerequest->bTraceOff)
	{
    	a_pclsResProtocol->SetCommand(TRM_TRACE_OFF);
	    menu["status"]      = "stop";

		for (nRet = 0; nRet < m_vTraceRequest.size();nRet++)
		{
			if(a_tracerequest->oper_no == m_vTraceRequest[nRet]->oper_no)
		    {
				m_vTraceRequest.erase(m_vTraceRequest.begin() + nRet);		// Delete
		    }
   		}
		m_nTraceCount--;	
	}
	else 
	{
    	a_pclsResProtocol->SetCommand(TRM_TRACE_ON);
	    menu["status"]      = "start";
	}

	a_pclsResProtocol->SetPayload(objRecvRoot.str());

	return 0;
}

string CTraceAPI::SetTraceProcess(string a_strPkgName, string a_strNode_type, string a_strProc_name, int a_nProcNo, string a_cmd, int a_node_no, string a_node_name)
{
	m_clsProtocol.SetCommand(TRM_TRACE_REGIST_ROUTE);
	m_clsProtocol.SetFlagRequest();

	rabbit::object 	root;

	rabbit::object 	body = root["BODY"];
	body["pkg_name"] = a_strPkgName;
	body["cmd"] 	 = a_cmd;
	body["node_no"] = a_node_no;
	body["node_name"] = a_node_name;
	body["node_type"] = a_strNode_type;
	body["proc_name"] = a_strProc_name;
	body["proc_no"] = a_nProcNo;

//	cout << root.str() << endl;
//	SendMessage(root.str());
	
	return root.str();
}

int CTraceAPI::SendMessage(string a_strMessage)
{
/*    m_clsProtocol.SetPayload(a_strMessage.c_str());

    if (m_clsProtocol.SendMesg() == false) 
    {
        fprintf(stderr, "message send failed\n");
        return false;
    }       
  */          
    return true;
}

char* CTraceAPI::space_remove(char *a_str)
{
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

