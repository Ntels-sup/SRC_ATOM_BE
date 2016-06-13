/* vim:ts=4:sw=4
 */
/**
 * \file	CModuleIPC.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include "CModuleIPC.hpp"

using std::map;
using std::pair;
using std::deque;


CModuleIPC& CModuleIPC::Instance(void)
{
	if (m_pcInstance == NULL) {
		m_pcInstance = new CModuleIPC();
	}
	return *m_pcInstance;
}

bool CModuleIPC::SendMesg(EN_THREAD a_enTo, CProtocol& a_cMesg)
{
	thipc_t::iterator iter = m_mapIPC.find(a_enTo);
	if (iter == m_mapIPC.end()) {
		m_strErrorMsg = "unknown target thread";
		return false;
	}
	
	pthread_mutex_lock(&iter->second.m_tMutex);

	iter->second.m_deqMsg.push_back(a_cMesg);
	pthread_cond_signal(&iter->second.m_tCond);

	pthread_mutex_unlock(&iter->second.m_tMutex);
	
	return true;
}

int CModuleIPC::RecvMesg(EN_THREAD a_enTo, CProtocol& a_cMesg, int a_nWaitTime)
{
	thipc_t::iterator iter = m_mapIPC.find(a_enTo);
	if (iter == m_mapIPC.end()) {
		m_strErrorMsg = "unknown target thread";
		return -1;
	}
	
	pthread_mutex_lock(&iter->second.m_tMutex);

	int nCnt = 0;
	bool bRetry = false;
	
	do {
		if (!iter->second.m_deqMsg.empty()) {
			a_cMesg = *iter->second.m_deqMsg.begin();
			iter->second.m_deqMsg.pop_front();
			nCnt = 1;
			bRetry = false;
		} else {
			if (a_nWaitTime == 0) {
				pthread_cond_wait(&iter->second.m_tCond, &iter->second.m_tMutex);
				bRetry = true;
			} else if (a_nWaitTime > 0) {
				struct timespec wtime = {a_nWaitTime, 0};
				int erno = pthread_cond_timedwait(&iter->second.m_tCond, 
										&iter->second.m_tMutex, &wtime);
				if (erno == 0) {
					bRetry = true;
				} else if (erno == ETIMEDOUT) {
					nCnt = 0;
					bRetry = false;
				} else {
					m_strErrorMsg = "pthread condition failed";
					nCnt = -1;
					bRetry = false;
				}
			}
		}
	} while (bRetry);

	pthread_mutex_unlock(&iter->second.m_tMutex);

	return nCnt;
}

CModuleIPC* CModuleIPC::m_pcInstance = NULL;
CModuleIPC::CModuleIPC()
{
	thipc_t::iterator iter;
	
	iter = m_mapIPC.insert(m_mapIPC.end(), thipc_t::value_type(TH_ROUTING, ST_MSG()));
	pthread_mutex_init(&iter->second.m_tMutex, NULL);
	pthread_cond_init(&iter->second.m_tCond, NULL);

	iter = m_mapIPC.insert(m_mapIPC.end(), thipc_t::value_type(TH_PRA, ST_MSG()));
	pthread_mutex_init(&iter->second.m_tMutex, NULL);
	pthread_cond_init(&iter->second.m_tCond, NULL);

	iter = m_mapIPC.insert(m_mapIPC.end(), thipc_t::value_type(TH_RSA, ST_MSG()));
	pthread_mutex_init(&iter->second.m_tMutex, NULL);
	pthread_cond_init(&iter->second.m_tCond, NULL);

	iter = m_mapIPC.insert(m_mapIPC.end(), thipc_t::value_type(TH_NM, ST_MSG()));
	pthread_mutex_init(&iter->second.m_tMutex, NULL);
	pthread_cond_init(&iter->second.m_tCond, NULL);		
	
	return;
}
