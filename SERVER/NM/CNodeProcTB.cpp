/* vim:ts=4:sw=4
 */
/**
 * \file	CNodeProcTB.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include "CFileLog.hpp"

#include "CConfigNm.hpp"
#include "CNodeProcTB.hpp"

using std::string;
using std::make_pair;
using std::vector;

//extern global symbol
extern CFileLog* g_pclsLog;


CNodeProcTB& CNodeProcTB::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CNodeProcTB();
	}
	
	return *m_pclsInstance;
}

bool CNodeProcTB::AddNode(int a_nId, ST_INFO& a_stNode)
{
	pthread_mutex_lock(&m_tMutex);
	
	auto iter = m_mapNode.find(a_nId);
	if (iter != m_mapNode.end()) {
		g_pclsLog->ERROR("already exist, add failed NODE");
		pthread_mutex_unlock(&m_tMutex);
		return false;
	}
	m_mapNode.insert(make_pair(a_nId, a_stNode));	

	pthread_mutex_unlock(&m_tMutex);
	
	return true;
}

int CNodeProcTB::GetNode(int a_nId, ST_INFO& a_stNode)
{
	pthread_mutex_lock(&m_tMutex);

	auto iter = m_mapNode.find(a_nId);
	if (iter == m_mapNode.end()) {
		pthread_mutex_unlock(&m_tMutex);
		return 0;
	}
	a_stNode = iter->second;
	
	pthread_mutex_unlock(&m_tMutex);
	
	return 1;
}

int CNodeProcTB::FindNode(string& a_strPkgName, string& a_strNodeType, 
													vector<ST_INFO>& a_vecNode)
{
	pthread_mutex_lock(&m_tMutex);

	for (auto iter = m_mapNode.begin(); iter != m_mapNode.end(); ++iter) {
		if (iter->second.m_strPkgName != a_strPkgName) {
			continue;
		}
		if (iter->second.m_strNodeType != a_strNodeType) {
			continue;
		}
		a_vecNode.push_back(iter->second);
	}
	
	pthread_mutex_unlock(&m_tMutex);
	
	return (int)a_vecNode.size();
}

bool CNodeProcTB::AddAtomProc(int a_nId, ST_INFO& a_stProc)
{
	pthread_mutex_lock(&m_tMutex);
	
	auto iter = m_mapAtom.find(a_nId);
	if (iter != m_mapAtom.end()) {
		g_pclsLog->ERROR("already exist, add failed ATOM process");
		pthread_mutex_unlock(&m_tMutex);
		return false;
	}
	m_mapAtom.insert(make_pair(a_nId, a_stProc));	
	
	pthread_mutex_unlock(&m_tMutex);
	
	return true;
}

int CNodeProcTB::GetAtomProc(int a_nId, ST_INFO& a_stProc)
{
	pthread_mutex_lock(&m_tMutex);
	
	auto iter = m_mapAtom.find(a_nId);
	if (iter == m_mapAtom.end()) {
		pthread_mutex_unlock(&m_tMutex);
		return 0;
	}
	a_stProc = iter->second;	
	
	pthread_mutex_unlock(&m_tMutex);
	
	return 1;
}

int CNodeProcTB::GetAtomProcNo(const char* a_szProcName)
{
	int nProcNo = 0;

	pthread_mutex_lock(&m_tMutex);
	
	auto iter = m_mapAtom.begin();
	for (; iter != m_mapAtom.end(); ++iter) {
		if (iter->second.m_strNodeProcName.compare(a_szProcName) != 0) {
			continue;
		}
		nProcNo = iter->second.m_nNodeProcNo;
		break;
	}
	
	pthread_mutex_unlock(&m_tMutex);
	
	return nProcNo;
}

void CNodeProcTB::Delete(int a_nId)
{
	pthread_mutex_lock(&m_tMutex);
	
	m_mapNode.erase(a_nId);
	m_mapAtom.erase(a_nId);
	
	pthread_mutex_unlock(&m_tMutex);
	
	return;
}

CNodeProcTB* CNodeProcTB::m_pclsInstance = NULL;
CNodeProcTB::CNodeProcTB()
{
	pthread_mutex_init(&m_tMutex, NULL);
	return;
}
