#ifndef __DEF_RSA_MA__
#define __DEF_RSA_MA__ 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>
#include <dlfcn.h>
#include <vector>
#include <list>

#include "CommonCode.hpp"
#include "CommandFormat.hpp"

#include "CModule.hpp"
//#include "CAddress.hpp"
#include "CModuleIPC.hpp"
#include "CMesgExchSocket.hpp"

#include "CFileLog.hpp"
#include "CConfig.hpp"

//Include For Db Connection
#include "MariaDB.hpp"
#include "FetchMaria.hpp"

#include "RSAGlobal.hpp"
#include "RSABase.hpp"
#include "RSAMsg.hpp"

#include "EventAPI.hpp"

#define DEF_MAX_STAT_COLUMN_ID	20
const char *STAT_COLUMN_ID[DEF_MAX_STAT_COLUMN_ID] =
{
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
    "11", "12", "13", "14", "15", "16", "17", "18", "19", "20"
};


class RSAMain
{
	private:
		CFileLog *m_pclsLog;
		CConfig *m_pclsConfig;
        EventAPI *m_pclsEvent;
		RSAMsg *m_pclsMsg;
		RESOURCE *m_pstRsc;
		DB *m_pclsDB;

		CModule::ST_MODULE_OPTIONS* m_pstModuleOption;
		CMesgExchSocket *m_pclsRootSock;

		int	m_nGroupCount;	
		char m_pszDBInfo[MAX_DB_CONN_INFO_IDX][DEF_MEM_BUF_64];

	public:
		RSAMain(CFileLog *a_pclsLog, CConfig *a_pclsConfig);
		~RSAMain();

		int FreeResourceData();
		int Initialize(CModule::ST_MODULE_OPTIONS* a_pstModuleOption);
		int LoadConfig();
		int ConnectRoot();
		int SendResponseToUI(const char* a_szPayload);
		int InitSharedLibrary();
		int MakeFullJson(struct tm* a_pstCur);
		int MakeSummaryJson(struct tm* a_pstCur);
		int MakeStatJson(time_t a_tCur);
		int SendRequestToRoot(char *a_szGroupName);
		int RecvMesgFromRoot();
		int ExecutePlugin(time_t a_tCur);
		int Run();
		CConfig *GetConfig() { return m_pclsConfig; }
		char* GetDBInfo(int a_nIdx) { return m_pszDBInfo[a_nIdx]; }
		int GetNodeID() { return m_pstModuleOption->m_nNodeNo; }
		char* GetNodeName() { return m_pstModuleOption->m_szNodeName; }
		char* GetPkgName() { return m_pstModuleOption->m_szPkgName; }
		char* GetNodeType() { return m_pstModuleOption->m_szNodeType; }
		CAddress* GetCAddress() { return m_pstModuleOption->m_pclsAddress; }
		CModuleIPC* GetModuleIPC() { return m_pstModuleOption->m_pclsModIpc; }
        EventAPI *GetEvent() { return m_pclsEvent; }
        DB *GetDBConn() { return m_pstModuleOption->m_pclsDB; }
};



#endif
