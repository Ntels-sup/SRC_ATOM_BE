#ifndef __CMAIN_HPP__
#define __CMAIN_HPP__

#include <list>

#include "CConfig.hpp"
#include "CThreadQueue.hpp"
#include "CConsoleSession.hpp"

class CMain{
	private:
		unsigned int m_nMessageTimeOut;
		bool m_blnLoginSuccessFlg;
		CConfig m_cConfig;
		CThreadQueue<CConsoleSession*> *m_cConsoleRcvQueue;
		CThreadQueue<CConsoleSession*> *m_cConsoleSndQueue;
		list<CConsoleSession*> m_lstConsoleSession;
	public:
		CMain();
		~CMain();
		int Init();
		CConsoleSession* FindConsoleSession(unsigned int nSessionId);
		int CmdResponseProcess(CCliRsp &a_cDecRsp);
		int SessionTimeOutHandler(time_t curTime);
		int ConsoleHandler();
		int ExternalHandler();
		int Run();
};

#endif
