#ifndef __CCONSOLETHREAD_HPP__
#define __CCONSOLETHREAD_HPP__

#include "CThread.hpp"
#include "CThreadQueue.hpp"
#include "CConsole.hpp"

class CConsoleThread : public CThread{
	private:
		CConsole *m_cConsole;
	public:
		CConsoleThread(CThreadQueue<CConsoleSession*> *rcvQueue, CThreadQueue<CConsoleSession*> *sndQueue);
		void LoginRetryCntCfg(int a_nRetryCnt);
		int InitDBCfg(string a_strDBName, string a_strDBIp, int a_nDBPort, string a_strDBUser, string a_strDBPassword);
		int InitPathCfg(string a_strLocalPath, string a_strUsrDumpFile, string a_strCmdDumpFile);
		~CConsoleThread();
		void Run();
};

#endif
