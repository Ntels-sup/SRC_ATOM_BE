#ifndef __CTRACEROUTE_HPP__
#define __CTARCEROUTE_HPP__
#include "TRM_Define.hpp"
#include <list>
#include <map>


class CTraceRoute
{
public:

    CTraceRoute();
    ~CTraceRoute();

    bool    			Init		(DB * a_pDB);
	int 				LoadRouteInfo (ST_TRACE_ROUTE *a_traceroute);
	int					InsertRouteInfo(ST_TRACE_ROUTE *a_traceroute);
	int					RouteUpdate();
	int					RouteDelete();
	int					RouteInsert();

private:
    DB 						* m_pDB;
};

#endif // __CTRACEROUTE_HPP__
