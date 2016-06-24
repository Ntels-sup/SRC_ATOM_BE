
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

bool CModuleIPC::SendMesg(CProtocol& a_cMesg)
{
	int nDstNodeNo = -1;
	int nDstProcNo = -1;
	a_cMesg.GetDestination(nDstNodeNo, nDstProcNo);
	

	int nToProcNo = -1;
	if (m_clsRoute.Lookup(nDstNodeNo, nDstProcNo, nToProcNo) == false) {
		m_strErrorMsg = "not found routing table";
		return false;
	}

	modipc_t::iterator iter = m_mapIPC.find(nToProcNo);
	if (iter == m_mapIPC.end()) {
		m_strErrorMsg = "not found procno";
		return false;
	}

	pthread_mutex_lock(&iter->second.m_tMutex);

	iter->second.m_deqMsg.push_back(a_cMesg);
	pthread_cond_signal(&iter->second.m_tCond);

	pthread_mutex_unlock(&iter->second.m_tMutex);

	return true;
}

int CModuleIPC::RecvMesg(int a_nMyProcNo, CProtocol& a_cMesg, int a_nWaitTime)
{
	modipc_t::iterator iter = m_mapIPC.find(a_nMyProcNo);
	if (iter == m_mapIPC.end()) {
		m_strErrorMsg = "not found procno";
		return -1;
	}

	int nCnt = 0;
	bool bRetry = false;
	
	pthread_mutex_lock(&iter->second.m_tMutex);
	
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
				struct timeval now;
				gettimeofday(&now, NULL);	
				struct timespec wtime = { now.tv_sec + a_nWaitTime, now.tv_usec * 1000 };

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

int CModuleIPC::FindProcNo(const char* a_szProcName)
{
	if (a_szProcName == NULL) {
		m_strErrorMsg = "invalied proces name";
		return -1;
	}
	
	modipc_t::iterator iter = m_mapIPC.begin();
	for (; iter != m_mapIPC.end(); ++iter) {
		if (iter->second.m_strProcName.compare(a_szProcName) == 0) {
			return iter->first;
		}
	}
	
	return -1; 
}

void CModuleIPC::PrintTable(CFileLog* a_pclsLog, int a_nLogLevel)
{
    a_pclsLog->LogMsg(a_nLogLevel, "Module Table ========================");
    a_pclsLog->LogMsg(a_nLogLevel, "  module name   procno");

    auto miter = m_mapIPC.begin();
    for (; miter != m_mapIPC.end(); ++miter) {
        a_pclsLog->LogMsg(a_nLogLevel, "%10s  %10d",
                        miter->second.m_strProcName.c_str(), miter->first);
    }
    a_pclsLog->LogMsg(a_nLogLevel, " ");

    return;
}


bool CModuleIPC::Regist(const char* a_szProcNo, int a_nProcNo)
{
	if (a_nProcNo < 0) {
		m_strErrorMsg = "invalied proces no";
		return false;
	}
	
	modipc_t::iterator iter = m_mapIPC.find(a_nProcNo);
	if (iter == m_mapIPC.end()) {
		iter = m_mapIPC.insert(m_mapIPC.end(), modipc_t::value_type(a_nProcNo, ST_MSG()));
		iter->second.m_strProcName = a_szProcNo;
		iter->second.m_deqMsg.clear();
		pthread_mutex_init(&iter->second.m_tMutex, NULL);
		pthread_cond_init(&iter->second.m_tCond, NULL);
	}
	
	return true;
}

bool CModuleIPC::UnRegist(int a_nProcNo)
{
	if (a_nProcNo < 0) {
		m_strErrorMsg = "invalied proces no";
		return false;
	}

	modipc_t::iterator iter = m_mapIPC.find(a_nProcNo);
	if (iter != m_mapIPC.end()) {
		pthread_mutex_destroy(&iter->second.m_tMutex);
		pthread_cond_destroy(&iter->second.m_tCond);
		m_mapIPC.erase(iter);
	}

	return true;
}

CModuleIPC* CModuleIPC::m_pcInstance = NULL;
CModuleIPC::CModuleIPC() : m_clsRoute(CMesgRoute::Instance())
{
	return;
}
