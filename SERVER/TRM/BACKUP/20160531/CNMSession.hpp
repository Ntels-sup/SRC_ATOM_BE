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
	std::string         m_strtraceresponse;
	std::string         m_strtracerequest;

	vector<char>		m_vbatchrequest;

	int 				m_nStartFlag;

	int 				SetRequestMsg(ST_TRACE_REQUEST *batchrequest_);
	int					CheckRecvMsg();
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
	// for test
	int 				TestWebGetTraceData(double a_oper_no, char* start_date, char* end_date, ST_TRACE_RESPONSE *a_traceresponse);

	int					SendInitResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *batchrequest_);
	int					SendWSMResponseMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_RESPONSE *a_traceresponse, int ret);
    int                 SendWSMResponseMg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *a_tracerequest, int ret);
	int					SendResponseMsg(DB * a_pDB, ST_COWORK_INFO *a_coworkinfo, ST_TRACE_RESPONSE *a_traceresponse);
	int					SendRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_TRACE_REQUEST *batchrequest_);

};

#endif

