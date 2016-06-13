#ifndef __CCONSOLE_HPP__
#define __CCONSOLE_HPP__

#include "MariaDB.hpp"
#include "CCli.hpp"
#include "CUser.hpp"
#include "CThreadQueue.hpp"
#include "CConsoleSession.hpp"

class CConsole{
	private:
		CCli *m_cCli;
		DB *m_cDb;
		unsigned int m_nSessionSeq;
		string m_strUserId;
		CThreadQueue<CConsoleSession*> *m_cRcvQueue;
		CThreadQueue<CConsoleSession*> *m_cSndQueue;
		int m_nLoginRetryCount;
		string m_strDBName;
		string m_strDBIp;
		int m_nDBPort;
		string m_strDBUser;
		string m_strDBPassword;
		string m_strCmdDumpFile;
		string m_strUserDumpFile;

		list<CUser*> lstUser;

		int LoadAllUserFromDb();
		int LoadAllUserFromCfg(string &a_strPath);
		int DumpAllUserToCfg();
		int CheckUser(const char *a_szUserId, const char *a_szPasswd);
		int DropAllUser();
	public:
		CConsole(CThreadQueue<CConsoleSession*> *a_cRcvQueue, CThreadQueue<CConsoleSession*> *a_cSndQueue);
		~CConsole();
		void SetLoginRetryCount(int a_nRetryCount) { m_nLoginRetryCount = a_nRetryCount; };
		int InitDBCfg(string a_strDBName, string a_strDBIp, int a_nDBPort, string a_strDBUser, string a_strDBPassword);
		int InitPathCfg(string &a_strLocalPath, string &a_strUsrDumpFile, string &a_strCmdDumpFile);
		int Display(CConsoleSession *a_cSession);
		CConsoleSession* MakeSession(unsigned int a_nType);
		int Login(unsigned int a_nRetryCnt);
		int Run();
};

#endif
