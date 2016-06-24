/* vim:ts=4:sw=4
 */
/**
 * \file	CAddress.cpp
 * \brief	Node ID, Process ID 정보 조회
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <cstring>

#include "CommonCode.hpp"

#include "CConfigNa.hpp"
#include "CAddress.hpp"

using std::list;


CAddress& CAddress::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CAddress();
	}
	return *m_pclsInstance;
}

bool CAddress::SetLocalPkg(std::string& a_strPkgName, std::string& a_strNodeType)
{
	m_strLocalPkgName = a_strPkgName; 
	m_strLocalNodeType = a_strNodeType;
	return true;
}

bool CAddress::SetLocalNodeNo(int a_nNodeNo)
{
	bool bRet = true;
	pthread_mutex_lock(&m_tMutex);

	m_nLocalNodeNo = a_nNodeNo;
	
	RegistLocal("ATOM_NA", m_nNaProcNo);
	RegistLocal("ATOM_NA_PRA", PROCID_ATOM_NA_PRA);
	RegistLocal("ATOM_NA_RSA", PROCID_ATOM_NA_RSA);
	RegistLocal("ATOM_NA_STA", PROCID_ATOM_NA_STA);
	RegistLocal("ATOM_NA_ALA", PROCID_ATOM_NA_ALA);
	RegistLocal("ATOM_NA_EXA", PROCID_ATOM_NA_EXA);
	RegistLocal("ATOM_NA_CLA", PROCID_ATOM_NA_CLA);

	pthread_mutex_unlock(&m_tMutex);

	return bRet;
}

bool CAddress::SetNaProcNo(int a_nProcNo)
{
	pthread_mutex_lock(&m_tMutex);

	m_nNaProcNo = a_nProcNo;
	RegistLocal("ATOM_NA", a_nProcNo);

	pthread_mutex_unlock(&m_tMutex);

	return true;
}

bool CAddress::SetAtomAddress(void)
{
	CConfigNa& clsCfg = CConfigNa::Instance();

	int nNodeNo = clsCfg.ProbeNodeNo("ATOM", "EMS");
	if (nNodeNo <= 0) return false;

	int nNmProcNo = clsCfg.ProbeProcNo("ATOM", "EMS", "NM");
	if (nNmProcNo <= 0) return false;

	int nAlmProcNo = clsCfg.ProbeProcNo("ATOM", "EMS", "ALM");
	if (nAlmProcNo <= 0) return false;

	int nBjmProcNo = clsCfg.ProbeProcNo("ATOM", "EMS", "BJM");
	if (nBjmProcNo <= 0) return false;

	int nStmProcNo = clsCfg.ProbeProcNo("ATOM", "EMS", "STM");
	if (nStmProcNo <= 0) return false;

	int nTrmProcNo = clsCfg.ProbeProcNo("ATOM", "EMS", "TRM");
	if (nTrmProcNo <= 0) return false;

	int nWsmProcNo = clsCfg.ProbeProcNo("ATOM", "EMS", "WSM");
	if (nWsmProcNo <= 0) return false;

	pthread_mutex_lock(&m_tMutex);

	Regist("ATOM", "EMS", "ATOM_NM",		nNodeNo, nNmProcNo);
	Regist("ATOM", "EMS", "ATOM_NM_PRA",	nNodeNo, PROCID_ATOM_NM_PRA);
	Regist("ATOM", "EMS", "ATOM_NM_RSA",	nNodeNo, PROCID_ATOM_NM_RSA);
	Regist("ATOM", "EMS", "ATOM_ALM",		nNodeNo, nAlmProcNo);
	Regist("ATOM", "EMS", "ATOM_BJM",		nNodeNo, nBjmProcNo);
	Regist("ATOM", "EMS", "ATOM_STM",		nNodeNo, nStmProcNo);
	Regist("ATOM", "EMS", "ATOM_TRM",		nNodeNo, nTrmProcNo);
	Regist("ATOM", "EMS", "ATOM_WSM",		nNodeNo, nWsmProcNo);

	pthread_mutex_unlock(&m_tMutex);

	return true;
}

int CAddress::Lookup(const char* a_szPkgName, const char* a_szNodeType,
					const char* a_szProcName, list<ST_ADDR>& a_lstAddr)
{
	//TODO
	
	return -1;
}

int CAddress::LookupAtom(const char* a_szProcName, int& a_nNodeNo, int& a_nProcNo)
{
	if (a_szProcName == NULL) {
		return 0;
	}

	int nRet = 0;

	pthread_mutex_lock(&m_tMutex);
	
	auto miter = m_mapAddr.find(a_szProcName);
	if (miter != m_mapAddr.end()) {
		a_nNodeNo = miter->second.m_nNodeNo;
		a_nProcNo = miter->second.m_nProcNo;
		nRet = 1;
	}

	pthread_mutex_unlock(&m_tMutex);

	return nRet;
}

bool CAddress::AddApp(const char* a_szPkgName, const char* a_szNodeType,
										const char* a_szProcName, int a_nProcNo)
{
	return Regist(a_szPkgName, a_szNodeType, a_szProcName, m_nLocalNodeNo, a_nProcNo, true);
}

void CAddress::PrintTable(CFileLog* a_pclsLog, int a_nLogLevel)
{
    a_pclsLog->LogMsg(a_nLogLevel, "Address Table ===============================");
    a_pclsLog->LogMsg(a_nLogLevel, "  PkgName  NodeType  ProcName  NodeNo  ProcNo");

    pthread_mutex_lock(&m_tMutex);

    auto miter = m_mapAddr.begin();
    for (; miter != m_mapAddr.end(); ++miter) {
        a_pclsLog->LogMsg(a_nLogLevel, "%10s  %10s  %20s %10d %10d",
                        miter->second.m_strPkgName.c_str(),
						miter->second.m_strNodeType.c_str(),
						miter->second.m_strProcName.c_str(),
						miter->second.m_nNodeNo,
						miter->second.m_nProcNo);
    }

    pthread_mutex_unlock(&m_tMutex);

    return;
}

bool CAddress::Regist(const char* a_szPkgName, const char* a_szNodeType, 
			const char* a_szProcName, int a_nNodeNo, int a_nProcNo, bool a_bIsApp)
{
	auto miter = m_mapAddr.find(a_szProcName);
	if (miter == m_mapAddr.end()) {
		miter = m_mapAddr.insert(m_mapAddr.end(), 
						map<string, ST_ADDR>::value_type(a_szProcName, ST_ADDR()));
	}
	miter->second.m_strPkgName = a_szPkgName;
	miter->second.m_strNodeType = a_szNodeType;
	miter->second.m_strProcName = a_szProcName;
	miter->second.m_nNodeNo = a_nNodeNo;
	miter->second.m_nProcNo = a_nProcNo;
	miter->second.m_bApp = a_bIsApp;

	return true;
}

bool CAddress::RegistLocal(const char* a_szProcName, int a_nProcNo, bool a_bIsApp)
{
	auto miter = m_mapAddr.find(a_szProcName);
	if (miter == m_mapAddr.end()) {
		miter = m_mapAddr.insert(m_mapAddr.end(), 
						map<string, ST_ADDR>::value_type(a_szProcName, ST_ADDR()));
	}
	miter->second.m_strPkgName = m_strLocalPkgName;
	miter->second.m_strNodeType = m_strLocalNodeType;
	miter->second.m_strProcName = a_szProcName;
	miter->second.m_nNodeNo = m_nLocalNodeNo;
	miter->second.m_nProcNo = a_nProcNo;
	miter->second.m_bApp = a_bIsApp;

	return true;
}

CAddress* CAddress::m_pclsInstance = NULL;
CAddress::CAddress()
{
	m_nLocalNodeNo = -1;
	pthread_mutex_init(&m_tMutex, NULL);
}
