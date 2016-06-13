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
 
#include "CMesgRoute.hpp"

using std::pair;


CMesgRoute& CMesgRoute::Instance(void)
{
	if (m_pcInstance == NULL) {
		m_pcInstance = new CMesgRoute();
	}
	return *m_pcInstance;
}

void CMesgRoute::SetLocalNodeNo(int a_nNodeNo)
{
	m_nLocalNodeNo = a_nNodeNo;
	return;
}

void CMesgRoute::SetDefaultProcNo(int a_nProcNo)
{
	m_nDefaultProcNo = a_nProcNo;
	return;	
}

bool CMesgRoute::Lookup(int a_nDstNodeNo, int a_nDstProcNo, int& a_nToProcNo)
{
 	// local node가 아니면 default procid를 반환한다.
	// default gateway 의미와 동일
	if (a_nDstNodeNo != 0) {
 		if (m_nLocalNodeNo != a_nDstNodeNo) {
			 a_nToProcNo = m_nDefaultProcNo;
			 return true;
		}
	}

	pair<int, int> key(0, a_nDstProcNo);
	bool ret = true;
 
	pthread_mutex_lock(&m_tRTlock);

	route_t::iterator iter = m_mapRoute.find(key);
	if (iter != m_mapRoute.end()) {
		a_nToProcNo = iter->second;
	} else {
		ret = false;
	}
	
	pthread_mutex_unlock(&m_tRTlock);
	
	return ret;
}

bool CMesgRoute::AddRoute(int a_nDstProcNo, int a_nToProcNo)
{
	pair<int, int> key(0, a_nDstProcNo);
 
	
	pthread_mutex_lock(&m_tRTlock);

	route_t::iterator iter = m_mapRoute.find(key);
	if (iter == m_mapRoute.end()) {
		m_mapRoute.insert(route_t::value_type(key, a_nToProcNo));
	} else {
		iter->second = a_nToProcNo;
	}
	
	pthread_mutex_unlock(&m_tRTlock);
	
	return true;
}

bool CMesgRoute::DelRoute(int a_nDstProc)
{
	pthread_mutex_lock(&m_tRTlock);

	if (a_nDstProc >= 0) {
		pair<int, int> key(m_nLocalNodeNo, a_nDstProc);
		m_mapRoute.erase(key);
	} else {
		route_t::iterator iter = m_mapRoute.begin();
		for (;iter != m_mapRoute.end();) {
			if (m_nLocalNodeNo == iter->first.first) {
				m_mapRoute.erase(iter++);		// 후위형 증가
			} else {
				++iter;
			}
		}
	}

	pthread_mutex_unlock(&m_tRTlock);

	return true;
}

void CMesgRoute::PrintTable(CFileLog* a_pclsLog, int a_nLogLevel)
{
	a_pclsLog->LogMsg(a_nLogLevel, "Routing Table ========================");
	a_pclsLog->LogMsg(a_nLogLevel, "  dst_node    dst_proc   to_module");

	pthread_mutex_lock(&m_tRTlock);

	route_t::iterator iter = m_mapRoute.begin();
	for (; iter != m_mapRoute.end(); ++iter) {
   		a_pclsLog->LogMsg(a_nLogLevel, "%10d  %10d  %10d",
						iter->first.first, iter->first.second, iter->second);
	}

	pthread_mutex_unlock(&m_tRTlock);

	a_pclsLog->LogMsg(a_nLogLevel, " ");
	a_pclsLog->LogMsg(a_nLogLevel, "  Local Node ID: %d", m_nLocalNodeNo);
	a_pclsLog->LogMsg(a_nLogLevel, "  Default Process ID: %d", m_nDefaultProcNo);
	
	a_pclsLog->LogMsg(a_nLogLevel, " ");

	return;
}

CMesgRoute* CMesgRoute::m_pcInstance = NULL;
CMesgRoute::CMesgRoute()
{
	m_nDefaultProcNo = -1;
	m_nLocalNodeNo = -1;
	
	pthread_mutex_init(&m_tRTlock, NULL);
	
	return;
}

CMesgRoute::~CMesgRoute()
{
	pthread_mutex_destroy(&m_tRTlock);
	return;
}
