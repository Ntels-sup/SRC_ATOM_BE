#include "TRM_Define.hpp"
#include "CTraceRoute.hpp"
#include <cstring>

extern CFileLog* g_pcLog;


CTraceRoute::CTraceRoute()
{
    m_pDB = NULL;
}

CTraceRoute::~CTraceRoute()
{
    if(NULL != m_pDB)
    {
		delete m_pDB;
	    m_pDB = NULL;
	}
}

bool CTraceRoute::Init(DB * a_pDB)
{
    m_pDB = a_pDB;

	return (m_pDB != NULL)?TRM_OK:TRM_NOK;
}

int CTraceRoute::LoadRouteInfo(ST_TRACE_ROUTE *a_traceroute)
{
	g_pcLog->DEBUG("CTraceRoute Init");
    char query[1024];
	int	 nRet = 0;

	char hpkg_name               [TRM_ROUTE_PKG_NAME_SIZE +1];                    
	char hcmd                    [TRM_ROUTE_CMD_SIZE +1];                            
	char hnode_no                [12];                                                                   
	char hnode_name              [TRM_ROUTE_NODE_NAME_SIZE +1];
	char hproc_no                [12];                                                                   
	char hnode_type              [TRM_ROUTE_NODE_TYPE_SIZE +1];

	ST_TRACE_ROUTE				*traceroutePtr;
	ST_TRACE_ROUTE				stTraceRoute;

	stTraceRoute 				= *a_traceroute;
	int							nTraceRouteCount = 0;

	sprintf(query,  "SELECT "
					"PKG_NAME, "
					"CMD, "
					"NODE_NO, "
					"NODE_NAME, "
					"PROC_NO, "
					"NODE_TYPE "
					"FROM TAT_TRC_ROUTE ");
//					"WHERE NODE_NO = %d "
//					"AND PKG_NAME = '%s' "
//					"ORDER BY PKG_NAME", stTraceRoute.node_no, stTraceRoute.pkg_name);

    if(m_pDB == NULL)
    {
        g_pcLog->INFO("DB table is empty");
        return TRM_NOK;
    }

    g_pcLog->DEBUG("Query [%s]", query);

    FetchMaria  f_mysql;

	nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {   
        g_pcLog->INFO("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return TRM_NOK;
    }

    // variable inialize
	memset(hpkg_name , 0, TRM_ROUTE_PKG_NAME_SIZE +1);
	memset(hcmd      , 0, TRM_ROUTE_CMD_SIZE +1);    
	memset(hnode_no  , 0, 12);                       
	memset(hnode_name, 0, TRM_ROUTE_NODE_NAME_SIZE +1);
	memset(hproc_no  , 0, 12);                       
	memset(hnode_type, 0, TRM_ROUTE_NODE_TYPE_SIZE +1); 

	//input local variable
	f_mysql.Set(hpkg_name , sizeof(hpkg_name));
	f_mysql.Set(hcmd      , sizeof(hcmd));      
	f_mysql.Set(hnode_no  , sizeof(hnode_no));  
	f_mysql.Set(hnode_name, sizeof(hnode_name));
	f_mysql.Set(hproc_no  , sizeof(hproc_no));  
	f_mysql.Set(hnode_type, sizeof(hnode_type));

	while(true)
	{

        if(f_mysql.Fetch() == TRM_NOK)
        {
            break;
        }

        // input struct
        traceroutePtr = a_traceroute + nTraceRouteCount;


		strncpy(traceroutePtr->pkg_name, hpkg_name , sizeof(hpkg_name)); 
		strncpy(traceroutePtr->cmd,      hcmd      , sizeof(hcmd));      
		traceroutePtr->node_no = atoi(hnode_no);  
		strncpy(traceroutePtr->node_name, hnode_name, sizeof(hnode_name));
		traceroutePtr->proc_no = atoi(hproc_no);  
		strncpy(traceroutePtr->node_type, hnode_type, sizeof(hnode_type));

        g_pcLog->DEBUG("%s, %s, %d, %s, %d, %s"
								, traceroutePtr->pkg_name
								, traceroutePtr->cmd     
								, traceroutePtr->node_no  
								, traceroutePtr->node_name
								, traceroutePtr->proc_no  
								, traceroutePtr->node_type);


        nTraceRouteCount++;		
	}

    return nTraceRouteCount;

}

int CTraceRoute::InsertRouteInfo(ST_TRACE_ROUTE *a_traceroute)
{
	g_pcLog->DEBUG("Insert TraceRoute Info");

    char            insertsql[1024];
    int             cnt = 0;

	ST_TRACE_ROUTE				stTraceRoute;
	stTraceRoute 				= *a_traceroute;

    sprintf(insertsql, "INSERT TAT_TRC_ROUTE ("
			           "PKG_NAME, "
					   "CMD, "
					   "NODE_NO, "
					   "NODE_NAME, "
					   "PROC_NO, "
					   "NODE_TYPE "
					   "VALUES ('%s', '%s', %d, '%s', %d, '%s')", 
					   stTraceRoute.pkg_name,
					   stTraceRoute.cmd,
					   stTraceRoute.node_no,
					   stTraceRoute.node_name,
					   stTraceRoute.proc_no,
					   stTraceRoute.node_type );

    if((cnt = m_pDB->Execute(insertsql, strlen(insertsql))) <= 0)
    {
        g_pcLog->INFO("Execute Fail [%s] [%d:%s]\n", insertsql, cnt, m_pDB->GetErrorMsg(cnt));
        return TRM_NOK;
    }
    else
        g_pcLog->INFO("INSERT OK [%s]\n\n", insertsql);

    return TRM_OK;

}

int	CTraceRoute::RouteDelete()
{
    g_pcLog->INFO("CTraceRoute Delete");

    return TRM_OK;

}

int	CTraceRoute::RouteInsert()
{
    g_pcLog->INFO("CTraceRoute Insert");

    return TRM_OK;

}

