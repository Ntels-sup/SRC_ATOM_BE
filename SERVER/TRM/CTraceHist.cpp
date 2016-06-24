#include "TRM_Define.hpp"
#include "CTraceHist.hpp"
#include <ctype.h>
#include <cstring>

extern CFileLog* g_pcLog;

CTraceHist::CTraceHist()
{
    m_pDB = NULL;
}

CTraceHist::~CTraceHist()
{
    if(m_pDB != NULL)
    {
		delete m_pDB;
	    m_pDB = NULL;
	}
}

bool CTraceHist::Init(DB * a_pDB)
{
    m_pDB = a_pDB;

	return (m_pDB != NULL)?TRM_OK:TRM_NOK;
}

int	CTraceHist::UpdateHist(DB * a_pDB, ST_TRACE_HIST *a_tracehist)
{
	m_pDB = a_pDB;
    g_pcLog->DEBUG("CTraceHist Update");

    char            updatesql[1024];
    int             iHistCnt = 0;
	ST_TRACE_HIST stTracehist;
	stTracehist = *a_tracehist;

    sprintf(updatesql, "UPDATE TAT_TRC_HIST SET "                 
						"END_DATE = NOW(), "                          
						"CMD = '%s', "                                
						"FILE_NAME = '%s', "                          
						"STATUS = %d "                               
						"WHERE OPER_NO = %ld "
						, stTracehist.cmd, stTracehist.filename, stTracehist.status
						, stTracehist.oper_no);
						
    g_pcLog->DEBUG("UPDATE [%p][%s]", m_pDB, updatesql);

    if((iHistCnt = m_pDB->Execute(updatesql, strlen(updatesql))) <= 0)
    {
        g_pcLog->ERROR("Execute Fail [%s] [%d:%s]", updatesql, iHistCnt, m_pDB->GetErrorMsg(iHistCnt));
        return TRM_NOK;
    }
    else
        g_pcLog->INFO("UPDATE OK [%p][%d] [%s]", m_pDB, iHistCnt, updatesql);

    return TRM_OK;

}

int	CTraceHist::UpdateOffHist(DB * a_pDB, ST_TRACE_HIST *a_tracehist)
{
	m_pDB = a_pDB;
    g_pcLog->DEBUG("CTraceHist Off Update");

    char            updatesql[1024];
    int             iHistCnt = 0;
	ST_TRACE_HIST stTracehist;
	stTracehist = *a_tracehist;

    sprintf(updatesql, "UPDATE TAT_TRC_HIST SET "                 
						"END_DATE = NOW(), "                          
						"FILE_NAME = '%s', "                          
						"STATUS = %d "                               
						"WHERE OPER_NO = %ld "
						, stTracehist.filename, stTracehist.status
						, stTracehist.oper_no);
						
    g_pcLog->DEBUG("UPDATE [%p][%s]", m_pDB, updatesql);

    if((iHistCnt = m_pDB->Execute(updatesql, strlen(updatesql))) <= 0)
    {
        g_pcLog->ERROR("Execute Fail [%s] [%d:%s]", updatesql, iHistCnt, m_pDB->GetErrorMsg(iHistCnt));
        return TRM_NOK;
    }
    else
        g_pcLog->INFO("UPDATE OK [%p][%d] [%s]", m_pDB, iHistCnt, updatesql);

    return TRM_OK;
}

int	CTraceHist::InsertHist(DB * a_pDB, ST_TRACE_HIST *a_tracehist)
{
	m_pDB = a_pDB;
    g_pcLog->DEBUG("CTraceHist Insert");

    char            insertsql[2048];
    int             iHistCnt = 0;
	ST_TRACE_HIST stTracehist;
	stTracehist = *a_tracehist;

    sprintf(insertsql, "INSERT TAT_TRC_HIST "
					   "(OPER_NO, PACKAGE_NAME, NODE_NAME, USER_ID, PROTOCOL, RUN_MODE, SEARCH_MODE, START_DATE, CMD, FILE_NAME, STATUS) "
					   "VALUES (%ld, '%s', '%s', '%s', %d, %d, %d, '%s', '%s', '%s', '%d')", 
						stTracehist.oper_no, 
						stTracehist.pkg_name, 
						stTracehist.node_name, 
						stTracehist.user_id, 
						stTracehist.protocol, 
						stTracehist.run_mode, 
						stTracehist.search_mode, 
						stTracehist.start_date, 
						stTracehist.cmd, 
						stTracehist.filename,
						stTracehist.status);

    g_pcLog->DEBUG("INSERT [%p][%s]", m_pDB, insertsql);

    if((iHistCnt = m_pDB->Execute(insertsql, strlen(insertsql))) <= 0)
    {
        g_pcLog->ERROR("Execute Fail [%s] [%d:%s]\n", insertsql, iHistCnt, m_pDB->GetErrorMsg(iHistCnt));
        return TRM_NOK;
    }
    else
        g_pcLog->INFO("INSERT OK [%p][%s]\n\n", m_pDB, insertsql);

    return TRM_OK;

}

int CTraceHist::LoadHistInfo(ST_TRACE_HIST *a_tracehist)
{
	g_pcLog->DEBUG("CTraceHist Init");
    char query[1024];
	int	 ntraceHistCount = 0;
	int  nRet = 0;

    char hoper_no                [12] ;
    char hpkg_name               [TRM_HIST_PKG_NAME_SIZE +1];
    char hnode_name              [TRM_HIST_NODE_NAME_SIZE +1];
    char huser_id                [TRM_HIST_USER_ID_SIZE +1];
    char hprotocol               [TRM_HIST_PROTOCOL_SIZE +1];
    char hrun_mode               [2];
    char hsearch_mode            [2];
    char hstart_date             [TRM_HIST_START_DATE_SIZE  +1];
    char hend_date               [TRM_HIST_END_DATE_SIZE +1];
    char hcmd                    [TRM_HIST_CMD_SIZE +1];
    char hfilename               [TRM_HIST_FILENAME_SiZE +1];
    char hstatus                 [TRM_HIST_STATUS_SIZE +1];

	ST_TRACE_HIST				*tracehistPtr;

	sprintf(query, "SELECT "
				   "OPER_NO, "
			       "PACKAGE_NAME, "
       			   "NODE_NAME, "
			       "USER_ID, "
			       "PROTOCOL, "
			       "RUN_MODE, "
			       "SEARCH_MODE, "
				   "START_DATE, "
			       "END_DATE, "
			       "CMD, "
			       "FILE_NAME, "
			       "STATUS "
				   "FROM TAT_TRC_HIST"
				   "ORDER BY OPER_NO");

    if(m_pDB == NULL)
    {
        g_pcLog->WARNING("DB table is empty");
        return TRM_NOK;
    }

    FetchMaria  f_mysql;
	nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {   
        g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return TRM_NOK;
    }

    // variable inialize
    memset(hoper_no, 0, 12);
	memset(hpkg_name, 0, TRM_HIST_PKG_NAME_SIZE +1);
	memset(hnode_name, 0, TRM_HIST_NODE_NAME_SIZE +1);
	memset(huser_id, 0, TRM_HIST_USER_ID_SIZE +1);
	memset(hprotocol, 0, TRM_HIST_PROTOCOL_SIZE +1);
	memset(hrun_mode, 0, 2);                       
	memset(hsearch_mode, 0, 2);                       
	memset(hstart_date, 0, TRM_HIST_START_DATE_SIZE +1);
	memset(hend_date, 0, TRM_HIST_END_DATE_SIZE +1);
	memset(hcmd, 0, TRM_HIST_CMD_SIZE +1);
	memset(hfilename, 0, TRM_HIST_FILENAME_SiZE +1);
	memset(hstatus, 0, TRM_HIST_STATUS_SIZE +1);

    //input local variable
	f_mysql.Set(hoper_no,    sizeof(hoper_no));
	f_mysql.Set(hpkg_name,   sizeof(hpkg_name));
	f_mysql.Set(hnode_name,  sizeof(hnode_name));
	f_mysql.Set(huser_id,    sizeof(huser_id));
	f_mysql.Set(hprotocol,   sizeof(hprotocol));
	f_mysql.Set(hrun_mode,   sizeof(hrun_mode));
	f_mysql.Set(hsearch_mode,sizeof(hsearch_mode));
	f_mysql.Set(hstart_date, sizeof(hstart_date));
	f_mysql.Set(hend_date,   sizeof(hend_date));
	f_mysql.Set(hcmd,        sizeof(hcmd));
	f_mysql.Set(hfilename,   sizeof(hfilename));
	f_mysql.Set(hstatus,     sizeof(hstatus));

	while(true)
	{
        if(f_mysql.Fetch() == TRM_NOK)
        {
            break;
        }

        // input struct
        tracehistPtr = a_tracehist + ntraceHistCount;

		tracehistPtr->oper_no = atoi(hoper_no);
		strncpy(tracehistPtr->pkg_name,   hpkg_name   , sizeof(hpkg_name   ));
		strncpy(tracehistPtr->node_name,  hnode_name  , sizeof(hnode_name  ));
		strncpy(tracehistPtr->user_id,    huser_id    , sizeof(huser_id    ));
		tracehistPtr->protocol = (int)atoi(hprotocol);
		tracehistPtr->run_mode = atoi(hrun_mode);
		tracehistPtr->search_mode = atoi(hsearch_mode);
		strncpy(tracehistPtr->start_date, hstart_date , sizeof(hstart_date ));
		strncpy(tracehistPtr->end_date,   hend_date   , sizeof(hend_date   ));
		strncpy(tracehistPtr->cmd,        hcmd        , sizeof(hcmd        ));
		strncpy(tracehistPtr->filename,   hfilename   , sizeof(hfilename   ));
		tracehistPtr->status = (int)atoi(hstatus);

        g_pcLog->DEBUG("%d, %s, %s, %s, %d, %d, %d, %s, %s, %s, %s, %d", 
										tracehistPtr->oper_no,
										tracehistPtr->pkg_name,  
										tracehistPtr->node_name, 
										tracehistPtr->user_id,   
										tracehistPtr->protocol, 
										tracehistPtr->run_mode,
										tracehistPtr->search_mode,
										tracehistPtr->start_date,
										tracehistPtr->end_date,  
										tracehistPtr->cmd,       
										tracehistPtr->filename,  
										tracehistPtr->status);

        ntraceHistCount++;
	}

    return TRM_OK;
}
