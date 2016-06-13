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

#include "CAddress.hpp"

using std::list;


CAddress& CAddress::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CAddress();
	}
	return *m_pclsInstance;
}

void CAddress::SetDefaultPkg(const char* a_szPkgName)
{
	if (a_szPkgName != NULL) {
		m_strDefPkgName = a_szPkgName;
	}
	return;	
}

void CAddress::SetDefaultNode(const char* a_szNodeType, int a_nNodeNo)
{
	if (a_szNodeType != NULL && a_nNodeNo >= 0) {
		m_strDefNodeTYpe = a_szNodeType;
		m_nDefNodeNo =  a_nNodeNo;
	}
	return;
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
	
	// TODO
	if (strcmp(a_szProcName, "ATOM_NM") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = 8;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NM_PRA") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = PROCID_ATOM_NM_PRA;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NM_RSA") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = PROCID_ATOM_NM_RSA;
		return 1;
	}

	if (strcmp(a_szProcName, "ATOM_ALM") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = 12;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_BJM") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = 10;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_STM") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = 13;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_TRM") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = 11;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_WSM") == 0) {
		a_nNodeNo = 1;
		a_nProcNo = 46;
		return 1;
	}

	if (strcmp(a_szProcName, "ATOM_NA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = 23;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NA_PRA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = PROCID_ATOM_NA_PRA;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NA_RSA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = PROCID_ATOM_NA_RSA;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NA_STA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = PROCID_ATOM_NA_STA;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NA_ALA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = PROCID_ATOM_NA_ALA;
		return 1;
	}
	if (strcmp(a_szProcName, "ATOM_NA_EXA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = PROCID_ATOM_NA_EXA;
		return 1;	
	}
	if (strcmp(a_szProcName, "ATOM_NA_CLA") == 0) {
		a_nNodeNo = m_nDefNodeNo;
		a_nProcNo = PROCID_ATOM_NA_CLA;
		return 1;	
	}

	return 0;
}

int CAddress::LookupAgentApp(const char* a_szProcName, list<ST_ADDR>& a_lstAddr)
{
	//TODO
	return 0;
}

bool CAddress::AddAtom(const char* a_strPkgName, const char* a_szNodeType, 
						const char* a_strProcName, int a_nProcNo)
{
	//TODO
	return false;
}

bool CAddress::AddApp(const char* a_strPkgName, const char* a_szNodeType,
						const char* a_strProcName, int a_nProcNo)
{
	//TODO
	return false;
}

bool CAddress::Remove(const char* a_szPkgName, const char* a_szNodeType, 
						const char* a_szProcName, int a_nProcNo)
{
	//TODO
	return false;
}

CAddress* CAddress::m_pclsInstance = NULL;
CAddress::CAddress()
{
	m_nDefNodeNo = -1;
	pthread_mutex_init(&m_tMutex, NULL);
}
