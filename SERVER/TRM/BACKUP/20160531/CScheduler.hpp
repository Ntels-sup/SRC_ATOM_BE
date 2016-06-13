#ifndef __CSCHEDULER_HPP__
#define __CSCHEDULER_HPP__

#include "TRM_Define.hpp"
#include <iostream>
#include <vector>
#include "DB.hpp"
#include "CTraceRoute.hpp"
#include "CTraceHist.hpp"
#include "CProcess.hpp"

using namespace std;

class CScheduler
{

private:
	DB * 					m_pDB;

	CTraceRoute	    		m_TraceRoute;	// Trace Route info
	CTraceHist				m_TraceHist;
	CProcess 				m_Process;		

	int 					LoadTraceInfo();
	int 					LoadProcessInfo(int a_routeCnt);
	int						CheckCurrentTime();

    ST_TRACE_ROUTE    		m_stTraceRoute			[TRM_MAX_TRACE_ROUTE];
   	ST_TRACE_HIST     		m_stTracehists			[TRM_MAX_TRACE_HIST];
   	ST_PROCESS	      		m_stProcess				[TRM_MAX_PROCESS];
    ST_COWORK_INFO  		m_stcoworkinfo      	[0];
	ST_TRACE_REQUEST  		m_stTraceRequest	    [TRM_MAX_REQUEST];
	ST_TRACE_RESPONSE 		m_stTraceResponse	    [TRM_MAX_RESPONSE];

    unsigned int       		m_nTraceRouteCount;
    unsigned int       		m_nBatchFlowCount;
    unsigned int       		m_nProcessCount[TRM_MAX_PROCESS];
	unsigned int			m_nMultiJobCount;
	unsigned int			m_nStartFlag;
	unsigned int			m_nDBInitFlag;

	vector<ST_TRACE_ROUTE *> m_vTraceRoute;
	vector<ST_TRACE_HIST *> m_vTraceHist;
	vector<ST_PROCESS *>	m_vProcess;

//	map<string, int> 		m_mapBatchFlow;
	void 					GetRouteDetailInfo			();
    void 					SetNextScheduleMinute       (void) ;
    void 					SetNextScheduleHour         (void) ;
    void 					SetNextScheduleDay          (void) ;
    void 					SetNextScheduleWeek         (void) ;
    void 					SetNextScheduleMonth        (void) ;
    void 					SetNextScheduleMonthLast    (void) ;
	void 					SetTraceResponse			(char * a_strCurrent_date);

	int						ReceiveData(ST_COWORK_INFO a_stcoworkinfo);
	int						CreateTraceFile(ST_TRACE_REQUEST *a_tracerequest);
	int						WriteTraceFile(ST_TRACE_RESPONSE *a_traceresponse);

public:
    CScheduler() ;
    ~CScheduler() ;

    int             		Initial() ;
    int             		Run() ;
	int             		SetTraceHist(char *a_FileName, ST_TRACE_REQUEST *a_tracerequest) ;
	int             		SetTraceOffHist(ST_TRACE_RESPONSE *a_traceresponse) ;
};
#endif

