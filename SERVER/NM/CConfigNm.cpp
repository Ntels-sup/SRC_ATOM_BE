/* vim:ts=4:sw=4
 */
/**
 * \file	CConfigNm.cpp
 * \brief
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include <sys/types.h>
#include <dirent.h>

#include "CConfig.hpp"
#include "MariaDB.hpp"

#include "CConfigNm.hpp"

using std::string;
using std::vector;


//extern global symbol

CConfigNm& CConfigNm::Instance(void)
{
	if (m_pcInstance == NULL) {
		m_pcInstance = new CConfigNm();
	}

	return *m_pcInstance;
}

bool CConfigNm::LoadCfgFile(const char* a_szCfgFile)
{
	if (a_szCfgFile != NULL) {
		m_strCfgFile = a_szCfgFile;
	} else {
		m_strCfgFile = getenv("ATOM_CFG_HOME");
	}
	
	CConfig clsCF;
	if (clsCF.Initialize((char*)m_strCfgFile.c_str()) < 0) {
		return false;
	}
	
	const char* szV = NULL;

	szV = clsCF.GetGlobalConfigValue("LOG_PATH"); if (szV) m_strLogPath = szV;
	szV = clsCF.GetGlobalConfigValue("LISTEN_IP"); if (szV) m_strListenIp = szV;
	szV = clsCF.GetGlobalConfigValue("LISTEN_PORT"); if (szV) m_nListenPort = atoi(szV);
	
	szV = clsCF.GetGlobalConfigValue("DB_HOST"); if (szV) m_strDbIp = szV;
	szV = clsCF.GetGlobalConfigValue("DB_PORT"); if (szV) m_nDbPort = atoi(szV);
	szV = clsCF.GetGlobalConfigValue("DB_USER"); if (szV) m_strDbUser = szV;
	szV = clsCF.GetGlobalConfigValue("DB_PASS"); if (szV) m_strDbPasswd = szV;
	szV = clsCF.GetGlobalConfigValue("DB_DATABASE"); if (szV) m_strDbName = szV;

	szV = clsCF.GetConfigValue("NM", "MODULE_PATH"); if (szV) m_strModPath = szV;
	szV = clsCF.GetConfigValue("NM", "PKG_NAME"); if (szV) m_strPkgName = szV;
	szV = clsCF.GetConfigValue("NM", "NODE_TYPE"); if (szV) m_strNodeType = szV;
	szV = clsCF.GetConfigValue("NM", "NODE_NO"); if (szV) m_nNodeNo = atoi(szV);
	szV = clsCF.GetConfigValue("NM", "PROCESS_NO"); if (szV) m_nProcNo = atoi(szV);

	return true;
}

bool CConfigNm::ProbeModule(vector<string> a_vecLoadModule)
{
	DIR *dir = opendir(m_strModPath.c_str());
	if (dir == NULL) {
		return false;
	}

	int len = 0;
	dirent* ent = NULL;
	
	while (true) {
		ent = readdir(dir);
		if (ent == NULL) {
			break;
		}

		if (strcmp(ent->d_name, ".") == 0) {
			continue;
		}
		if (strcmp(ent->d_name, "..") == 0) {
			continue;
		}
		len = strlen(ent->d_name);
		if (len < 4) {
			continue;
		}
		if (strncmp(ent->d_name, "ATOM_NM_", 8) != 0) {
			continue;
		}
		if (strcmp(ent->d_name+(len-3), ".so") != 0) {
			continue;
		}

		// 특정 모듈만 지정해서 loading 할 경우
		if (!a_vecLoadModule.empty()) {
			vector<string>::iterator iter = a_vecLoadModule.begin();
			for (; iter != a_vecLoadModule.end(); ++iter) {
				if (iter->compare(ent->d_name) == 0) {
					break;
				}
			}
			if (iter == a_vecLoadModule.end()) {
				continue;
			}
		}

		string modpath = m_strModPath;
		modpath += "/";
		modpath += ent->d_name;

		m_vecModule.push_back(modpath);
	}

	closedir(dir);

	return true; 
}

bool CConfigNm::DBSetup(void)
{
	if (m_pclsDB != NULL) {
		return true;
	}

	m_pclsDB = new MariaDB();

	m_pclsDB->UsedThread();
	int ret = m_pclsDB->Connect(m_strDbIp.c_str(), m_nDbPort, m_strDbUser.c_str(),
								m_strDbPasswd.c_str(), m_strDbName.c_str());

	return (ret >= 0) ? true : false;
}

void CConfigNm::ConfigPrint(CFileLog* a_pclsLog)
{
	if (a_pclsLog == NULL) {
		return;
	}

	a_pclsLog->INFO("CFG, Configuration =================================");
	//std::vector<std::string> m_vecModule;"
	a_pclsLog->INFO("    LogPath:      %s", m_strLogPath.c_str());
	a_pclsLog->INFO("    Listen IP:    %s", m_strListenIp.c_str());
	a_pclsLog->INFO("    Listen Port:  %d", m_nListenPort);
	a_pclsLog->INFO("    Module Path:  %s", m_strModPath.c_str());
	a_pclsLog->INFO("    DB IP:        %s", m_strDbIp.c_str());
	a_pclsLog->INFO("    DB Port:      %d", m_nDbPort);
	//std::string m_strDbUser;
	//std::string m_strDbPasswd;
	a_pclsLog->INFO("    DB Name:      %s", m_strDbName.c_str());
	a_pclsLog->INFO("    PKG Name:     %s", m_strPkgName.c_str());
	a_pclsLog->INFO("    Node Type:    %s", m_strNodeType.c_str());
	a_pclsLog->INFO("    Process Name: %s", m_strProcName.c_str());
	a_pclsLog->INFO("    Node ID:      %d", m_nNodeNo);
	a_pclsLog->INFO("    Process ID:   %d", m_nProcNo);
	a_pclsLog->INFO("");

	return;
}

CConfigNm* CConfigNm::m_pcInstance = NULL;
CConfigNm::CConfigNm()
{
	m_strLogPath = getenv("HOME");
	m_strLogPath += "/LOG";
	m_strListenIp = "0.0.0.0";
	m_nListenPort = 8000;

	m_strModPath = getenv("HOME");
	m_strModPath += "/BIN";

	m_bVerbose = false;

	m_strDbIp = "127.0.0.1";
	m_nDbPort = 3306;
	m_strDbUser = "atom";
	m_strDbPasswd = "atom";
	m_strDbName	= "ATOM";

	m_nNodeNo = -1;
	m_nProcNo = -1;
	
	m_strPkgName = "ATOM";
	m_strNodeType = "EMS";
	m_strProcName = "NM";
	m_strNodeName = "ATOM-EMS";
	m_strNodeVersion = "1.0.0.0";

	m_pclsDB = NULL;

	return;
}
