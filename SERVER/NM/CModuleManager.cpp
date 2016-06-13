/* vim:ts=4:sw=4
 */
/**
 * \file	CModuleManager.cpp
 * \brief	동적 모듈 관리
 *			
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include "CConfigNm.hpp"
#include "CModuleManager.hpp"

using std::string;
using std::pair;

extern CFileLog* g_pclsLog;


CModuleManager& CModuleManager::Instance(void)
{
	if (m_pclsInstance == NULL)
	{
		m_pclsInstance = new CModuleManager();
	}

	return *m_pclsInstance;
}

bool CModuleManager::AddModule(string& a_strModPath)
{
	CConfigNm& clsCfg = CConfigNm::Instance();

	CModule* pclsMod = NULL;
	string strProcName;
	int nNodeNo = -1;
	int nProcNo = -1;
	pair<string, CModule*> parMod;


	g_pclsLog->INFO("NA, module loading, %s", a_strModPath.c_str());

	pclsMod = new CModule();
	if (pclsMod->Loading(a_strModPath) == false) {
		g_pclsLog->ERROR("NA, module loading failed");
		delete pclsMod;
		return false;
	}

	strProcName = pclsMod->GetModuleName();
	CAddress::Instance().LookupAtom(strProcName.c_str(), nNodeNo, nProcNo);
	if (nProcNo <= 0) {
		g_pclsLog->ERROR("MODM, not found process id, procname: %s, nodeno: %d, procno: %d",
												 strProcName.c_str(), nNodeNo, nProcNo);
		delete pclsMod;
		return false;
	}

	// module options (argument)
	strncpy(pclsMod->m_stOption.m_szPkgName, clsCfg.m_strPkgName.c_str(), 
									sizeof(pclsMod->m_stOption.m_szPkgName)-1);
	strncpy(pclsMod->m_stOption.m_szNodeType, clsCfg.m_strNodeType.c_str(),
									sizeof(pclsMod->m_stOption.m_szNodeType)-1);
	strncpy(pclsMod->m_stOption.m_szProcName, strProcName.c_str(), 
									sizeof(pclsMod->m_stOption.m_szProcName));
	strncpy(pclsMod->m_stOption.m_szNodeName, clsCfg.m_strNodeName.c_str(),
									sizeof(pclsMod->m_stOption.m_szNodeName)-1);
	strncpy(pclsMod->m_stOption.m_szNodeVersion, clsCfg.m_strNodeVersion.c_str(),
									sizeof(pclsMod->m_stOption.m_szNodeVersion)-1);
	pclsMod->m_stOption.m_nNodeNo = nNodeNo;
	pclsMod->m_stOption.m_nProcNo = nProcNo;
	strncpy(pclsMod->m_stOption.m_szCfgFile, clsCfg.m_strCfgFile.c_str(),
									sizeof(pclsMod->m_stOption.m_szCfgFile));
	pclsMod->m_stOption.m_pclsModIpc = &CModuleIPC::Instance();
	pclsMod->m_stOption.m_pclsAddress = &CAddress::Instance();
	pclsMod->m_stOption.m_pclsDB = clsCfg.m_pclsDB;

	// module 초기화
	if (pclsMod->Initalize() == false) {
		g_pclsLog->ERROR("MODM, module initialize failed, %s", a_strModPath.c_str());
		delete pclsMod;
		return false;
	}

	CMesgRoute::Instance().AddRouteModule(clsCfg.m_nNodeNo, nProcNo, nProcNo);
	CModuleIPC::Instance().Regist(strProcName.c_str(), nProcNo);

	parMod.first = strProcName;
	parMod.second = pclsMod;
	m_mapModule.insert(parMod);

	return true;
}

bool CModuleManager::ActiveModule(const char* a_szModName)
{
	if (a_szModName != NULL) {
		auto iter = m_mapModule.find(a_szModName);
		if (iter == m_mapModule.end()) {
			return false;
		}

		iter->second->Process();

		return true;
	}

	// Module, 초기화  및 스레드 실행
	for (auto iter = m_mapModule.begin(); iter != m_mapModule.end(); ++iter) {
		iter->second->Process();
	}

	return true;
}

bool CModuleManager::StopModule(const char* a_szModName)
{
	if (a_szModName != NULL) {
		auto iter = m_mapModule.find(a_szModName);
		if (iter == m_mapModule.end()) {
			return false;
		}	

		if (iter->second->IsThreadRunning()) {
			//TODO stop command 전송
		}
		iter->second->UnLoading();

		return true;
	}

	for (auto iter = m_mapModule.begin(); iter != m_mapModule.end(); ++iter) {
		if (iter->second->IsThreadRunning()) {
            //TODO stop command 전송
        }
		iter->second->UnLoading();
	}

	return true;
}

bool CModuleManager::FindModule(const char* a_szModName)
{
	if (a_szModName != NULL) {
		return false;
	}

	auto iter = m_mapModule.find(a_szModName);
	if (iter == m_mapModule.end()) {
		return false;
	}
	
	return true;
}

bool CModuleManager::SetOptPkgName(string& a_strPkgName, const char* a_szModName)
{
	if (a_szModName != NULL) {
		auto iter = m_mapModule.find(a_szModName);
		if (iter == m_mapModule.end()) {
			return false;
		}

		strncpy(iter->second->m_stOption.m_szPkgName, a_strPkgName.c_str(),
								sizeof(iter->second->m_stOption.m_szPkgName)-1);
		return true;
	}

	for (auto iter = m_mapModule.begin(); iter != m_mapModule.end(); ++iter) {
		strncpy(iter->second->m_stOption.m_szPkgName, a_strPkgName.c_str(),
								sizeof(iter->second->m_stOption.m_szPkgName)-1);
	}

	return true;
}

bool CModuleManager::SetOptNodeType(string& a_strNodeType, const char* a_szModName)
{
	if (a_szModName != NULL) {
		auto iter = m_mapModule.find(a_szModName);
		if (iter == m_mapModule.end()) {
			return false;
		}

		strncpy(iter->second->m_stOption.m_szNodeType, a_strNodeType.c_str(),
								sizeof(iter->second->m_stOption.m_szNodeType)-1);
		return true;
	}

	for (auto iter = m_mapModule.begin(); iter != m_mapModule.end(); ++iter) {
		strncpy(iter->second->m_stOption.m_szNodeType, a_strNodeType.c_str(),
								sizeof(iter->second->m_stOption.m_szNodeType)-1);
	}

	return true;
}

bool CModuleManager::SetOptNodeName(string& a_strNodeName, const char* a_szModName)
{
	if (a_szModName != NULL) {
		auto iter = m_mapModule.find(a_szModName);
		if (iter == m_mapModule.end()) {
			return false;
		}

		strncpy(iter->second->m_stOption.m_szNodeName, a_strNodeName.c_str(),
								sizeof(iter->second->m_stOption.m_szNodeName)-1);
		return true;
	}

	for (auto iter = m_mapModule.begin(); iter != m_mapModule.end(); ++iter) {
		strncpy(iter->second->m_stOption.m_szNodeName, a_strNodeName.c_str(),
								sizeof(iter->second->m_stOption.m_szNodeName)-1);
	}

	return true;
}

bool CModuleManager::SetOptNodeNo(int a_nNodeNo, const char* a_szModName)
{
	if (a_szModName != NULL) {
		auto iter = m_mapModule.find(a_szModName);
		if (iter == m_mapModule.end()) {
			return false;
		}

		iter->second->m_stOption.m_nNodeNo = a_nNodeNo;
		return true;
	}

	for (auto iter = m_mapModule.begin(); iter != m_mapModule.end(); ++iter) {
		iter->second->m_stOption.m_nNodeNo = a_nNodeNo;
	}

	return true;
}

bool CModuleManager::SetOptProcNo(int a_nProcNo, const char* a_szModName)
{
	if (a_szModName == NULL) {
		return false;
	}

	auto iter = m_mapModule.find(a_szModName);
	if (iter == m_mapModule.end()) {
		return false;
	}
	iter->second->m_stOption.m_nProcNo = a_nProcNo;

	return true;
}

CModuleManager* CModuleManager::m_pclsInstance = NULL;
CModuleManager::CModuleManager()
{
	//NoneOP
	return;
}

CModuleManager::~CModuleManager()
{
	//NoneOP
	return;
}
