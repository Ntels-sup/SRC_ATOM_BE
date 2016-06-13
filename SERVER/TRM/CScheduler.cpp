#include "TRM_Define.hpp"
#include "CScheduler.hpp"
#include "CNMSession.hpp"
#include "CATMDate.hpp"
#include "CATMTime.hpp"
#include "MariaDB.hpp"
#include <vector>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

//extern symbol
extern CFileLog* g_pcLog;
extern CConfigTRM g_pcCFG;
extern CNMSession g_pcNM;

CScheduler::CScheduler()
{
    m_pDB = NULL;

	m_nStartFlag = TRM_OK;
	m_nTraceRouteCount = 0;

	m_nDBInitFlag = 0;			//start

	if(Initial() != TRM_OK)
	{
		g_pcLog->INFO("CScheduler Initialize error");
		m_nStartFlag = TRM_NOK;
	}
}

CScheduler::~CScheduler()
{
    if(m_pDB != NULL)
    {
        delete m_pDB;
        m_pDB = NULL;
    }
}

int CScheduler::Initial()
{
	int nDbRet = 0;
	g_pcLog->INFO("CScheduler Initialize");

    m_pDB = new (std::nothrow) MariaDB();
    if(m_pDB == NULL)
    {
        g_pcLog->INFO("new operator Fail [%d:%s]", errno, strerror(errno));
        return TRM_NOK;
    }

    // CONNECT
//    if(m_pDB->Connect("127.0.0.1", 3306, "atom", "atom", "ATOM")  != true)
    nDbRet = m_pDB->Connect(g_pcCFG.DB.m_strDbAddr.c_str(), 
							g_pcCFG.DB.m_nDbPort, 
							g_pcCFG.DB.m_strDbuserId.c_str(), 
							g_pcCFG.DB.m_strDbPasswd.c_str(), 
							g_pcCFG.DB.m_strDbName.c_str());

	if(nDbRet < 0)
    {
        g_pcLog->INFO("Connect Fail [%d:%s]", nDbRet, m_pDB->GetErrorMsg(nDbRet));
        return TRM_NOK;
    }

    g_pcLog->INFO("DB Connection ...OK");

	// TAT_TRC_ROUTE init
    if(m_TraceRoute.Init(m_pDB) != TRM_OK)
    {
        g_pcLog->INFO("CTraceRoute - Init fail\n");
        return TRM_NOK;
    }

	// TAT_TRC_HIST init
    if(m_TraceHist.Init(m_pDB) != TRM_OK)
    {
        g_pcLog->INFO("CTraceHist - Init fail\n");
        return TRM_NOK;
    }

    if(m_Process.Init(m_pDB) != TRM_OK)
    {
        g_pcLog->INFO("CProcess - Init fail\n");
        return TRM_NOK;
    }

//  if(LoadCoworkNo("ATOM", "NM", "TRM", "WSM") == TRM_NOK)
    if(m_Process.LoadCoworkNo((char *)g_pcCFG.LOG.m_strNodeName.c_str(), 
							  (char *)g_pcCFG.LOG.m_strNmName.c_str(), 
							  (char *)g_pcCFG.LOG.m_strTrmName.c_str(), 
							  (char *)g_pcCFG.LOG.m_strWsmName.c_str(), 
							  m_stcoworkinfo) == TRM_NOK)
    {
        g_pcLog->INFO("CScheduler LoadCoworkNo Error");
        return TRM_NOK;
    }
        g_pcLog->INFO("Pkg_Name : %s, Node_Type : %s, Node_No : %d, Proc_No(NM) : %d, Proc_No(TRM) : %d, Proc_No(WSM) : %d"
            , m_stcoworkinfo->pkg_name
            , m_stcoworkinfo->node_type
            , m_stcoworkinfo->node_no
            , m_stcoworkinfo->nm_proc_no
            , m_stcoworkinfo->trm_proc_no
            , m_stcoworkinfo->wsm_proc_no);


	g_pcLog->INFO("Data Initialization - OK. ");

	return TRM_OK;
}

int CScheduler::Run()
{	
	g_pcLog->INFO("CScheduler Run");
	if(m_nStartFlag == TRM_NOK)
		return false;

	if(LoadTraceInfo() == TRM_NOK)
	{
		g_pcLog->INFO("CScheduler LoadTraceInfo Error");
		return false;
	}

    if(g_pcNM.Regist(m_stcoworkinfo) == TRM_NOK)
    {
        return false;
    }

	CheckCurrentTime();
	
	return true;

}

void CScheduler::GetRouteDetailInfo()
{
	int  i = 0;

	for( i = 0; i < (int)m_vTraceRoute.size(); i++)
	{
	    if(strncmp(m_vTraceRoute[i]->pkg_name, m_stTraceRequest->pkg_name, sizeof(m_stTraceRequest->pkg_name)) == 0 &&
 	       strncmp(m_vTraceRoute[i]->node_type, m_stTraceRequest->node_type, sizeof(m_stTraceRequest->node_type)) == 0 &&
	        m_vTraceRoute[i]->node_no == m_stTraceRequest->node_no)
	    {
		    m_stTraceRequest->proc_no = m_vTraceRoute[i]->proc_no;
		    strncpy(m_stTraceRequest->cmd, m_vTraceRoute[i]->cmd, sizeof(m_vTraceRoute[i]->cmd));
		}
	}

}

void CScheduler::SetTraceResponse(char * a_strCurrent_date)
{
	char strCurrent_date[TRM_GROUP_START_DATE_SIZE + 1] = "";

	strncpy(strCurrent_date, a_strCurrent_date, sizeof(TRM_GROUP_START_DATE_SIZE +1));

	m_stTraceResponse->sequence = m_stTraceRequest->sequence;
	m_stTraceResponse->oper_no = m_stTraceRequest->oper_no;
	strncpy(m_stTraceResponse->pkg_name, m_stTraceRequest->pkg_name, sizeof(m_stTraceRequest->pkg_name));
	strncpy(m_stTraceResponse->trace, m_stTraceRequest->trace, sizeof(m_stTraceRequest->trace));
	strncpy(m_stTraceResponse->start_date, m_stTraceRequest->start_date, sizeof(m_stTraceRequest->end_date));
	strncpy(m_stTraceResponse->trace, m_stTraceRequest->trace, sizeof(m_stTraceRequest->trace));
	strncpy(m_stTraceResponse->status, "finish", sizeof("finish"));
	strncpy(m_stTraceResponse->end_date, strCurrent_date, sizeof(strCurrent_date));
	sprintf(m_stTraceResponse->filename, "%s/TRACE_%ld", (char *)g_pcCFG.TRM.m_strTraceDataPath.c_str(), m_stTraceResponse->oper_no);
	m_stTraceResponse->success = false;

}

int CScheduler::ReceiveData(ST_COWORK_INFO a_stcoworkinfo)
{
	CATMTime nowT;
	char strCurrent_date[TRM_GROUP_START_DATE_SIZE + 1] = "";
	char strTemp_date[TRM_GROUP_START_DATE_SIZE + 1] = "";
	char strExpire_date[TRM_GROUP_START_DATE_SIZE + 1] = "";
	unsigned int  nElaspsed = 0;
	unsigned int  nTraceTimeOut = 0;
	int  nRecvFlag = 0;
	int  nCommand = 0;
	int  nTimeoutFlag = 0;

	nowT = time(NULL);

// Receive Message
	nRecvFlag = g_pcNM.RecvMsg();
	nCommand = g_pcNM.IsRequest();
    ST_COWORK_INFO          stcoworkinfo;
    stcoworkinfo = a_stcoworkinfo;

//	g_pcLog->INFO("nRecvFlag[%d], nCommand [%d]", nRecvFlag, nCommand);
	switch(nRecvFlag)
	{
		case TRM_RECV_TRACE_ON:
			if(nCommand == 0)	// Request
			{
				if(g_pcNM.GetRequestMsg(0, m_stTraceRequest) == TRM_NOK)
				{       
					g_pcLog->INFO("RecvRequestMsg : false");
					return TRM_NOK;
				}
				else
				{
					GetRouteDetailInfo();

				    if(g_pcNM.SendRequestMsg(&stcoworkinfo, m_stTraceRequest) == TRM_NOK)
				    {
				        g_pcLog->INFO("SendRequestMsg, Error");
				        return TRM_NOK;
				    }
					
					g_pcLog->INFO("CreateTraceFile [%x]", m_stTraceRequest);
					if(CreateTraceFile(m_stTraceRequest) == TRM_NOK)
					{
						g_pcLog->INFO("CreateTraceFile Error[%x]", m_stTraceRequest);
					}

// 	Debug - Web 답변 처리 위한 
//					if(g_pcNM.SendWSMResponseMg(&stcoworkinfo, m_stTraceRequest, 0, 1)  == TRM_NOK )
//					{
//					    g_pcLog->INFO("SendWSMResponseMsg Error");
//					    return TRM_NOK;
//					}

					// Time Out 시간 60초 이후 Trace Off 발생시킴
					sprintf(strTemp_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));
				    g_pcLog->DEBUG("strCurrent_date : %s, strTemp_date : %s", strCurrent_date, strTemp_date);
					sprintf(strExpire_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));

					while(1)
					{
						nowT.Now();
		        		sprintf(strCurrent_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));
						nElaspsed = atoi(strCurrent_date) - atoi(strTemp_date);		// no data
						nTraceTimeOut = atoi(strCurrent_date) - atoi(strExpire_date);	// Auto Expire Time
		
						if(nElaspsed >= g_pcCFG.TRM.m_nTimeout || nTraceTimeOut >= g_pcCFG.TRM.m_nAutoExpireTime)		//60
						{

							nTimeoutFlag = 1;

			            	g_pcLog->INFO("strCurrent_date : %s, strTemp_date : %s", strCurrent_date, strTemp_date);
							SetTraceResponse(strCurrent_date);
		        		  	break;
						}
					}
					if(nTimeoutFlag == 1)
					{				
						nTimeoutFlag = 0;
						goto GoTo_TimeOut;
					}
				}
			}
			else if(nCommand == 1) // Response
			{
				nTimeoutFlag = 0;
				if(g_pcNM.GetResponseMsg(m_stTraceResponse) == TRM_NOK)				
				{
					g_pcLog->INFO("SendResponseMsg False");
					if(g_pcNM.SendWSMResponseMsg(&stcoworkinfo, m_stTraceResponse, 0, false)  == TRM_NOK )
					{
					    g_pcLog->INFO("SendWSMResponseMsg Error");
					    return TRM_NOK;
					}				    
				}
				else
				{
					g_pcLog->INFO("SendResponseMsg True");
				    if(g_pcNM.SendWSMResponseMsg(&stcoworkinfo, m_stTraceResponse, 0, true)  == TRM_NOK )	
				    {
				        g_pcLog->INFO("SendWSMResponseMsg Error");
				        return TRM_NOK;
				    }
				}

			}
			else
			{
				g_pcLog->INFO("RecvRequestMsg : Check Command info %d", nCommand);
				return TRM_NOK;
			}

			break;
		case TRM_RECV_TRACE_OFF:

			nTimeoutFlag = 0;
			if(nCommand == 0)	// Request
			{
				if(g_pcNM.GetRequestMsg(1, m_stTraceRequest) == TRM_NOK)	// From WSM
				{       
					g_pcLog->INFO("RecvRequestMsg : false");
					return TRM_NOK;
				}
				else
				{
					GetRouteDetailInfo();

					if(g_pcNM.SendRequestMsg(m_stcoworkinfo, m_stTraceRequest)  == TRM_NOK)
					{	
			        	g_pcLog->INFO("SendRequestMsg, Error");
						return TRM_NOK;
					}
				}

// 	Debug - Web 답변 처리 위한 
//				if(g_pcNM.SendWSMResponseMg(&stcoworkinfo, m_stTraceRequest, 1, 1)  == TRM_NOK )
//				{
//				    g_pcLog->INFO("SendWSMResponseMsg Error");
//				    return TRM_NOK;
//				}
			}
			else if(nCommand == 1) // Response
			{
				if(g_pcNM.GetResponseMsg(m_stTraceResponse) == TRM_NOK )				
				{
GoTo_TimeOut:				
					g_pcLog->INFO("RecvResponseOffMsg : false");
					if(g_pcNM.SendWSMResponseMsg(&stcoworkinfo, m_stTraceResponse, 1, false)  == TRM_NOK )
					{
					    g_pcLog->INFO("SendWSMResponseMsg Error");
					}				    
					return TRM_NOK;

				}
				else
				{
					SetTraceOffHist(m_stTraceResponse);		
					g_pcLog->INFO("SetTraceOffHist m_stTraceResponse [%x]", m_stTraceResponse);
					if(g_pcNM.SendWSMResponseMsg(&stcoworkinfo, m_stTraceResponse, 1, true)  == TRM_NOK )
					{
					    g_pcLog->INFO("SendWSMResponseMsg Error");
					}				    
				}

			}
			else
			{
				g_pcLog->INFO("RecvRequestMsg : Check Command info %d", nCommand);
				return TRM_NOK;
			}	
			break;
		case TRM_RECV_TRACE_DATA:
//trace_data_debug:
			//Recv Data는 Response
			nTimeoutFlag = 1;
			sprintf(strTemp_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));

			if(g_pcNM.GetTraceData(m_stTraceResponse) == TRM_NOK)
//			if(g_pcNM.TestWebGetTraceData(m_stTraceRequest->oper_no, 
//						m_stTraceRequest->start_date, 
//						strTemp_date, 
//						m_stTraceResponse)== TRM_NOK)
			{
				g_pcLog->INFO("RecvResponseOffMsg : false");
				return TRM_NOK;
			}
			else
			{
				g_pcLog->INFO("SendResponseMsg True");
				if(g_pcNM.SendWSMResponseMsg(&stcoworkinfo, m_stTraceResponse, 2, true)  == TRM_NOK )	
				{
				   g_pcLog->INFO("SendWSMResponseMsg Error");
				   return TRM_NOK;
				}
                WriteTraceFile(m_stTraceResponse);
				g_pcLog->DEBUG("SetTraceOffHist m_stTraceResponse [%x]", m_stTraceResponse);
			}

			break;
		case TRM_RECV_REGIST:
			g_pcLog->INFO("TRM Recv Regist OK");
			break;
        case TRM_RECV_INIT:
            if(LoadTraceInfo() == TRM_OK)
            {
    			g_pcLog->INFO("TRM Recv Reconfig OK");
            }
            else
            {
    			g_pcLog->INFO("TRM Recv Reconfig Failure");
            }
            break;
		case TRM_RECV_REGIST_ROUTE:
			if(g_pcNM.GetRegistRouteInfo(m_stTraceRoute) == TRM_NOK)
			{
				g_pcLog->INFO("GetRegistRouteInfo : false");
				return TRM_NOK;
			}
			else
			{
				m_TraceRoute.InsertRouteInfo(m_stTraceRoute);
			}
			// Init
            if(LoadTraceInfo() == TRM_OK)
            {
    			g_pcLog->INFO("TRM Recv Reconfig OK");
            }
			break;
        case TRM_RECV_ERROR:
            exit(-1);
            break;
//		dafault:
//			g_pcLog->INFO("debug [%d]", nRecvFlag);
//		 	break;
	}
	return TRM_OK;
}

int CScheduler::CreateTraceFile(ST_TRACE_REQUEST *a_tracerequest)
{
	char strFileName[1024];

	ST_TRACE_REQUEST stTraceRequest;
	stTraceRequest = *a_tracerequest;

	memset(strFileName, 0x00, sizeof(strFileName));

	sprintf(strFileName, "%s/TRACE_%ld"
						, (char *)g_pcCFG.TRM.m_strTraceDataPath.c_str()
						, stTraceRequest.oper_no);

	g_pcLog->INFO("strFileName [%s]", strFileName);

    if(access(strFileName, R_OK) == -1)
    {
        mkdir((char *)g_pcCFG.TRM.m_strTraceDataPath.c_str(), 0777);
    }

    FILE* fp = fopen(strFileName, "a");
    if(!fp)
    {
        g_pcLog->INFO("Can not Open File [%s]", strFileName);
    }

	SetTraceHist(strFileName, m_stTraceRequest);

	return TRM_OK;
}

int CScheduler::WriteTraceFile(ST_TRACE_RESPONSE *a_traceresponse)
{
	char strFileName[1024];

	ST_TRACE_RESPONSE stTraceResponse;
	stTraceResponse = *a_traceresponse;

	memset(strFileName, 0x00, sizeof(strFileName));

	g_pcLog->INFO("WriteTraceFile [%s]", strFileName);

	sprintf(strFileName, "%s/TRACE_%ld"
						, (char *)g_pcCFG.TRM.m_strTraceDataPath.c_str()
						, stTraceResponse.oper_no);

	g_pcLog->INFO("WriteTraceFile strFileName [%s]", strFileName);

	if(access(strFileName, R_OK) == -1)
	{
		mkdir((char *)g_pcCFG.TRM.m_strTraceDataPath.c_str(), 0777);
    }

	FILE* fp = fopen(strFileName, "a");
	if(!fp)
	{
		g_pcLog->INFO("Can not Open File [%s]", strFileName);
	}

    fseek(fp, 0, SEEK_END);
    fwrite(stTraceResponse.msg, strlen(stTraceResponse.msg), 1, fp);	
	fclose(fp);

	return TRM_OK;
}

int CScheduler::CheckCurrentTime()
{
	CATMTime nowT;
//    char strCurrent_date[TRM_GROUP_START_DATE_SIZE + 1] = "";
//    char strTemp_date[TRM_GROUP_START_DATE_SIZE + 1] = "";
//	int  nElaspsed = 0;

    ST_COWORK_INFO         stcoworkinfo;
    stcoworkinfo =  *m_stcoworkinfo;

	nowT = time(NULL);
	
//	sprintf(strTemp_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));

	while(1)	
	{
		// Check Current Time For Debug
		nowT.Now();
		
//		sprintf(strCurrent_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));

//		if(nElaspsed >= 60)	//60 Sec
//		{
//			g_pcLog->INFO("strCurrent_date : %s, strTemp_date : %s", strCurrent_date, strTemp_date);
//            sprintf(strTemp_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));
//            nElaspsed = 0;
//		}

		if(ReceiveData(stcoworkinfo) == TRM_RECV_ERROR)
		{
			break;
		}
		sleep(100);
	}

	return TRM_OK;
}


int CScheduler::LoadTraceInfo()
{
	int i = 0;	// TraceRoute

	m_nTraceRouteCount = m_TraceRoute.LoadRouteInfo(m_stTraceRoute);
	if(m_nTraceRouteCount < 0)
	{
		g_pcLog->INFO("LoadRouteInfo Error");
		return TRM_NOK;
	}
	else
	{
		for (i = 0; i < (int)m_nTraceRouteCount; i++)
		{
			m_vTraceRoute.push_back(&m_stTraceRoute[i]);
			g_pcLog->INFO("m_stTraceRoute[%d].pkg_name [%s]", i, m_stTraceRoute[i].pkg_name);

			if(LoadProcessInfo(i) == TRM_NOK)
			{
				g_pcLog->INFO("LoadProcInfo Error");
				return TRM_NOK;
			}
		}
	}

	return TRM_OK;
}

int CScheduler::LoadProcessInfo(int a_routeCnt)
{
	int j = 0;	// Process

	m_nProcessCount[a_routeCnt] = m_Process.LoadProcInfo(m_stProcess, &m_stTraceRoute[a_routeCnt]);
	if(m_nProcessCount[a_routeCnt] != 0)
	{
		for(j = 0; j < (int)m_nProcessCount[a_routeCnt]; j++)
		{
			m_vProcess.push_back(&m_stProcess[j]);
			g_pcLog->INFO("m_stProcess[%d].node_name [%s]", j, m_stProcess[j].node_name);
		}
	}
	else
	{
		return TRM_NOK;
	}

	return TRM_OK;
}


int CScheduler::SetTraceHist(char *a_FileName, ST_TRACE_REQUEST *a_tracerequest)
{
	int i = 0; 
    ST_TRACE_HIST    stTraceHist;
    ST_TRACE_REQUEST stTraceRequest;

    stTraceRequest = *a_tracerequest;

    stTraceHist.oper_no = stTraceRequest.oper_no;
    strncpy(stTraceHist.pkg_name, stTraceRequest.pkg_name, sizeof(stTraceRequest.pkg_name));
	
	for( i =0; i < (int)m_vProcess.size(); i++)
	{
		g_pcLog->INFO("m_vProcess[%d] : pkg_name : [%s], node_type :[%s], proc_no : [%d], node_name : [%s]", 
				i, 
				m_vProcess[i]->pkg_name, 
				m_vProcess[i]->node_type, 
				m_vProcess[i]->proc_no, 
				m_vProcess[i]->node_name);

//		if(strcmp(m_vProcess[i]->pkg_name, stTraceRequest.pkg_name) == 0 &&
//			m_vProcess[i]->node_type == stTraceRequest.node_type && 
//			m_vProcess[i]->proc_no == stTraceRequest.proc_no)
//		{
    		strncpy(stTraceHist.node_name, m_vProcess[i]->node_name, sizeof(m_vProcess[i]->node_name));
    		sprintf(stTraceHist.cmd, "%s %s", m_vProcess[i]->exec_bin, m_vProcess[i]->exec_arg);
//		}
	}
	
    strncpy(stTraceHist.user_id, stTraceRequest.user_id, sizeof(stTraceRequest.user_id));
    stTraceHist.protocol = stTraceRequest.protocol;
    stTraceHist.run_mode = stTraceRequest.run_mode;
    stTraceHist.search_mode = stTraceRequest.search_mode;
    strncpy(stTraceHist.start_date, stTraceRequest.start_date, sizeof(stTraceRequest.start_date));
    strncpy(stTraceHist.filename, a_FileName, strlen(a_FileName));

    m_TraceHist.UpdateHist(m_pDB, &stTraceHist);

	return TRM_OK;
}

int CScheduler::SetTraceOffHist(ST_TRACE_RESPONSE *a_traceresponse)
{
    ST_TRACE_HIST    stTraceHist;
    ST_TRACE_RESPONSE stTraceResponse;

    stTraceResponse = *a_traceresponse;

    stTraceHist.oper_no = stTraceResponse.oper_no;
    strncpy(stTraceHist.end_date, stTraceResponse.end_date, sizeof(stTraceResponse.end_date));
    strncpy(stTraceHist.filename, stTraceResponse.filename, sizeof(stTraceResponse.filename));

	if(strcmp(stTraceResponse.status, "start") == 0)
	{
    	stTraceHist.status = 1;
	}
	else if(strcmp(stTraceResponse.status, "process") == 0)
	{
    	stTraceHist.status = 2;

	}
	else if(strcmp(stTraceResponse.status, "finish") == 0)
	{
    	stTraceHist.status = 3;
	}
	
    m_TraceHist.UpdateHist(m_pDB, &stTraceHist);

	return TRM_OK;
}
