/* vim:ts=4:sw=4
 */
/**
 * \file	CConfigPra.cpp
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
#include "FetchMaria.hpp"
#include "CConfig.hpp"

#include "CConfigPra.hpp"

using std::string;
using std::list;

//extern symbol
extern CFileLog* g_pclsLogPra;


CConfigPra& CConfigPra::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CConfigPra();
	}

	return *m_pclsInstance;
}

bool CConfigPra::LoadCfgFile(const char* a_szCfgFile)
{
	if (a_szCfgFile != NULL) {
		m_strCfgFile = a_szCfgFile;
	} else {
		m_strCfgFile = getenv("HOME");
		m_strCfgFile += "/ATOM/CFG/ATOM.cfg";
	}
	
	CConfig clsCF;
	if (clsCF.Initialize((char*)m_strCfgFile.c_str()) < 0) {
		return false;
	}

	const char* szV = NULL;

	szV = clsCF.GetGlobalConfigValue("LOG_PATH"); if (szV) m_strLogPath = szV;
	//szV = clsCF.GetGlobalConfigValue("DB_HOST"); if (szV) m_strDbIp = szV;
	//szV = clsCF.GetGlobalConfigValue("DB_PORT"); if (szV) m_nDbPort = atoi(szV);
	//szV = clsCF.GetGlobalConfigValue("DB_USER"); if (szV) m_strDbUser = szV;
	//szV = clsCF.GetGlobalConfigValue("DB_PASS"); if (szV) m_strDbPasswd = szV;
	//szV = clsCF.GetGlobalConfigValue("DB_DATABASE"); if (szV) m_strDbName = szV;

	//szV = clsCF.GetConfigValue("PRA", "VNF_WAITTIME");	if (szV) m_nVnfWaitTime = atoi(szV);

	return true;
}

int CConfigPra::LoadProcess(list<CProcessManager::ST_APPINFO>& a_lstProcess)
{
	if (m_pclsDB == NULL) {
		g_pclsLogPra->ERROR("CFG, db instance null");
		return -1;
	}

	int dbret = m_pclsDB->Connect(m_strDbIp.c_str(),
								m_nDbPort, 
								m_strDbUser.c_str(),
								m_strDbPasswd.c_str(), 
								m_strDbName.c_str());
	if (dbret < 0) {
		g_pclsLogPra->ERROR("CFG, db connection failed");
		return -1;
	}

	char szSql[360];
	snprintf(szSql, sizeof(szSql), 
					"SELECT proc_no, proc_name, S.svc_name, "
							"exec_bin, exec_arg, exec_env, exec_yn, "
							"IFNULL(start_order, 0), IFNULL(stop_order, 0), "
							"cpu_bind_core, mmr_bind_node, "
							"IFNULL(norm_stop_cd, 0), IFNULL(resta_cnt, 3), "
							"proc_type, use_yn "
					"FROM TAT_PROCESS as P, TAT_SVC as S "
					"WHERE P.pkg_name = '%s' "
					  "AND P.node_type = '%s' "
					  "AND P.use_yn = 'Y' "
					  "AND P.svc_no = S.svc_no ",
					m_strPkgName.c_str(), m_strNodeType.c_str());

	#ifdef NA_DEBUG
	g_pclsLogPra->DEBUG("%s", szSql);
	#endif

	FetchMaria fdata;

	int cnt = m_pclsDB->Query(&fdata, szSql, strlen(szSql));
	if (cnt > 0) {
		char	szProcNo[11]					= {0x00,};
		char	szProcName[DB_PROC_NAME_SIZE]	= {0x00,};
		char	szSvcName[DB_SVC_NAME_SIZE]		= {0x00,};
		char	szExecBin[512]					= {0x00,};
		char	szExecArg[256]					= {0x00,};
		char	szExecEnv[256]					= {0x00,};
		char	szExecYN[DB_YN_SIZE]			= {0x00,};		
		char	szStartOrder[11]				= {0x00,};
		char	szStopOrder[11]					= {0x00,};
		char	szCpuBind[512]					= {0x00,};
		char	szMemBind[256]					= {0x00,};
		char	szStopCode[11]					= {0x00,};
		char	szRestartCnt[11]				= {0x00,};
		char	szProcType[2]					= {0x00,};
		char	szUseYN[DB_YN_SIZE]				= {0x00,};

		fdata.Set(szProcNo,		sizeof(szProcNo));
		fdata.Set(szProcName,	sizeof(szProcName));
		fdata.Set(szSvcName,	sizeof(szSvcName));
		fdata.Set(szExecBin,	sizeof(szExecBin));
		fdata.Set(szExecArg,	sizeof(szExecArg));
		fdata.Set(szExecEnv,	sizeof(szExecEnv));
		fdata.Set(szExecYN,		sizeof(szExecYN));
		fdata.Set(szStartOrder,	sizeof(szStartOrder));
		fdata.Set(szStopOrder,	sizeof(szStopOrder));
		fdata.Set(szCpuBind,	sizeof(szCpuBind));
		fdata.Set(szMemBind,	sizeof(szMemBind));
		fdata.Set(szStopCode,	sizeof(szStopCode));
		fdata.Set(szRestartCnt,	sizeof(szRestartCnt));
		fdata.Set(szProcType,	sizeof(szProcType));
		fdata.Set(szUseYN,		sizeof(szUseYN));
            
		CProcessManager::ST_APPINFO stAppInfo;

		g_pclsLogPra->INFO("Loading Process list ======================");
		g_pclsLogPra->INFO("procno, procname, svc, bin, sorder, eorder");
		
		for (int i=0; i < cnt; i++) {
			if (fdata.Fetch() == false) {
				g_pclsLogPra->ERROR("CFG, select fatch failed");
				return -1;
			}

			if (strcmp(szProcName, "NA") == 0) {
				// NodeAgent 는 PRA 관리대상이 아니다.
				continue;
			}

			stAppInfo.m_nProcNo = atoi(szProcNo);
			stAppInfo.m_strPkgName = m_strPkgName;
			stAppInfo.m_strProcName = szProcName;
			stAppInfo.m_strNodeType = m_strNodeType;
			stAppInfo.m_strSvcName = szSvcName;
			if (szExecYN[0] == 'Y') {
				stAppInfo.m_bIsExec = true;
			} else {
				stAppInfo.m_bIsExec = false;
			}
			stAppInfo.m_strExeBin = szExecBin;
			stAppInfo.m_strExeArg = szExecArg;
			stAppInfo.m_strExeEnv = szExecEnv;
			stAppInfo.m_nStartOrder = atoi(szStartOrder);
			stAppInfo.m_nStopOrder = atoi(szStopOrder);
			
			stAppInfo.m_strCpuBind = szCpuBind;
			stAppInfo.m_strMmrBind = szMemBind;
			
			stAppInfo.m_nNormalStopCd = atoi(szStopCode);
			stAppInfo.m_nRestartCnt = atoi(szRestartCnt);
			if (szProcType[0] == 'B') {
				stAppInfo.m_bIsBatch = true;
			} else {
				stAppInfo.m_bIsBatch = false;
			}
			if (szUseYN[0] == 'Y') {
				stAppInfo.m_bIsUse = true;
			} else {
				stAppInfo.m_bIsUse = false;
			}
			
			a_lstProcess.push_back(stAppInfo);

			g_pclsLogPra->INFO("%s, %s, %s, %s, %s, %s", 
								szProcNo, szProcName, szSvcName, 
								szExecBin, szStartOrder, szStopOrder);
		} //end for

	} else if (cnt == 0) {
		g_pclsLogPra->INFO("CFG, empty execute process");
		cnt = 0;
	} else {
		g_pclsLogPra->INFO("CFG, nod found date, pkgnm: %s, ntype:%s",
								m_strPkgName.c_str(), m_strNodeType.c_str());
		cnt = -1;
	}

    return cnt;
}

bool CConfigPra::LoadProcessFile(void)
{
	//TODO
	return true;
}

bool CConfigPra::SaveProcessFile(void)
{
	//TODO
	return true;
}

void CConfigPra::ConfigPrint(void)
{
	g_pclsLogPra->INFO("CFG, Configuration =================================");
	g_pclsLogPra->INFO("    Log Path:     %s", m_strLogPath.c_str());
	g_pclsLogPra->INFO("    PKG Name:     %s", m_strPkgName.c_str());
	g_pclsLogPra->INFO("    Node Type:    %s", m_strNodeType.c_str());
	g_pclsLogPra->INFO("    Process Name: %s", m_strProcName.c_str());
	g_pclsLogPra->INFO("    Node ID:      %d", m_nNodeNo);
	g_pclsLogPra->INFO("    Process ID:   %d", m_nProcNo);

	g_pclsLogPra->INFO("    DB IP:        %s", m_strDbIp.c_str());
	g_pclsLogPra->INFO("    DB Port:      %d", m_nDbPort);
	//std::string m_strDbUser;
	//std::string m_strDbPasswd;
	g_pclsLogPra->INFO("    DB Name:      %s", m_strDbName.c_str());

	return;
}

CConfigPra* CConfigPra::m_pclsInstance = NULL;
CConfigPra::CConfigPra()
{
    m_strLogPath = getenv("HOME");
	m_strLogPath += "/ATOM/LOG";

	m_strPkgName = "vOFCS";
	m_strNodeType = "AP";
	m_strProcName = "NA";
	
	m_nNodeNo = 0;
	m_nProcNo = -1;

	m_strDbIp = "127.0.0.1";
	m_nDbPort = 3306;
	m_strDbUser = "atom";
	m_strDbPasswd = "atom";
	m_strDbName = "ATOM";
	
	return;
}
