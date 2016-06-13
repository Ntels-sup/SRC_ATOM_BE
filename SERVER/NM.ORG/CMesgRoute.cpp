
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
	if (m_pcInstance == NULL) {
		m_pcInstance = new CMesgRoute();
	}
	return *m_pcInstance;
}

bool CMesgRoute::AddRouteSock(int a_nDstNode, int a_nDstProc, int a_nToSock)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
	bool ret = true; 
	
	pthread_mutex_lock(&m_tSockLock);

	rtsock_t::iterator iter = m_mapSockRoute.find(key);
	if (iter == m_mapSockRoute.end()) {
		m_mapSockRoute.insert(rtsock_t::value_type(key, a_nToSock));

		// 새롭게 추가된 socket  등록
		m_lstNewSock.push_back(a_nToSock);
	} else {
		m_strErrorMsg = "already exist routing table";
		ret = false;
	}

	pthread_mutex_unlock(&m_tSockLock);

	return ret;
}

bool CMesgRoute::FindRouteSock(int a_nDstNode, int a_nDstProc, int& a_nToSock)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
	bool ret = true;
 
	pthread_mutex_lock(&m_tSockLock);

	rtsock_t::iterator iter = m_mapSockRoute.find(key);
	if (iter != m_mapSockRoute.end()) {
		a_nToSock = iter->second;
	} else if (m_nLocalNodeNo != a_nDstNode) {
		// 지정 nodeid 만을 대상으로 경로를 찾는다.
		// 지정 node를 관리하는 atom agent 찾는데 사용.  
		key.first = a_nDstNode;
		key.second = 0;
		iter = m_mapSockRoute.find(key);
		if (iter != m_mapSockRoute.end()) {
			a_nToSock = iter->second;
		} else {
			ret = false;
		}
	} else {
		ret = false;
	}
	
	pthread_mutex_unlock(&m_tSockLock);
	
	return ret;
}

bool CMesgRoute::DelRouteSock(int a_nToSock)
{
	if (a_nToSock < 0) {
		return false;
	}
	
	pthread_mutex_lock(&m_tSockLock);

	rtsock_t::iterator iter = m_mapSockRoute.begin();
	for (;iter != m_mapSockRoute.end();) {
		if (iter->second == a_nToSock) {
			m_mapSockRoute.erase(iter++);		// 후위형 증가
		} else {
			++iter;
		}
	}
	
	m_lstNewSock.remove(a_nToSock);

	pthread_mutex_unlock(&m_tSockLock);

	return true;
}

bool CMesgRoute::GetNewRouteSock(list<int>& a_lstSock)
{
	pthread_mutex_lock(&m_tSockLock);

	a_lstSock.clear();
	a_lstSock.swap(m_lstNewSock);
	
	pthread_mutex_unlock(&m_tSockLock);

	return true;	
}

bool CMesgRoute::AddRouteThread(int a_nDstNode, int a_nDstProc,
											CModuleIPC::EN_THREAD a_nDthread)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
 
	
	pthread_mutex_lock(&m_tThreadLock);

	rtthread_t::iterator iter = m_mapThreadRoute.find(key);
	if (iter == m_mapThreadRoute.end()) {
		m_mapThreadRoute.insert(rtthread_t::value_type(key, a_nDthread));
	} else {
		m_strErrorMsg = "duplicate routing table";
		//iter->second = a_nDthread;
	}
	
	pthread_mutex_unlock(&m_tThreadLock);

	return true;
}

bool CMesgRoute::FindRouteThread(int a_nDstNode, int a_nDstProc, 
										CModuleIPC::EN_THREAD& a_nDthread)
{
	pair<int, int> key(a_nDstNode, a_nDstProc);
	bool ret = true;
 
	pthread_mutex_lock(&m_tThreadLock);

	rtthread_t::iterator iter = m_mapThreadRoute.find(key);
	if (iter != m_mapThreadRoute.end()) {
		a_nDthread = iter->second;
	} else {
		ret = false;
	}
	
	pthread_mutex_unlock(&m_tThreadLock);
	
	return ret;
}

bool CMesgRoute::DelRouteThread(CModuleIPC::EN_THREAD a_nDthread)
{
	pthread_mutex_lock(&m_tThreadLock);

	rtthread_t::iterator iter = m_mapThreadRoute.begin();
	for (;iter != m_mapThreadRoute.end();) {
		if (iter->second == a_nDthread) {
			m_mapThreadRoute.erase(iter++);		// 후위형 증가
		} else {
			++iter;
		}
	}

	pthread_mutex_unlock(&m_tThreadLock);

	return true;
}

void CMesgRoute::PrintTable(CFileLog* a_pclsLog, int a_nLogLevel)
{
	a_pclsLog->LogMsg(a_nLogLevel, "Routing Table =====================");
	a_pclsLog->LogMsg(a_nLogLevel, "  dst_node    dst_proc     to_sock");

	pthread_mutex_lock(&m_tSockLock);

	rtsock_t::iterator iter = m_mapSockRoute.begin();
	for (; iter != m_mapSockRoute.end(); ++iter) {
		a_pclsLog->LogMsg(a_nLogLevel, "%10d  %10d  %10d", 
						iter->first.first, iter->first.second, iter->second);
	}

	pthread_mutex_unlock(&m_tSockLock);

	a_pclsLog->LogMsg(a_nLogLevel, " ");
	a_pclsLog->LogMsg(a_nLogLevel, "  dst_node    dst_proc   to_module");

	pthread_mutex_lock(&m_tThreadLock);

    rtthread_t::iterator iter2 = m_mapThreadRoute.begin();
	for (; iter2 != m_mapThreadRoute.end(); ++iter2) {
		a_pclsLog->LogMsg(a_nLogLevel, "%10d  %10d  %10d", 
						iter2->first.first, iter2->first.second, iter2->second);
	}

	pthread_mutex_unlock(&m_tThreadLock);

	a_pclsLog->LogMsg(a_nLogLevel, " ");
	a_pclsLog->LogMsg(a_nLogLevel, "  Local Node ID: %d", m_nLocalNodeNo);
	a_pclsLog->LogMsg(a_nLogLevel, " ");

	return;
}

CMesgRoute* CMesgRoute::m_pcInstance = NULL;
CMesgRoute::CMesgRoute()
{
	m_nLocalNodeNo = -1;
	pthread_mutex_init(&m_tSockLock, NULL);
	pthread_mutex_init(&m_tThreadLock, NULL);
	
	return;
}

CMesgRoute::~CMesgRoute()
{
	pthread_mutex_destroy(&m_tSockLock);
	pthread_mutex_destroy(&m_tThreadLock);
	
	return;
}
