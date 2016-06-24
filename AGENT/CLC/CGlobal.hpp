#ifndef __CGLOBAL_HPP__
#define __CGLOBAL_HPP__
#include <string>
#include "CProtocol.hpp"
#include "CFileLog.hpp"
#include "CThreadQueue.hpp"
#include "CMesgExchSocket.hpp"
//#include "CTimerHandler.hpp"
//#include "CNMInterface.hpp"
#include "CClaInterface.hpp"

using namespace std;

class CGlobal{
	private:
		static CGlobal *m_cInstance;
		CFileLog *m_cLog;
		/* Timer event process */
		//CTimerHandler *m_cTimerHandler;
		//CClaInterface *m_cClaInterface;
		CClaInterface *m_cClaInterface;

		CGlobal();
		~CGlobal();
	public:
		static CGlobal *GetInstance();
		static int GetCmdCode(std::string &a_strCmdCode){
			if(a_strCmdCode.size() < 10){
				return 0;
			}

			return atoi(&a_strCmdCode.c_str()[4]);
		};
		CFileLog *GetLogP();
		//CTimerHandler *GetTimerHandlerP() { return m_cTimerHandler; };
		//CClaInterface *GetClaInterfaceP() { return m_cClaInterface; };
		CClaInterface *GetClaInterfaceP() { return m_cClaInterface; };
		int GetPathLen(const char *a_szDir, unsigned int *a_nLastCur);
		int ForceDir(const char *a_szDir);
};

#endif
