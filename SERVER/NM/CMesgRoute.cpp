
/* vim:ts=4:sw=4
 */
/**
 * \file	CMesgRoute.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */
 
#include "CFileLog.hpp"

#include "CMesgRoute.hpp"

using std::pair;
using std::list;


CMesgRoute& CMesgRoute::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CMesgRoute();
	}
	return *m_pclsInstance;
}

bool CMesgRoute::Lookup(int a_nDstNode, int a_nDstProc, int& a_nToRoute, bool& a_bIsSock)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
	bool ret = true;
 
	pthread_mutex_lock(&m_tMutex);

	route_t::iterator iter = m_mapRoute.find(key);
	if (iter != m_mapRoute.end()) {
		a_nToRoute = iter->second.m_nToRoute;
		a_bIsSock = iter->second.m_bIsSock;
	} else {
		if (a_nDstNode == 0) {
			// local node
			key.first = m_nLocalNodeNo;
			key.second = a_nDstProc;
		} else {
			// nodeno 만을 대상으로 경로를 찾는다.
			// node 관리하는 agent에 메세지를 전송하면 agent가 procno 별로 라우팅
			key.first = a_nDstNode;
			key.second = 0;
		}
		
		iter = m_mapRoute.find(key);
		if (iter != m_mapRoute.end()) {
			a_nToRoute = iter->second.m_nToRoute;
			a_bIsSock = iter->second.m_bIsSock;
		} else {
			ret = false;
		}
	}
	
	pthread_mutex_unlock(&m_tMutex);
	
	return ret;
}

bool CMesgRoute::AddRouteSock(int a_nDstNode, int a_nDstProc, int a_nToRoute)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
	bool ret = true; 
	
	pthread_mutex_lock(&m_tMutex);

	route_t::iterator iter = m_mapRoute.find(key);
	if (iter == m_mapRoute.end()) {
		ST_ROUTE stRoute = {a_nToRoute, true};
		auto iter = m_mapRoute.insert(route_t::value_type(key, stRoute));
		if (iter.second) {
			// 새롭게 추가된 socket  등록
			m_lstNewSock.push_back(a_nToRoute);
		} else {
			m_strErrorMsg = "map insert failed";
			ret = false;
		}
	} else {
		m_strErrorMsg = "already exist routing table";
		ret = false;
	}

	pthread_mutex_unlock(&m_tMutex);

	return ret;
}

bool CMesgRoute::AddRouteModule(int a_nDstNode, int a_nDstProc, int a_nToRoute)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
	bool ret = true; 
	
	pthread_mutex_lock(&m_tMutex);

	route_t::iterator iter = m_mapRoute.find(key);
	if (iter == m_mapRoute.end()) {
		ST_ROUTE stRoute = {a_nToRoute, false};
		auto iter = m_mapRoute.insert(route_t::value_type(key, stRoute));
		if (iter.second == false) {
			m_strErrorMsg = "map insert failed";
			ret = false;
		}
	} else {
		m_strErrorMsg = "already exist routing table";
		ret = false;
	}

	pthread_mutex_unlock(&m_tMutex);

	return ret;
}

bool CMesgRoute::DelRouteSock(int a_nToRoute)
{
	if (a_nToRoute < 0) {
		return false;
	}
	
	pthread_mutex_lock(&m_tMutex);

	route_t::iterator iter = m_mapRoute.begin();
	for (;iter != m_mapRoute.end();) {
		if (iter->second.m_nToRoute == a_nToRoute && iter->second.m_bIsSock) {
			m_mapRoute.erase(iter++);		// 후위형 증가
		} else {
			++iter;
		}
	}
	
	m_lstNewSock.remove(a_nToRoute);

	pthread_mutex_unlock(&m_tMutex);

	return true;
}

bool CMesgRoute::DelRouteModule(int a_nToRoute)
{
	if (a_nToRoute < 0) {
		return false;
	}
	
	pthread_mutex_lock(&m_tMutex);

	route_t::iterator iter = m_mapRoute.begin();
	for (;iter != m_mapRoute.end();) {
		if (iter->second.m_nToRoute == a_nToRoute && !iter->second.m_bIsSock) {
			m_mapRoute.erase(iter++);		// 후위형 증가
		} else {
			++iter;
		}
	}
	
	pthread_mutex_unlock(&m_tMutex);

	return true;
}

bool CMesgRoute::GetNewRouteSock(list<int>& a_lstSock)
{
	pthread_mutex_lock(&m_tMutex);

	a_lstSock.clear();
	a_lstSock.swap(m_lstNewSock);
	
	pthread_mutex_unlock(&m_tMutex);

	return true;	
}

void CMesgRoute::PrintTable(CFileLog* a_pclsLog, int a_nLogLevel)
{
	a_pclsLog->LogMsg(a_nLogLevel, "Routing Table ===============================");
	a_pclsLog->LogMsg(a_nLogLevel, "  dst_node    dst_proc          if       type");

	pthread_mutex_lock(&m_tMutex);

	route_t::iterator iter = m_mapRoute.begin();
	for (; iter != m_mapRoute.end(); ++iter) {
		a_pclsLog->LogMsg(a_nLogLevel, "%10d  %10d  %10d %10s", 
						iter->first.first, iter->first.second, 
						iter->second.m_nToRoute, 
						(iter->second.m_bIsSock) ? "SOCK" : "MODULE");
	}

	pthread_mutex_unlock(&m_tMutex);

	a_pclsLog->LogMsg(a_nLogLevel, " ");
	a_pclsLog->LogMsg(a_nLogLevel, "  Local Node ID: %d", m_nLocalNodeNo);
	a_pclsLog->LogMsg(a_nLogLevel, " ");

	return;
}

CMesgRoute* CMesgRoute::m_pclsInstance = NULL;
CMesgRoute::CMesgRoute()
{
	m_nLocalNodeNo = -1;
	pthread_mutex_init(&m_tMutex, NULL);
	
	return;
}

CMesgRoute::~CMesgRoute()
{
	pthread_mutex_destroy(&m_tMutex);
	
	return;
}
