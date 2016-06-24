#ifndef __DEF_RSA_ROOT__
#define __DEF_RSA_ROOT__ 1

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>
#include <dlfcn.h>
#include <vector>

#include "CommonCode.hpp"
#include "CommandFormat.hpp"
#include "CFileLog.hpp"
#include "CConfig.hpp"

//Include For Db Connection
#include "MariaDB.hpp"
#include "FetchMaria.hpp"

#include "RSAGlobal.hpp"

#include "RSABase.hpp"

#include "CMesgExchSocketServer.hpp"


class RSARoot
{
	private:
		CFileLog *m_pclsLog;
		CConfig *m_pclsConfig;
		RESOURCE *m_pstRsc;
		int	m_nGroupCount;	
		int m_nNodeID;
		CMesgExchSocketServer *m_pclsSock;
		map<string, RESOURCE*> m_mapRsc;
		char m_pszDBInfo[MAX_DB_CONN_INFO_IDX][DEF_MEM_BUF_64];
		DB *m_pclsDB;

		char m_szPkgName[DEF_MEM_BUF_64];
		char m_szNodeType[DEF_MEM_BUF_64];

        CSocket *m_pclsClient;
	public:
		RSARoot(CFileLog *a_pclsLog, CConfig *a_pclsConfig);
		~RSARoot();

		int Initialize();
		int LoadConfig();
		int InitSharedLibrary();
		int SendResponseToRSA(CSocket *a_pclsClient, int a_nCmd, const char *a_szBody);
        int CheckTrapMsg(std::string &a_strJson);
		int RecvMesgFromRSA(CSocket *a_pclsClient);
		int ProcessInitMsg(string &a_strBuff);
		int ProcessRSAMsg(string &a_strBuff);
		int ProcessStatMsg(string &a_strBuff);
		int Run();
		char *GetDBInfo(int a_nIdx) { return m_pszDBInfo[a_nIdx]; }
		int GetNodeID() { return m_nNodeID; }
        CConfig *GetConfig() { return m_pclsConfig; }
		DB *GetDBConn() { return m_pclsDB; }

		char *GetNodeType() { return m_szNodeType; }
		char *GetPkgName() { return m_szPkgName; }

};



#endif
