#ifndef __CNMSession_hpp__
#define __CNMSession_hpp__

#include <vector>
#include <iostream>
#include <cstring>
#include "TRM_Define.hpp"
#include "CTraceHist.hpp"
#include "CMesgExchSocket.hpp"
#include "rabbit.hpp"

using namespace std;

class CNMSession
{
private:
	DB *            	m_pDB;
    CTraceHist      	m_TraceHist;
	rabbit::object  	m_root;
	string 		        m_strtraceresponse;
	string      		m_strtracerequest;

	int 				m_nStartFlag;

	int 				SetRequestMsg(ST_TRACE_REQUEST *a_batchrequest);
#ifdef TRM_DEBUG	
	int					CheckRecvMsg();
#endif
	int					ProcessCtrl();

public:
    CNMSession() ;
    ~CNMSession() ;

	CMesgExchSocket 	m_sock;
	ST_TRACE_REQUEST	m_stTraceRequest;
	ST_TRACE_RESPONSE	m_stTraceResponse;

    int                 Initial();
	int					Regist(ST_COWORK_INFO *a_coworkinfo);
    int                 Final  ();
    int                 Run    ();
	int					RecvMsg();
	int					IsRequest();
    char*               space_remove(char *a_str);

	int 				GetRequestMsg(int nOnOff, ST_TRACE_REQUEST *a_tracerequest);
	int 				GetResponseMsg(ST_TRACE_RESPONSE *a_traceresponse);
	int 				GetTraceData(ST_TRACE_RESPONSE *a_traceresponse);
	int					GetRegistRouteInfo(ST_TRACE_ROUTE *a_traceroute);
#ifdef TRM_DEBUG	
	int 				TestWebGetTraceData(double a_oper_no, char* start_date, char* end_date, ST_TRACE_RESPONSE *a_traceresponse);
#endif

	int					SendInitResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_batchrequest);
	int					SendWSMResponseMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_RESPONSE *a_traceresponse, int a_nOffData, int a_nRet);
    int                 SendWSMResponseMg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest, int a_nOnOffData, int a_nRet);
	int					SendResponseMsg(DB * a_pDB, ST_COWORK_INFO *a_coworkinfo, ST_TRACE_RESPONSE *a_traceresponse);
	int					SendRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_batchrequest);

};

#endif

