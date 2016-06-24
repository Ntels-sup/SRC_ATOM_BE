/* vim:ts=4:sw=4
 */
/**
 * \file	CConfigNa.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include <sys/types.h>
#include <dirent.h>

#include "CommonCode.hpp"
#include "CFileLog.hpp"
#include "CConfig.hpp"
#include "FetchMaria.hpp"

#include "CConfigNa.hpp"

using std::string;
using std::vector;


CConfigNa& CConfigNa::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CConfigNa();
	}

	return *m_pclsInstance;
}

bool CConfigNa::LoadCfgFile(const char* a_szCfgFile)
{
	if (a_szCfgFile != NULL) {
		m_strCfgFile = a_szCfgFile;
	}

	CConfig clsCF;
	if (clsCF.Initialize((char*)m_strCfgFile.c_str()) < 0) {
		return false;
	}

	const char* szV = NULL;

	szV = clsCF.GetGlobalConfigValue("LOG_PATH"); if (szV) m_strLogPath = szV;
	szV = clsCF.GetGlobalConfigValue("ATOM_SERVER_IP"); if (szV) m_strServIp = szV;
	szV = clsCF.GetGlobalConfigValue("ATOM_SERVER_PORT"); if (szV) m_nServPort = atoi(szV);
	szV = clsCF.GetGlobalConfigValue("DB_HOST"); if (szV) m_strDbIp = szV;
	szV = clsCF.GetGlobalConfigValue("DB_PORT"); if (szV) m_nDbPort = atoi(szV);
	szV = clsCF.GetGlobalConfigValue("DB_USER"); if (szV) m_strDbUser = szV;
	szV = clsCF.GetGlobalConfigValue("DB_PASS"); if (szV) m_strDbPasswd = szV;
	szV = clsCF.GetGlobalConfigValue("DB_DATABASE"); if (szV) m_strDbName = szV;

	szV = clsCF.GetConfigValue("NA", "VNF_MODE"); 
	if (szV) {
		if (strcmp(szV, "OFF") == 0 || strcmp(szV, "off") == 0) {
			m_bIsVnfMode = false;
		}
	}
	szV = clsCF.GetConfigValue("NA", "VNF_WAITTIME");	if (szV) m_nVnfWaitTime = atoi(szV);
	szV = clsCF.GetConfigValue("NA", "MODULE_PATH");	if (szV) m_strModPath = szV;
	szV = clsCF.GetConfigValue("NA", "NODE_UUID");		if (szV) m_strUuid = szV;
	szV = clsCF.GetConfigValue("NA", "PKG_NAME");		if (szV) m_strPkgName = szV;
	szV = clsCF.GetConfigValue("NA", "NODE_TYPE");		if (szV) m_strNodeType = szV;
	szV = clsCF.GetConfigValue("NA", "NODE_NAME");		if (szV) m_strNodeName = szV;
	szV = clsCF.GetConfigValue("NA", "NODE_NO");		if (szV) m_nNodeNo = atoi(szV);
	szV = clsCF.GetConfigValue("NA", "PROCESS_NO");		if (szV) m_nProcNo = atoi(szV);
	szV = clsCF.GetConfigValue("NA", "NODE_VERSION");	if (szV) m_strVersion = szV;
	szV = clsCF.GetConfigValue("NA", "PKG_MNG_PATH");	if (szV) m_strPkgMngPath = szV;

	return true;
}

bool CConfigNa::ProbeVnfProperties(void)
{
	FILE* pstFile = fopen("/etc/vnf_env.properties", "r");
	if (pstFile == NULL) {
		return false;
	}
	
	char szBuff[160] = {0x00,};
	int nFound = 0;
	int nRead = 0;
	
	
	while (fgets(szBuff, sizeof(szBuff), pstFile) != NULL) {

		// newline 제거
		nRead = strlen(szBuff);
		if (szBuff[nRead - 1] == '\n') {
			szBuff[nRead - 1] = 0x00;
		}

		if (strncasecmp(szBuff, "ems_ip=", 7) == 0) {
			m_strServIp = szBuff + 7;
			m_strDbIp = szBuff + 7;
			nFound++;
		
		} else if (strncasecmp(szBuff, "vnf_name=", 9) == 0) {
			m_strPkgName = szBuff + 9;
			nFound++;
			
		} else if (strncasecmp(szBuff, "vnfc_type=", 10) == 0) {
			m_strNodeType = szBuff + 10;
			nFound++;

		} else if (strncasecmp(szBuff, "node_ip=", 8) == 0) {
			m_strNodeIp = szBuff + 8;
			nFound++;

		} else if (strncasecmp(szBuff, "uuid=", 5) == 0) {
			m_strUuid = szBuff + 5;
			nFound++;
		}

		
		if (nFound == 5) {
			break;
		}
	}
	fclose(pstFile);

	if (nFound != 3) {
		return false;
	}
	
	CConfig clsCF;
	if (clsCF.Initialize((char*)m_strCfgFile.c_str()) == 0) {
		clsCF.UpdateConfigValue("GLOBAL", "ATOM_SERVER_IP", m_strDbIp.c_str());
		clsCF.UpdateConfigValue("GLOBAL", "DB_HOST", m_strDbIp.c_str());
		clsCF.UpdateConfigValue("NA", "PKG_NAME", m_strPkgName.c_str());
		clsCF.UpdateConfigValue("NA", "NODE_TYPE", m_strNodeType.c_str());
		clsCF.UpdateConfigValue("NA", "NODE_UUID", m_strNodeType.c_str());
		clsCF.ReWriteConfig();
	}
	
	return true;
}

bool CConfigNa::DBSetup(void)
{
	if (m_pclsDB != NULL) {
		return true;
	}

	m_pclsDB = new MariaDB;
	
	m_pclsDB->UsedThread();	
	int ret = m_pclsDB->Connect(m_strDbIp.c_str(), m_nDbPort, m_strDbUser.c_str(), 
			             	    m_strDbPasswd.c_str(), m_strDbName.c_str());

	return (ret >= 0)?true:false;
}

bool CConfigNa::ProbeModule(vector<string>& a_vecLoadModule)
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
		if (strncmp(ent->d_name, "ATOM_NA_", 8) != 0) {
			continue;
		}
		if (strcmp(ent->d_name+(len-3), ".so") != 0) {
			continue;
		}

		// NA 시작 시 옵션으로 특정 모듈을 지정할 경우 해당 모듈만 load
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

int CConfigNa::ProbeNodeNo(const char* a_szPkgName, const char* a_szNodeType)
{
	if (m_pclsDB == NULL) {
		return -1;
	}

	int nodeid = 0;
	char szSql[360];
	snprintf(szSql, sizeof(szSql), 
						"SELECT node_no FROM TAT_NODE "
						"WHERE pkg_name = '%s' AND node_type = '%s' and internal_yn = 'Y' ",
							a_szPkgName, a_szNodeType);
	FetchMaria 	fdata;
	int cnt = m_pclsDB->Query(&fdata, szSql, strlen(szSql));
	if (cnt == 1) {

		char szNodeNo[11];
		memset(szNodeNo, 0x00, sizeof(szNodeNo));
		
		fdata.Set(szNodeNo, sizeof(szNodeNo));
		if (fdata.Fetch() == false) {
			return -1;
		}
		
		nodeid = atoi(szNodeNo);
		
	} else if (cnt == 0) {
		nodeid = 0;
	} else {
		nodeid = -1;
	}

    return nodeid;
}

int CConfigNa::ProbeProcNo(const char* a_szPkgName, const char* a_szNodeType,
													const char* a_szProcName)
{
	if (m_pclsDB == NULL) {
		return -1;
	}

	int ret = 0;
	char szSql[360];
	snprintf(szSql, sizeof(szSql)-1, 
				"SELECT proc_no FROM TAT_PROCESS "
				"WHERE pkg_name = '%s' AND node_type = '%s' AND proc_name = '%s'",
				a_szPkgName, a_szNodeType, a_szProcName);

	FetchMaria 		fdata;
	int cnt = m_pclsDB->Query(&fdata, szSql, strlen(szSql));
	if (cnt == 1) {
		char szNodeNo[11];
		memset(szNodeNo, 0x00, sizeof(szNodeNo));
		
		fdata.Set(szNodeNo, sizeof(szNodeNo));
		if (fdata.Fetch() == false) {
			return -1;
		}
		
		ret = atoi(szNodeNo);
	} else if (cnt <= 0) {
		ret = 0;
	} else {
		ret = -1;
	}

    return ret;
}

void CConfigNa::ConfigPrint(CFileLog* a_pclsLog)
{
	a_pclsLog->INFO("CFG, Configuration =================================");
	a_pclsLog->INFO("    Log Path:     %s", m_strLogPath.c_str());
	a_pclsLog->INFO("    Module Path:  %s", m_strModPath.c_str());
	a_pclsLog->INFO("    Server IP:    %s", m_strServIp.c_str());
	a_pclsLog->INFO("    Server Port:  %d", m_nServPort);
	a_pclsLog->INFO("    UUID          %s", m_strUuid.c_str());
	a_pclsLog->INFO("    PKG Name:     %s", m_strPkgName.c_str());
	a_pclsLog->INFO("    Node Type:    %s", m_strNodeType.c_str());
	a_pclsLog->INFO("    Process Name: %s", m_strProcName.c_str());
	a_pclsLog->INFO("    Node IP:      %s", m_strNodeIp.c_str());
	a_pclsLog->INFO("    Node ID:      %d", m_nNodeNo);
	a_pclsLog->INFO("    Process ID:   %d", m_nProcNo);

	a_pclsLog->INFO("    DB IP:        %s", m_strDbIp.c_str());
	a_pclsLog->INFO("    DB Port:      %d", m_nDbPort);
	//std::string m_strDbUser;
	//std::string m_strDbPasswd;
	a_pclsLog->INFO("    DB Name:      %s", m_strDbName.c_str());
	a_pclsLog->INFO(" ");

	return;
}

CConfigNa* CConfigNa::m_pclsInstance = NULL;
CConfigNa::CConfigNa()
{
	// NA default config ????
	m_strCfgFile = getenv("HOME");
	m_strCfgFile += "/ATOM/CFG/ATOM.cfg";
			
	m_bIsVnfMode = true;
	m_nVnfWaitTime = 0;					// 0?? block mode
	
    m_strLogPath = getenv("HOME");
	m_strLogPath += "/ATOM/LOG";

	m_strModPath = getenv("HOME");
	m_strModPath += "/ATOM/BIN/libexec";

	m_strServIp = "127.0.0.1";
	m_nServPort = 8000;

	m_strPkgName = "";
	m_strNodeType = "";
	m_strProcName = "NA";
	m_strNodeIp = "127.0.0.1";
	//m_strUuid = "";
	m_strVersion = "1.0.0";

	m_nNodeNo = 0;
	m_nProcNo = 0;
	
	m_strDbIp = "127.0.0.1";
	m_nDbPort = 3306;
	m_strDbUser = "atom";
	m_strDbPasswd = "atom";
	m_strDbName = "ATOM";

	m_strPkgMngPath = getenv("HOME");
	m_strPkgMngPath += "/ATOM/PKG";
	
	m_pclsDB = NULL;

	return;
}
CConfigNa::~CConfigNa()
{
	if (m_pclsDB != NULL) {
		delete m_pclsDB;
		m_pclsDB = NULL;
	}
}
