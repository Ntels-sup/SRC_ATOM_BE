#include "CGlobal.hpp"
#include "CConsoleThread.hpp"

CConsoleThread::CConsoleThread(CThreadQueue<CConsoleSession*> *rcvQueue, CThreadQueue<CConsoleSession*> *sndQueue)
{
    m_cConsole = new CConsole(rcvQueue, sndQueue);
}

CConsoleThread::~CConsoleThread()
{
    delete m_cConsole;
}

int CConsoleThread::InitDBCfg(string a_strDBName, string a_strDBIp, int a_nDBPort, string a_strDBUser, string a_strDBPassword)
{
	int nRet = 0;

	nRet = m_cConsole->InitDBCfg(a_strDBName, a_strDBIp, a_nDBPort, a_strDBUser, a_strDBPassword);
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"DB Config init failed(nRet=%d)\n",nRet);
	}

	return nRet;
}

void CConsoleThread::LoginRetryCntCfg(int a_nRetryCnt)
{
	m_cConsole->SetLoginRetryCount(a_nRetryCnt);
}

int CConsoleThread::InitPathCfg(string a_strLocalPath, string a_strUsrDumpFile, string a_strCmdDumpFile)
{
	int nRet = 0;

	nRet = m_cConsole->InitPathCfg(a_strLocalPath, a_strUsrDumpFile, a_strCmdDumpFile);
	if(nRet != CLC_OK){
		CLC_LOG(CLC_ERR,"DUMP Path init failed(nRet=%d)\n",nRet);
	}
	return nRet ;
}

void CConsoleThread::Run()
{
    m_cConsole->Run();
}

